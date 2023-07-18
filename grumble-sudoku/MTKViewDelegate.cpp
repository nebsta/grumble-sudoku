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
}

MTKViewDelegate::~MTKViewDelegate() {
  _device->release();
  _commandQueue->release();
}

void MTKViewDelegate::drawInMTKView(MTK::View* pView) {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer* commandBuffer = _commandQueue->commandBuffer();
  MTL::RenderPassDescriptor* renderPassDesc = pView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* renderCommEncoder = commandBuffer->renderCommandEncoder(renderPassDesc);
  renderCommEncoder->endEncoding();
  commandBuffer->presentDrawable(pView->currentDrawable());
  commandBuffer->commit();

  pool->release();
}
