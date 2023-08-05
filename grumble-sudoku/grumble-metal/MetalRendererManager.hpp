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
#include <png.h>

#include <grumble/render/RendererManager.hpp>
#include <grumble/render/ImageRenderer.hpp>
#include <grumble/sprite/SpriteManager.hpp>
#include <grumble/io/ImageFile.hpp>

#include "Buffers/UniformData.hpp"
#include "MetalUtils.hpp"
#include "VertexData.hpp"

#define MAX_FRAMES_IN_FLIGHT 3
#define MAX_INSTANCES 500

class MetalRendererManager: public grumble::RendererManager {
  typedef std::array<MTL::Buffer*, MAX_INSTANCES> UniformInstanceBuffers;
  
public:
  MetalRendererManager(MTL::Device* device,
                       MTK::View *mtkView,
                       std::shared_ptr<grumble::SpriteManager> spriteManager);
  ~MetalRendererManager() override;
  
  void buildShaders();
  void buildBuffers();
  void buildTextures();
  
  MTL::CommandBuffer* generateCommandBuffer();
  void finishFrame();
  
  void setup() override;
  
  void setActiveFrame(int index);
  void renderView(grumble::Transform transform, std::shared_ptr<grumble::Renderer>) override;
  void renderImageView(grumble::Transform transform, std::shared_ptr<grumble::ImageRenderer>) override;
  void screenSizeUpdated(CGSize size);
  
private:
  std::shared_ptr<grumble::SpriteManager> _spriteManager;
  
  MTK::View* _mtkView;
  MTL::CommandBuffer* _currentCommandBuffer;
  MTL::RenderCommandEncoder* _currentCommandEncoder;
  
  MTL::Device* _device;
  MTL::CommandQueue* _commandQueue;
  MTL::RenderPipelineState* _pipelineState;
  MTL::Library* _shaderLibrary;

  MTL::Buffer* _vertexBuffer;
  
  std::array<UniformInstanceBuffers, MAX_FRAMES_IN_FLIGHT> _uniformBuffers;
  std::map<std::string, MTL::Texture*> _textureBuffers;
  
  int _activeFrameIndex;
  int _instanceIndex;
  simd::float4x4 _projectionMatrix;
};
