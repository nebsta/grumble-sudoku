//
//  MetalScreenManager.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 20/07/2023.
//

#pragma once

#include <Metal/Metal.hpp>
#include <glm/glm.hpp>

#include <grumble/ui/ScreenManager.hpp>
#include <grumble/logging/Logger.hpp>

class MetalScreenManager: public grumble::ScreenManager {
private:
  glm::vec2 _screenSize;
  
public:
  ~MetalScreenManager() override = default;
  
  void setup() override;
  
  glm::vec2 screenSize() const override;
  
  void updateScreenSize(CGSize size);
};
