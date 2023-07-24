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
#define MAX_INSTANCES 500

class MetalRendererManager: public grumble::RendererManager {
  typedef std::array<MTL::Buffer*, MAX_INSTANCES> UniformInstanceBuffers;
  
public:
  MetalRendererManager(MTL::Device* device, MTK::View *mtkView);
  ~MetalRendererManager() override;
  
  void buildShaders();
  void buildBuffers();
  
  MTL::CommandBuffer* generateCommandBuffer();
  void finishFrame();
  
  void setActiveFrame(int index);
  void render(std::shared_ptr<grumble::View> view) override;
  void screenSizeUpdated(CGSize size);
  
private:
  MTK::View* _mtkView;
  MTL::CommandBuffer* _currentCommandBuffer;
  MTL::RenderCommandEncoder* _currentCommandEncoder;
  
  MTL::Device* _device;
  MTL::CommandQueue* _commandQueue;
  MTL::RenderPipelineState* _pipelineState;
  MTL::Library* _shaderLibrary;

  MTL::Buffer* _vertexPositionsBuffer;
  
  std::array<UniformInstanceBuffers, MAX_FRAMES_IN_FLIGHT> _uniformBuffers;
  
  int _activeFrameIndex;
  int _instanceIndex;
  simd::float4x4 _projectionMatrix;
};
