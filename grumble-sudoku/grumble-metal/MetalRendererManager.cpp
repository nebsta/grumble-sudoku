//
//  MetalRendererManager.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 22/07/2023.
//

#include "MetalRendererManager.hpp"

MetalRendererManager::MetalRendererManager(MTL::Device* device,
                                           MTK::View* mtkView,
                                           grumble::SpriteManager::shared_ptr spriteManager) :
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
  std::map<std::string, MTL::Buffer*>::iterator iter = _texCoordBuffers.begin();
  for (; iter != _texCoordBuffers.end(); ++iter) {
    (*iter).second->release();
  }
  
  _emptyTexCoordBuffer->release();
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
  VertexData vertexData[VERTEX_COUNT] = {
    {{ -1.0f,  1.0f, 0.0f }},
    {{ 1.0f,  1.0f, 0.0f }},
    {{  -1.0f, -1.0f, 0.0f }},
    {{ 1.0f,  -1.0f, 0.0f }}
  };

  const size_t vertexDataSize = VERTEX_COUNT * sizeof(VertexData);
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
  const size_t texCoordDataSize = VERTEX_COUNT * sizeof(TexCoordData);
  
  _emptyTexCoordBuffer = _device->newBuffer(texCoordDataSize, MTL::ResourceStorageModeManaged);
  simd::float2 emptyTexCoords[VERTEX_COUNT] = { { 0.0, 0.0}, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 } };
  memcpy(_emptyTexCoordBuffer->contents(), emptyTexCoords, texCoordDataSize);
  _emptyTexCoordBuffer->didModifyRange(NS::Range::Make(0, _emptyTexCoordBuffer->length()));
  
  std::vector<std::shared_ptr<grumble::SpriteAtlas>> allAtlases = _spriteManager->allAtlases();
  grumble::SpriteAtlas::iterator iterator = allAtlases.begin();
  for (; iterator != allAtlases.end(); iterator++) {
    std::shared_ptr<grumble::SpriteAtlas> atlas = (*iterator);
    
    logInfo("Setting up atlas in MetalRenderManager: {}", atlas->toString());
    
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
    
    grumble::Sprite::vector sprites = atlas->allSprites();
    grumble::Sprite::iterator spriteIter = sprites.begin();
    for (; spriteIter != sprites.end(); spriteIter++) {
      grumble::Sprite::shared_ptr sprite = (*spriteIter);
      MTL::Buffer* texCoordBuffer = _device->newBuffer(texCoordDataSize, MTL::ResourceStorageModeManaged);

      simd::float2 texCoords[VERTEX_COUNT] = {
        MetalUtil::to_simd_float2(sprite->region().topLeft),
        MetalUtil::to_simd_float2(sprite->region().topRight),
        MetalUtil::to_simd_float2(sprite->region().bottomLeft),
        MetalUtil::to_simd_float2(sprite->region().bottomRight)
      };
      
      memcpy(texCoordBuffer->contents(), texCoords, texCoordDataSize);
      texCoordBuffer->didModifyRange(NS::Range::Make(0, texCoordBuffer->length()));
      
      logDebug("Setting up texCoord for sprite: {}", sprite->toString());
      
      std::pair<std::string,MTL::Buffer*> item = { sprite->id(), texCoordBuffer };
      _texCoordBuffers.insert(item);
    }
  }
}

void MetalRendererManager::setActiveFrame(int index) {
  _activeFrameIndex = index;
}

void MetalRendererManager::renderView(grumble::Transform::shared_ptr transform,
                                      grumble::Renderer::shared_ptr renderer) {
  MTL::Buffer* uniformBuffer = _uniformBuffers[_activeFrameIndex][_instanceIndex];
  
  UniformData* uniformData = reinterpret_cast<UniformData*>(uniformBuffer->contents());
  simd::float4x4 modelMatrix = MetalUtil::to_simd_float4x4(transform->modelMatrix(renderScale()));
  uniformData->modelMatrix = modelMatrix;
  uniformData->projectionMatrix = _projectionMatrix;
  uniformData->tint = MetalUtil::to_simd_float4(renderer->tint());
  uniformBuffer->didModifyRange(NS::Range::Make(0, sizeof(UniformData)));
  
  MTL::PrimitiveType primitiveType = MetalUtil::to_mtl_primitive_type(renderer->renderMethod());
  _currentCommandEncoder->setFragmentTexture(nullptr, 0);
  _currentCommandEncoder->setVertexBuffer(_vertexBuffer, 0, 0);
  _currentCommandEncoder->setVertexBuffer(_emptyTexCoordBuffer, 0, 1);
  _currentCommandEncoder->setVertexBuffer(uniformBuffer, 0, 2);

  _currentCommandEncoder->drawPrimitives(primitiveType, NS::UInteger(0), NS::UInteger(4));
  
  _instanceIndex++;
}

void MetalRendererManager::renderImageView(grumble::Transform::shared_ptr transform,
                                           grumble::ImageRenderer::shared_ptr renderer) {
  MTL::Buffer* uniformBuffer = _uniformBuffers[_activeFrameIndex][_instanceIndex];
  MTL::Buffer* texCoordBuffer = _texCoordBuffers[renderer->sprite()->id()];
  MTL::Texture* texture = _textureBuffers.at(renderer->sprite()->atlas());
  
  UniformData* uniformData = reinterpret_cast<UniformData*>(uniformBuffer->contents());
  simd::float4x4 modelMatrix = MetalUtil::to_simd_float4x4(transform->modelMatrix(renderScale()));
  uniformData->modelMatrix = modelMatrix;
  uniformData->projectionMatrix = _projectionMatrix;
  uniformData->tint = MetalUtil::to_simd_float4(renderer->tint());
  uniformBuffer->didModifyRange(NS::Range::Make(0, sizeof(UniformData)));
  
  
  MTL::PrimitiveType primitiveType = MetalUtil::to_mtl_primitive_type(renderer->renderMethod());
  _currentCommandEncoder->setFragmentTexture(texture, 0);
  _currentCommandEncoder->setVertexBuffer(_vertexBuffer, 0, 0);
  _currentCommandEncoder->setVertexBuffer(texCoordBuffer, 0, 1);
  _currentCommandEncoder->setVertexBuffer(uniformBuffer, 0, 2);

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
