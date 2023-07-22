//
//  MetalRenderer.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 21/07/2023.
//

#pragma once

#include <grumble/render/Renderer.hpp>
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

class MetalRenderer: public grumble::Renderer {
protected:
  virtual void preDraw() override;
  virtual void draw() override;
  virtual void onPostDraw() override;
};
