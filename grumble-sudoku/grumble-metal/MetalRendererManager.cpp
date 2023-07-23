//
//  MetalRendererManager.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 22/07/2023.
//

#include "MetalRendererManager.hpp"

MetalRendererManager::MetalRendererManager(MTL::Device* device) {
  _device = device->retain();
  _commandQueue = _device->newCommandQueue();
  
  buildShaders();
  buildBuffers();
}

MetalRendererManager::~MetalRendererManager() {
  _device->release();
  _commandQueue->release();
  _pipelineState->release();
  _vertexPositionsBuffer->release();
  _vertexColorsBuffer->release();
  _shaderLibrary->release();
}

MTL::CommandQueue* MetalRendererManager::commandQueue() {
  return _commandQueue;
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

  MTL::Buffer* pVertexPositionsBuffer = _device->newBuffer(positionsDataSize, MTL::ResourceStorageModeManaged);
  MTL::Buffer* pVertexColorsBuffer = _device->newBuffer(colorDataSize, MTL::ResourceStorageModeManaged);

  _vertexPositionsBuffer = pVertexPositionsBuffer;
  _vertexColorsBuffer = pVertexColorsBuffer;
  
  const size_t instanceDataSize = MAX_FRAMES_IN_FLIGHT * MAX_INSTANCES * sizeof(BufferInstanceData);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    _instanceDataBuffer[i] = _device->newBuffer(instanceDataSize, MTL::ResourceStorageModeManaged);
  }

  memcpy(_vertexPositionsBuffer->contents(), positions, positionsDataSize);
  memcpy(_vertexColorsBuffer->contents(), colors, colorDataSize);

  _vertexPositionsBuffer->didModifyRange(NS::Range::Make(0, _vertexPositionsBuffer->length()));
  _vertexColorsBuffer->didModifyRange(NS::Range::Make(0, _vertexColorsBuffer->length()));
  
  MTL::Function* vertexFunction = _shaderLibrary->newFunction(NS::String::string("vertexMain", NS::UTF8StringEncoding));
  MTL::ArgumentEncoder* argumentEncoder = vertexFunction->newArgumentEncoder(0);
  
  _argumentBuffer = _device->newBuffer(argumentEncoder->encodedLength(), MTL::ResourceStorageModeManaged);
  argumentEncoder->setArgumentBuffer(_argumentBuffer, 0);
  argumentEncoder->setBuffer(_vertexPositionsBuffer, 0, 0);
  argumentEncoder->setBuffer(_vertexColorsBuffer, 0, 1);
  _argumentBuffer->didModifyRange(NS::Range::Make(0, _argumentBuffer->length()));
  
  vertexFunction->release();
  argumentEncoder->release();
}

void MetalRendererManager::frameSetup(MTK::View* mtkView, MTL::CommandBuffer* commandBuffer) {
  _mtkView = mtkView;
  _commandBuffer = commandBuffer;
}

void MetalRendererManager::render(std::shared_ptr<grumble::View> view) {
  if (_mtkView == nullptr || _commandQueue == nullptr) {
    grumble::Logger::error("Trying to render without setting the mtkView or commandQueue!");
    exit(1);
    return;
  }
  
  MTL::PrimitiveType primitiveType = MetalUtil::to_mtl_primitive_type(view->renderer().renderMethod());
  
  MTL::RenderPassDescriptor* renderPassDesc = _mtkView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* renderCommEncoder = _commandBuffer->renderCommandEncoder(renderPassDesc);

  renderCommEncoder->setRenderPipelineState(_pipelineState);
  renderCommEncoder->setVertexBuffer(_argumentBuffer, 0, 0);
  renderCommEncoder->useResource(_vertexPositionsBuffer, MTL::ResourceUsageRead);
  renderCommEncoder->useResource(_vertexColorsBuffer, MTL::ResourceUsageRead);

  renderCommEncoder->drawPrimitives(primitiveType, NS::UInteger(0), NS::UInteger(4));
  renderCommEncoder->endEncoding();
}

MTL::CommandBuffer* MetalRendererManager::commandBuffer() {
  return _commandBuffer;
}
