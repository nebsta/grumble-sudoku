//
//  MetalRendererManager.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 22/07/2023.
//

#pragma once

#include <memory>

#include <grumble/render/RendererManager.hpp>

#include "MetalRenderer.hpp"

class MetalRendererManager: public grumble::RendererManager {
public:
  MetalRendererManager();
  ~MetalRendererManager() override = default;
  
  std::shared_ptr<grumble::Renderer> createViewRenderer() override;
};
