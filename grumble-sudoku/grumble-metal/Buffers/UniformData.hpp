//
//  UniformData.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 23/07/2023.
//

#pragma once

#include <simd/simd.h>

struct UniformData {
  simd::float3 tint;
  simd::float4x4 modelMatrix;
  simd::float4x4 projectionMatrix;
};
