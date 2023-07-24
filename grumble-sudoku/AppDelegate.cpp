//
//  AppDelegate.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#include "AppDelegate.hpp"

AppDelegate::AppDelegate() {
  
}

AppDelegate::~AppDelegate() {
  _mtkView->release();
  _window->release();
  _device->release();
  
  delete _viewDelegate;
}

NS::Menu* AppDelegate::createMenuBar() {
  using NS::StringEncoding::UTF8StringEncoding;

  NS::Menu* pMainMenu = NS::Menu::alloc()->init();
  NS::MenuItem* pAppMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu* pAppMenu = NS::Menu::alloc()->init(NS::String::string("Appname", UTF8StringEncoding));

  NS::String* appName = NS::RunningApplication::currentApplication()->localizedName();
  NS::String* quitItemName = NS::String::string("Quit ", UTF8StringEncoding )->stringByAppendingString(appName);
  SEL quitCb = NS::MenuItem::registerActionCallback( "appQuit", [](void*,SEL,const NS::Object* pSender){
      auto pApp = NS::Application::sharedApplication();
      pApp->terminate( pSender );
  } );

  NS::MenuItem* pAppQuitItem = pAppMenu->addItem(quitItemName, quitCb, NS::String::string("q", UTF8StringEncoding));
  pAppQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
  pAppMenuItem->setSubmenu(pAppMenu);

  NS::MenuItem* pWindowMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu* pWindowMenu = NS::Menu::alloc()->init(NS::String::string( "Window", UTF8StringEncoding));

  SEL closeWindowCb = NS::MenuItem::registerActionCallback("windowClose", [](void*, SEL, const NS::Object*){
      auto pApp = NS::Application::sharedApplication();
          pApp->windows()->object< NS::Window >(0)->close();
  });
  NS::MenuItem* pCloseWindowItem = pWindowMenu->addItem(NS::String::string("Close Window", UTF8StringEncoding), closeWindowCb, NS::String::string("w", UTF8StringEncoding));
  pCloseWindowItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);

  pWindowMenuItem->setSubmenu(pWindowMenu);

  pMainMenu->addItem(pAppMenuItem);
  pMainMenu->addItem(pWindowMenuItem);

  pAppMenuItem->release();
  pWindowMenuItem->release();
  pAppMenu->release();
  pWindowMenu->release();

  return pMainMenu->autorelease();
}

void AppDelegate::applicationWillFinishLaunching(NS::Notification* pNotification) {
  NS::Menu* pMenu = createMenuBar();
  NS::Application* pApp = reinterpret_cast<NS::Application*>(pNotification->object());
  pApp->setMainMenu(pMenu);
  pApp->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void AppDelegate::applicationDidFinishLaunching(NS::Notification* pNotification) {
  CGRect frame = (CGRect){ {100.0, 100.0}, {512.0, 512.0} };

  _window = NS::Window::alloc()->init(
    frame,
    NS::WindowStyleMaskClosable|NS::WindowStyleMaskTitled|NS::WindowStyleMaskResizable,
    NS::BackingStoreBuffered,
    false);
  
  _device = MTL::CreateSystemDefaultDevice();
  
  _mtkView = MTK::View::alloc()->init(frame, _device);
  
  std::shared_ptr<MetalRendererManager> metalRendererManager = std::make_shared<MetalRendererManager>(_device, _mtkView);
  _game = std::make_shared<grumble::Game>(metalRendererManager);
  _game->setup(2.0f); // Should eventually be read/updated from AppKit/UIKit
  
  // color square sample
//  glm::vec2 cellSize = { 32.0f, 32.0f };
//  grumble::TransformOrigin origin = grumble::TransformOrigin::TopLeft;
//  float xOffset = 32.0f;
//  float yOffset = 32.0f;
//  std::shared_ptr<grumble::View> rootView = _game->viewFactory()->createView({0, 0}, cellSize, origin);
//  for (int i = 0; i < 16; i++) {
//    for (int j = 0; j < 16; j++) {
//      std::shared_ptr<grumble::View> subView = _game->viewFactory()->createView({xOffset * i, yOffset * j}, cellSize, origin);
//      subView->renderer().setTint(COLOR_RANDOM);
//      rootView->addChild(subView);
//    }
//  }
  
  _mtkView->setColorPixelFormat( MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB );
  _mtkView->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 0.0, 1.0));

  _viewDelegate = new MTKViewDelegate(_device, _game, metalRendererManager);
  _mtkView->setDelegate(_viewDelegate);

  _window->setContentView(_mtkView);
  _window->setTitle(NS::String::string("Grumble Sudoku", NS::StringEncoding::UTF8StringEncoding));

  _window->makeKeyAndOrderFront(nullptr);

  NS::Application* pApp = reinterpret_cast<NS::Application*>(pNotification->object());
  pApp->activateIgnoringOtherApps(true);
}

bool AppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* pSender) {
  return true;
}
