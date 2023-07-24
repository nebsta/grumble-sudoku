//
//  MTKViewDelegate.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#include "MTKViewDelegate.hpp"

MTKViewDelegate::MTKViewDelegate(MTL::Device* device,
                                 std::shared_ptr<grumble::Game> game,
                                 std::shared_ptr<MetalRendererManager> metalRendererManager): MTK::ViewDelegate() {
  _device = device->retain();
  _game = game;
  _rendererManager = metalRendererManager;
  _drawSemaphore = dispatch_semaphore_create(MAX_FRAMES_IN_FLIGHT);
}

MTKViewDelegate::~MTKViewDelegate() {
  _device->release();
}

void MTKViewDelegate::drawInMTKView(MTK::View* pView) {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  
  int activeFrameIndex = (_activeFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
  _activeFrameIndex = activeFrameIndex;
  
  MTL::CommandBuffer* commandBuffer = _rendererManager->commandBuffer();
  dispatch_semaphore_wait(_drawSemaphore, DISPATCH_TIME_FOREVER);
  MTKViewDelegate *delegateInstance = this;
  commandBuffer->addCompletedHandler(^void( MTL::CommandBuffer* pCmd ){
    dispatch_semaphore_signal(delegateInstance->_drawSemaphore);
  });
  
  _rendererManager->setActiveFrame(activeFrameIndex);
  _game->render();

  commandBuffer->commit();

  pool->release();
}

void MTKViewDelegate::drawableSizeWillChange(MTK::View* pView, CGSize size) {
  grumble::Logger::debug("drawableSizeWillChange: " + iosutils::to_string(size));
  _rendererManager->screenSizeUpdated(size);
}
