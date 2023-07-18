//
//  MTKViewDelegate.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#include "MTKViewDelegate.hpp"

MTKViewDelegate::MTKViewDelegate(MTL::Device* device): MTK::ViewDelegate() {
  _device = device->retain();
  _commandQueue = _device->newCommandQueue();
  buildShaders();
  buildBuffers();
}

MTKViewDelegate::~MTKViewDelegate() {
  _device->release();
  _commandQueue->release();
  _pipelineState->release();
  _vertexPositionsBuffer->release();
  _vertexColorsBuffer->release();
}

void MTKViewDelegate::buildShaders() {
  NS::Error* error = nullptr;

//  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  
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

  library->release();
  vertexFunction->release();
  fragFunction->release();
  pipelineDescriptors->release();
}

void MTKViewDelegate::buildBuffers() {
  const size_t NumVertices = 3;

  simd::float3 positions[NumVertices] =
  {
      { -0.8f,  0.8f, 0.0f },
      {  0.0f, -0.8f, 0.0f },
      { +0.8f,  0.8f, 0.0f }
  };

  simd::float3 colors[NumVertices] =
  {
      {  1.0, 0.3f, 0.2f },
      {  0.8f, 1.0, 0.0f },
      {  0.8f, 0.0f, 1.0 }
  };

  const size_t positionsDataSize = NumVertices * sizeof(simd::float3);
  const size_t colorDataSize = NumVertices * sizeof(simd::float3);

  MTL::Buffer* pVertexPositionsBuffer = _device->newBuffer(positionsDataSize, MTL::ResourceStorageModeManaged);
  MTL::Buffer* pVertexColorsBuffer = _device->newBuffer(colorDataSize, MTL::ResourceStorageModeManaged);

  _vertexPositionsBuffer = pVertexPositionsBuffer;
  _vertexColorsBuffer = pVertexColorsBuffer;

  memcpy(_vertexPositionsBuffer->contents(), positions, positionsDataSize);
  memcpy(_vertexColorsBuffer->contents(), colors, colorDataSize);

  _vertexPositionsBuffer->didModifyRange(NS::Range::Make(0, _vertexPositionsBuffer->length()));
  _vertexColorsBuffer->didModifyRange(NS::Range::Make(0, _vertexColorsBuffer->length()));
}

void MTKViewDelegate::drawInMTKView(MTK::View* pView) {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer* commandBuffer = _commandQueue->commandBuffer();
  MTL::RenderPassDescriptor* renderPassDesc = pView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* renderCommEncoder = commandBuffer->renderCommandEncoder(renderPassDesc);
  renderCommEncoder->setRenderPipelineState(_pipelineState);
  renderCommEncoder->setVertexBuffer(_vertexPositionsBuffer, 0, 0);
  renderCommEncoder->setVertexBuffer(_vertexColorsBuffer, 0, 1);
  renderCommEncoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3));
  renderCommEncoder->endEncoding();
  commandBuffer->presentDrawable(pView->currentDrawable());
  commandBuffer->commit();

  pool->release();
}
