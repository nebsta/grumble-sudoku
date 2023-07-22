//
//  MTKViewDelegate.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#pragma once

#include <memory>

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

#include <grumble/logging/Logger.hpp>
#include <grumble/core/Game.hpp>
#include <grumble/ui/ScreenManager.hpp>

#include "MetalScreenManager.hpp"
#include "utils/IOSUtils.hpp"

class MTKViewDelegate : public MTK::ViewDelegate {
  public:
    MTKViewDelegate(MTL::Device* pDevice, std::shared_ptr<grumble::Game> game, std::shared_ptr<MetalScreenManager> metalScreenManager);
    virtual ~MTKViewDelegate() override;
    virtual void drawInMTKView(MTK::View* pView) override;
    virtual void drawableSizeWillChange(MTK::View* pView, CGSize size) override;
  
  private:
    std::shared_ptr<grumble::Game> _game;
    std::shared_ptr<MetalScreenManager> _screenManager;
  
    MTL::Device* _device;
    MTL::CommandQueue* _commandQueue;
    MTL::RenderPipelineState* _pipelineState;
    MTL::Buffer* _vertexPositionsBuffer;
    MTL::Buffer* _vertexColorsBuffer;
  
    dispatch_semaphore_t _drawSemaphore;
    int _activeFrame;
  
    void buildShaders();
    void buildBuffers();
};
