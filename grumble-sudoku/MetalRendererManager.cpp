//
//  MetalRendererManager.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 22/07/2023.
//

#include "MetalRendererManager.hpp"

MetalRendererManager::MetalRendererManager() {
  
}

std::shared_ptr<grumble::Renderer> MetalRendererManager::createViewRenderer() {
  return std::make_shared<MetalRenderer>(MetalRenderer());
}

