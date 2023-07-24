//
//  MetalRendererManager.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 22/07/2023.
//

#include "MetalRendererManager.hpp"

MetalRendererManager::MetalRendererManager(MTL::Device* device, MTK::View* mtkView) {
  _device = device->retain();
  _mtkView = mtkView->retain();
  _commandQueue = _device->newCommandQueue();
  
  screenSizeUpdated(mtkView->drawableSize());
  buildShaders();
  buildBuffers();
}

MetalRendererManager::~MetalRendererManager() {
  _device->release();
  _mtkView->release();
  _commandQueue->release();
  _pipelineState->release();
  _vertexPositionsBuffer->release();
  _shaderLibrary->release();
  
  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    _uniformBuffers[i]->release();
  }
}

MTL::CommandBuffer* MetalRendererManager::generateCommandBuffer() {
  _currentCommandBuffer = _commandQueue->commandBuffer();
  
  MTL::RenderPassDescriptor* renderPassDesc = _mtkView->currentRenderPassDescriptor();
  _currentCommandEncoder = _currentCommandBuffer->renderCommandEncoder(renderPassDesc);
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

  simd::float3 positions[NumVertices] =
  {
      { -1.0f,  1.0f, 0.0f },
      { -1.0f,  -1.0f, 0.0f },
      {  +1.0f, +1.0f, 0.0f },
      { +1.0f,  -1.0f, 0.0f }
  };

  const size_t positionsDataSize = NumVertices * sizeof(simd::float3);
  const size_t uniformDataSize = sizeof(UniformData);

  MTL::Buffer* pVertexPositionsBuffer = _device->newBuffer(positionsDataSize, MTL::ResourceStorageModeManaged);

  _vertexPositionsBuffer = pVertexPositionsBuffer;
  
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    _uniformBuffers[i] = _device->newBuffer(uniformDataSize, MTL::ResourceStorageModeManaged);
  }

  memcpy(_vertexPositionsBuffer->contents(), positions, positionsDataSize);

  _vertexPositionsBuffer->didModifyRange(NS::Range::Make(0, _vertexPositionsBuffer->length()));
}

void MetalRendererManager::setActiveFrame(int index) {
  _activeFrameIndex = index;
}

void MetalRendererManager::render(std::shared_ptr<grumble::View> view) {
  MTL::Buffer* uniformBuffer = _uniformBuffers[_activeFrameIndex];
  
  UniformData* uniformData = reinterpret_cast<UniformData*>(uniformBuffer->contents());
  simd::float4x4 modelMatrix = MetalUtil::to_simd_float4x4(view->transform().modelMatrix());
  uniformData->modelMatrix = modelMatrix;
  uniformData->projectionMatrix = _projectionMatrix;
  uniformData->tint = MetalUtil::to_simd_float4(view->renderer().tint());
  uniformBuffer->didModifyRange(NS::Range::Make(0, sizeof(UniformData)));
  
  MTL::PrimitiveType primitiveType = MetalUtil::to_mtl_primitive_type(view->renderer().renderMethod());
  _currentCommandEncoder->setRenderPipelineState(_pipelineState);
  _currentCommandEncoder->setVertexBuffer(_vertexPositionsBuffer, 0, 0);
  _currentCommandEncoder->setVertexBuffer(uniformBuffer, 0, 1);

  _currentCommandEncoder->drawPrimitives(primitiveType, NS::UInteger(0), NS::UInteger(4));
}

void MetalRendererManager::screenSizeUpdated(CGSize size) {
  glm::mat4 glmOrtho =  glm::ortho(0.0f, 512.0f, 512.0f, 0.0f);
  _projectionMatrix = MetalUtil::to_simd_float4x4(glmOrtho);
//  _projectionMatrix = MetalUtil::ortho_matrix(0.0f, float(size.width), float(size.height), 0.0f, -1.0f, 1.0f);
  
}

void MetalRendererManager::finishFrame() {
  _currentCommandEncoder->endEncoding();
  _currentCommandBuffer->presentDrawable(_mtkView->currentDrawable());
  _currentCommandBuffer->commit();
}
