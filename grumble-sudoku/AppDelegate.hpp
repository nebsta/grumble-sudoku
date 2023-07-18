//
//  AppDelegate.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#pragma once

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

#include "MTKViewDelegate.hpp"

class AppDelegate : public NS::ApplicationDelegate
{
  public:
    ~AppDelegate();

    NS::Menu* createMenuBar();

    virtual void applicationWillFinishLaunching(NS::Notification* notification) override;
    virtual void applicationDidFinishLaunching(NS::Notification* notification) override;
    virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) override;

  private:
    NS::Window* _window;
    MTK::View* _mtkView;
    MTL::Device* _device;
    MTKViewDelegate* _viewDelegate = nullptr;
};
