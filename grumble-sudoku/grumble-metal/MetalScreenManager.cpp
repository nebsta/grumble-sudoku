//
//  MetalScreenManager.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 20/07/2023.
//

#include "MetalScreenManager.hpp"

void MetalScreenManager::setup() {
  
}

glm::vec2 MetalScreenManager::screenSize() const {
  return _screenSize;
}

void MetalScreenManager::updateScreenSize(CGSize size) {
  _screenSize = glm::vec2(size.width, size.height);
}

const glm::mat4 MetalScreenManager::projectionMatrix() const {
  return glm::ortho(0.0f, _screenSize.x, 0.0f, _screenSize.y);
}
