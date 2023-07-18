//
//  MTKViewDelegate.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#pragma once

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

class MTKViewDelegate : public MTK::ViewDelegate {
  public:
    MTKViewDelegate(MTL::Device* pDevice);
    virtual ~MTKViewDelegate() override;
    virtual void drawInMTKView(MTK::View* pView) override;
  
  private:
    MTL::Device* _device;
    MTL::CommandQueue* _commandQueue;
};
