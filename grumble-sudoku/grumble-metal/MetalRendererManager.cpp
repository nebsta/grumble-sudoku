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
  
  buildShaders();
  buildBuffers();
}

MetalRendererManager::~MetalRendererManager() {
  _device->release();
  _mtkView->release();
  _commandQueue->release();
  _pipelineState->release();
  _vertexPositionsBuffer->release();
  _vertexColorsBuffer->release();
  _shaderLibrary->release();
}

MTL::CommandBuffer* MetalRendererManager::commandBuffer() {
  _commandBuffer = _commandQueue->commandBuffer();
  return _commandBuffer;
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
      { -0.8f,  0.8f, 0.0f },
      { -0.8f,  -0.8f, 0.0f },
      {  +0.8f, +0.8f, 0.0f },
      { +0.8f,  -0.8f, 0.0f }
  };

  simd::float3 colors[NumVertices] =
  {
    {  1.0, 0.3f, 0.2f },
    {  0.8f, 1.0, 0.0f },
    {  0.8f, 0.0f, 1.0 },
    {  0.8f, 0.0f, 1.0 }
  };

  const size_t positionsDataSize = NumVertices * sizeof(simd::float3);
  const size_t colorDataSize = NumVertices * sizeof(simd::float3);
  const size_t uniformDataSize = sizeof(UniformData);

  MTL::Buffer* pVertexPositionsBuffer = _device->newBuffer(positionsDataSize, MTL::ResourceStorageModeManaged);
  MTL::Buffer* pVertexColorsBuffer = _device->newBuffer(colorDataSize, MTL::ResourceStorageModeManaged);

  _vertexPositionsBuffer = pVertexPositionsBuffer;
  _vertexColorsBuffer = pVertexColorsBuffer;
  
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    _uniformBuffers[i] = _device->newBuffer(uniformDataSize, MTL::ResourceStorageModeManaged);
  }

  memcpy(_vertexPositionsBuffer->contents(), positions, positionsDataSize);
  memcpy(_vertexColorsBuffer->contents(), colors, colorDataSize);

  _vertexPositionsBuffer->didModifyRange(NS::Range::Make(0, _vertexPositionsBuffer->length()));
  _vertexColorsBuffer->didModifyRange(NS::Range::Make(0, _vertexColorsBuffer->length()));
}

void MetalRendererManager::render(std::shared_ptr<grumble::View> view) {
  _activeFrameIndex = (_activeFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
  MTL::Buffer* uniformBuffer = _uniformBuffers[_activeFrameIndex];
  
  UniformData* uniformData = reinterpret_cast<UniformData*>(uniformBuffer->contents());
  
  uniformData->modelMatrix = MetalUtil::to_simd_float4x4(view->transform().modelMatrix());
  uniformData->projectionMatrix = _projectionMatrix;
  uniformBuffer->didModifyRange(NS::Range::Make(0, uniformBuffer->length()));
  
  MTL::PrimitiveType primitiveType = MetalUtil::to_mtl_primitive_type(view->renderer().renderMethod());
  
  MTL::RenderPassDescriptor* renderPassDesc = _mtkView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* renderCommEncoder = _commandBuffer->renderCommandEncoder(renderPassDesc);
  
  renderCommEncoder->setRenderPipelineState(_pipelineState);
  renderCommEncoder->setVertexBuffer(_vertexPositionsBuffer, 0, 0);
  renderCommEncoder->setVertexBuffer(uniformBuffer, 0, 1);

  renderCommEncoder->drawPrimitives(primitiveType, NS::UInteger(0), NS::UInteger(4));
  renderCommEncoder->endEncoding();
  
  _commandBuffer->presentDrawable(_mtkView->currentDrawable());
}

void MetalRendererManager::screenSizeUpdated(CGSize size) {
  glm::mat4 glmOrtho = glm::ortho(0.0f, float(size.width), 0.0f, float(size.height));
  _projectionMatrix = MetalUtil::to_simd_float4x4(glmOrtho);
}
