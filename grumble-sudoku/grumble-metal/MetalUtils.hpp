//
//  MetalUtils.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 23/07/2023.
//

#pragma once

#include <Metal/Metal.hpp>

#include <grumble/render/RenderMethod.hpp>

class MetalUtil {
public:
  static MTL::PrimitiveType to_mtl_primitive_type(grumble::RenderMethod method) {
    switch (method) {
      case grumble::RenderMethod::Triangle: return MTL::PrimitiveType::PrimitiveTypeTriangle;
      case grumble::RenderMethod::TriangleStrip: return MTL::PrimitiveType::PrimitiveTypeTriangleStrip;
    }
  }
};
