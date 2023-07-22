//
//  IOSUtils.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 20/07/2023.
//

#pragma once

#include <AppKit/AppKit.hpp>

namespace iosutils {
  static std::string to_string(CGSize size) {
    return std::to_string(size.width) + ", " + std::to_string(size.height);
  }
};
