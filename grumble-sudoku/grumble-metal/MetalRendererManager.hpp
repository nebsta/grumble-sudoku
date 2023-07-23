//
//  MetalRendererManager.hpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 22/07/2023.
//

#pragma once

#include <memory>

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

#include <grumble/render/RendererManager.hpp>

#include "Buffers/UniformData.hpp"
#include "MetalUtils.hpp"

#define MAX_FRAMES_IN_FLIGHT 3

class MetalRendererManager: public grumble::RendererManager {
  
public:
  MetalRendererManager(MTL::Device* device, MTK::View *mtkView);
  ~MetalRendererManager() override;
  
  void buildShaders();
  void buildBuffers();
  
  MTL::CommandBuffer* commandBuffer();
  
  void render(std::shared_ptr<grumble::View> view) override;
  void screenSizeUpdated(CGSize size);
  
private:
  MTK::View* _mtkView;
  MTL::CommandBuffer* _commandBuffer;
  
  MTL::Device* _device;
  MTL::CommandQueue* _commandQueue;
  MTL::RenderPipelineState* _pipelineState;
  MTL::Library* _shaderLibrary;

  MTL::Buffer* _vertexPositionsBuffer;
  MTL::Buffer* _vertexColorsBuffer;
  
  std::array<MTL::Buffer*, MAX_FRAMES_IN_FLIGHT> _uniformBuffers;
  
  int _activeFrameIndex;
  simd::float4x4 _projectionMatrix;
};
