//
//  BufferInstanceData.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 23/07/2023.
//

#pragma once

#include <simd/simd.h>

struct BufferInstanceData {
    simd::float4x4 transform;
    simd::float4 color;
};
