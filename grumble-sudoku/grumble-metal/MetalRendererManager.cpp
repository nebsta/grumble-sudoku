//
//  MetalRendererManager.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 22/07/2023.
//

#include "MetalRendererManager.hpp"

MetalRendererManager::MetalRendererManager(MTL::Device* device,
                                           MTK::View* mtkView,
                                           std::shared_ptr<grumble::SpriteManager> spriteManager) :
  _spriteManager(spriteManager) {
  _device = device->retain();
  _mtkView = mtkView->retain();
  _commandQueue = _device->newCommandQueue();
  
  screenSizeUpdated(mtkView->drawableSize());
}

MetalRendererManager::~MetalRendererManager() {
  _device->release();
  _mtkView->release();
  _commandQueue->release();
  _pipelineState->release();
  _vertexBuffer->release();
  _shaderLibrary->release();
  
  std::map<std::string,MTL::Texture*>::iterator iterator = _textureBuffers.begin();
  for (; iterator != _textureBuffers.end(); iterator++) {
    (*iterator).second->release();
  }
  
  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    for (int j = 0; i < MAX_INSTANCES; j++) {
      _uniformBuffers[i][j]->release();
    }
  }
}

void MetalRendererManager::setup() {
  buildShaders();
  buildBuffers();
  buildTextures();
}

MTL::CommandBuffer* MetalRendererManager::generateCommandBuffer() {
  _currentCommandBuffer = _commandQueue->commandBuffer();
  
  MTL::RenderPassDescriptor* renderPassDesc = _mtkView->currentRenderPassDescriptor();
  _currentCommandEncoder = _currentCommandBuffer->renderCommandEncoder(renderPassDesc);
  _currentCommandEncoder->setRenderPipelineState(_pipelineState);
  _instanceIndex = 0;
  
  return _currentCommandBuffer;
}

void MetalRendererManager::buildShaders() {
  NS::Error* error = nullptr;
  
  MTL::Library* library = _device->newDefaultLibrary();
  if (!library) {
    __builtin_printf("library load failed: %s", error->localizedDescription()->utf8String());
    assert(false);
  }

  MTL::Function* vertexFunction = library->newFunction(NS::String::string("vertexMain", NS::UTF8StringEncoding));
  MTL::Function* fragFunction = library->newFunction(NS::String::string("fragmentMain", NS::UTF8StringEncoding));

  MTL::RenderPipelineDescriptor* pipelineDescriptors = MTL::RenderPipelineDescriptor::alloc()->init();
  pipelineDescriptors->setVertexFunction(vertexFunction);
  pipelineDescriptors->setFragmentFunction(fragFunction);
  pipelineDescriptors->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

  _pipelineState = _device->newRenderPipelineState(pipelineDescriptors, &error);
  if (!_pipelineState) {
    __builtin_printf("pipeline state creation failed: %s", error->localizedDescription()->utf8String());
    assert(false);
  }

  vertexFunction->release();
  fragFunction->release();
  pipelineDescriptors->release();
  
  _shaderLibrary = library;
}

void MetalRendererManager::buildBuffers() {
  const size_t NumVertices = 4;

  VertexData vertexData[NumVertices] =
  {
    {{ -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f }},
    {{ 1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f }},
    {{  -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }},
    {{ 1.0f,  -1.0f, 0.0f }, { 1.0f, 1.0f }}
  };

  const size_t vertexDataSize = NumVertices * sizeof(VertexData);
  const size_t uniformDataSize = sizeof(UniformData);

  MTL::Buffer* vertexBuffer = _device->newBuffer(vertexDataSize, MTL::ResourceStorageModeManaged);
  _vertexBuffer = vertexBuffer;
  
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    for (size_t j = 0; j < MAX_INSTANCES; j++) {
      _uniformBuffers[i][j] = _device->newBuffer(uniformDataSize, MTL::ResourceStorageModeManaged);
    }
  }

  memcpy(_vertexBuffer->contents(), vertexData, vertexDataSize);

  _vertexBuffer->didModifyRange(NS::Range::Make(0, _vertexBuffer->length()));
}

void MetalRendererManager::buildTextures() {
  std::vector<std::shared_ptr<grumble::SpriteAtlas>> allAtlases = _spriteManager->allAtlases();
  grumble::SpriteAtlas::Iterator iterator = allAtlases.begin();
  for (; iterator != allAtlases.end(); iterator++) {
    std::shared_ptr<grumble::SpriteAtlas> atlas = (*iterator);
    
    MTL::TextureDescriptor* textureDesc = MTL::TextureDescriptor::alloc()->init();
    textureDesc->setWidth(atlas->file()->width());
    textureDesc->setHeight(atlas->file()->height());
    textureDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
    textureDesc->setTextureType(MTL::TextureType2D);
    textureDesc->setStorageMode(MTL::StorageModeManaged);
    textureDesc->setUsage(MTL::ResourceUsageSample | MTL::ResourceUsageRead);

    MTL::Texture* texture = _device->newTexture(textureDesc);

    MTL::Region region = MTL::Region(0, 0, 0, atlas->file()->width(), atlas->file()->height(), 1);
    
    texture->replaceRegion(region, 0, atlas->file()->data().get(), atlas->file()->bytesPerRow());
    
    textureDesc->release();
    
    _textureBuffers[atlas->name()] = texture;
  }
}

void MetalRendererManager::setActiveFrame(int index) {
  _activeFrameIndex = index;
}

void MetalRendererManager::renderView(grumble::Transform transform, std::shared_ptr<grumble::Renderer> renderer) {
  MTL::Buffer* uniformBuffer = _uniformBuffers[_activeFrameIndex][_instanceIndex];
  
  UniformData* uniformData = reinterpret_cast<UniformData*>(uniformBuffer->contents());
  simd::float4x4 modelMatrix = MetalUtil::to_simd_float4x4(transform.modelMatrix(renderScale()));
  uniformData->modelMatrix = modelMatrix;
  uniformData->projectionMatrix = _projectionMatrix;
  uniformData->tint = MetalUtil::to_simd_float4(renderer->tint());
  uniformBuffer->didModifyRange(NS::Range::Make(0, sizeof(UniformData)));
  
  MTL::PrimitiveType primitiveType = MetalUtil::to_mtl_primitive_type(renderer->renderMethod());
  _currentCommandEncoder->setVertexBuffer(_vertexBuffer, 0, 0);
  _currentCommandEncoder->setVertexBuffer(uniformBuffer, 0, 1);

  _currentCommandEncoder->drawPrimitives(primitiveType, NS::UInteger(0), NS::UInteger(4));
  
  _instanceIndex++;
}

void MetalRendererManager::renderImageView(grumble::Transform transform, std::shared_ptr<grumble::ImageRenderer> renderer) {
  MTL::Buffer* uniformBuffer = _uniformBuffers[_activeFrameIndex][_instanceIndex];
  MTL::Texture* texture = _textureBuffers[renderer->sprite()->atlas()];
  
  UniformData* uniformData = reinterpret_cast<UniformData*>(uniformBuffer->contents());
  simd::float4x4 modelMatrix = MetalUtil::to_simd_float4x4(transform.modelMatrix(renderScale()));
  uniformData->modelMatrix = modelMatrix;
  uniformData->projectionMatrix = _projectionMatrix;
  uniformData->tint = MetalUtil::to_simd_float4(renderer->tint());
  uniformBuffer->didModifyRange(NS::Range::Make(0, sizeof(UniformData)));
  
  MTL::PrimitiveType primitiveType = MetalUtil::to_mtl_primitive_type(renderer->renderMethod());
  _currentCommandEncoder->setFragmentTexture(texture, 0);
  _currentCommandEncoder->setVertexBuffer(_vertexBuffer, 0, 0);
  _currentCommandEncoder->setVertexBuffer(uniformBuffer, 0, 1);

  _currentCommandEncoder->drawPrimitives(primitiveType, NS::UInteger(0), NS::UInteger(4));
  
  _instanceIndex++;
}

void MetalRendererManager::screenSizeUpdated(CGSize size) {
  glm::mat4 glmOrtho = submitScreenSize({size.width, size.height});
  _projectionMatrix = MetalUtil::to_simd_float4x4(glmOrtho);
}

void MetalRendererManager::finishFrame() {
  _currentCommandEncoder->endEncoding();
  _currentCommandBuffer->presentDrawable(_mtkView->currentDrawable());
  _currentCommandBuffer->commit();
}
