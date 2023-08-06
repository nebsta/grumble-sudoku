//
//  MetalUtils.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 23/07/2023.
//

#pragma once

#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <grumble/render/RenderMethod.hpp>
#include <grumble/sprite/SpriteRegion.hpp>

class MetalUtil {
public:
  static MTL::PrimitiveType to_mtl_primitive_type(grumble::RenderMethod method) {
    switch (method) {
      case grumble::RenderMethod::Triangle: return MTL::PrimitiveType::PrimitiveTypeTriangle;
      case grumble::RenderMethod::TriangleStrip: return MTL::PrimitiveType::PrimitiveTypeTriangleStrip;
    }
  }
  
  static simd::float4x4 to_simd_float4x4(glm::mat4 matrix) {
    return simd::float4x4(simd::make_float4(matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]),
                          simd::make_float4(matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]),
                          simd::make_float4(matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]),
                          simd::make_float4(matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]));
  }
  
  static simd::float4 to_simd_float4(glm::vec4 vector) {
    return simd::make_float4(vector.x, vector.y, vector.z, vector.w);
  }
  
  static simd::float4x4 ortho_matrix(float left, float right, float bottom, float top, float near, float far) {
      return simd::float4x4(simd::make_float4(2.0f / (right - left), 0, 0, 0),
                            simd::make_float4(0, 2.0f / (top - bottom), 0, 0),
                            simd::make_float4(0, 0, 1.0f  / (far - near), 0),
                            simd::make_float4((left + right) / (left - right), (top + bottom) / (bottom - top), near / (near - far), 1.0f));
  }
  
  static simd::float2 to_simd_float2(glm::vec2 vector) {
    return simd::make_float2(vector.x, vector.y);
  }
};
