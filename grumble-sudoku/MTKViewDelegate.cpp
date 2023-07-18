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
  
  NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer* pCmd = _commandQueue->commandBuffer();
  MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder( pRpd );
  pEnc->endEncoding();
  pCmd->presentDrawable( pView->currentDrawable() );
  pCmd->commit();

  pPool->release();
}
