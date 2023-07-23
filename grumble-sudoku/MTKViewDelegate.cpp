//
//  MTKViewDelegate.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#include "MTKViewDelegate.hpp"


MTKViewDelegate::MTKViewDelegate(MTL::Device* device,
                                 std::shared_ptr<grumble::Game> game,
                                 std::shared_ptr<MetalScreenManager> metalScreenManager,
                                 std::shared_ptr<MetalRendererManager> metalRendererManager): MTK::ViewDelegate() {
  _device = device->retain();
  _game = game;
  _screenManager = metalScreenManager;
  _rendererManager = metalRendererManager;
  _drawSemaphore = dispatch_semaphore_create(MAX_FRAMES_IN_FLIGHT);
  _activeFrame = 0;
  
  _testView = std::make_unique<grumble::View>(grumble::View({0, 0}, { 10, 10 }));
}

MTKViewDelegate::~MTKViewDelegate() {
  _device->release();
}

void MTKViewDelegate::drawInMTKView(MTK::View* pView) {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

//  _activeFrame = (_activeFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//  dispatch_semaphore_wait(_drawSemaphore, DISPATCH_TIME_FOREVER);
  
  MTL::CommandBuffer* commandBuffer = _rendererManager->commandBuffer();
  _game->render();
  
  commandBuffer->commit();

  pool->release();
}

void update() {
  
}

void MTKViewDelegate::drawableSizeWillChange(MTK::View* pView, CGSize size) {
  grumble::Logger::debug("drawableSizeWillChange: " + iosutils::to_string(size));
  _screenManager->updateScreenSize(size);
}
