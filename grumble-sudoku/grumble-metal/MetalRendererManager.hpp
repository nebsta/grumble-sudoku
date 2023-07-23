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

#include "MetalRenderer.hpp"
#include "BufferInstanceData.hpp"
#include "MetalUtils.hpp"

#define MAX_FRAMES_IN_FLIGHT 3
#define MAX_INSTANCES 100

class MetalRendererManager: public grumble::RendererManager {
public:
  MetalRendererManager(MTL::Device* device);
  ~MetalRendererManager() override;
  
  void buildShaders();
  void buildBuffers();
  
  MTL::CommandQueue* commandQueue();
  MTL::CommandBuffer* commandBuffer();
  
  void frameSetup(MTK::View* mtkView, MTL::CommandBuffer* commandBuffer);
  
  void render(std::shared_ptr<grumble::View> view) override;
  
private:
  MTK::View* _mtkView;
  MTL::CommandBuffer* _commandBuffer;
  
  MTL::Device* _device;
  MTL::CommandQueue* _commandQueue;
  MTL::RenderPipelineState* _pipelineState;
  MTL::Library* _shaderLibrary;

  MTL::Buffer* _argumentBuffer;
  MTL::Buffer* _vertexPositionsBuffer;
  MTL::Buffer* _vertexColorsBuffer;
  MTL::Buffer* _instanceDataBuffer[MAX_FRAMES_IN_FLIGHT];
};
