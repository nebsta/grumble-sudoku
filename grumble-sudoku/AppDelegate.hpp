//
//  AppDelegate.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#pragma once

#include <memory>

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

#include <grumble/core/Game.hpp>
#include <grumble/ui/View.hpp>
#include <grumble/render/RendererManager.hpp>

#include "MTKViewDelegate.hpp"
#include "MetalScreenManager.hpp"
#include "MetalRendererManager.hpp"

class AppDelegate : public NS::ApplicationDelegate
{
  public:
    AppDelegate();
    ~AppDelegate();

    NS::Menu* createMenuBar();

    virtual void applicationWillFinishLaunching(NS::Notification* notification) override;
    virtual void applicationDidFinishLaunching(NS::Notification* notification) override;
    virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) override;

  private:
    std::shared_ptr<grumble::Game> _game;
    std::shared_ptr<MetalScreenManager> _metalScreenManager;
  
    NS::Window* _window;
    MTK::View* _mtkView;
    MTL::Device* _device;
    MTKViewDelegate* _viewDelegate = nullptr;
};
