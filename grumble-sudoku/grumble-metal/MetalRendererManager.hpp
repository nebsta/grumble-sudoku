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
#include "Buffers/VertexData.hpp"
#include "Buffers/TexCoordData.hpp"

#include "MetalUtils.hpp"

#define MAX_FRAMES_IN_FLIGHT 3
#define MAX_INSTANCES 500
#define VERTEX_COUNT 4

class MetalRendererManager: public grumble::RendererManager {
  typedef std::array<MTL::Buffer*, MAX_INSTANCES> InstanceBuffers;
  
public:
  typedef std::shared_ptr<MetalRendererManager> shared_ptr;
  
  MetalRendererManager(MTL::Device* device,
                       MTK::View *mtkView,
                       grumble::SpriteManager::shared_ptr spriteManager);
  ~MetalRendererManager() override;
  
  void buildShaders();
  void buildBuffers();
  void buildTextures();
  
  MTL::CommandBuffer* generateCommandBuffer();
  void finishFrame();
  
  void setup() override;
  
  void setActiveFrame(int index);
  void renderView(grumble::Transform::shared_ptr transform, grumble::Renderer::shared_ptr) override;
  void renderImageView(grumble::Transform::shared_ptr transform, grumble::ImageRenderer::shared_ptr) override;
  void screenSizeUpdated(CGSize size);
  
private:
  grumble::SpriteManager::shared_ptr _spriteManager;
  
  MTK::View* _mtkView;
  MTL::CommandBuffer* _currentCommandBuffer;
  MTL::RenderCommandEncoder* _currentCommandEncoder;
  
  MTL::Device* _device;
  MTL::CommandQueue* _commandQueue;
  MTL::RenderPipelineState* _pipelineState;
  MTL::Library* _shaderLibrary;

  MTL::Buffer* _emptyTexCoordBuffer;
  MTL::Buffer* _vertexBuffer;
  
  std::array<InstanceBuffers, MAX_FRAMES_IN_FLIGHT> _uniformBuffers;
  std::map<std::string, MTL::Texture*> _textureBuffers;
  std::map<std::string, MTL::Buffer*> _texCoordBuffers;
  
  int _activeFrameIndex;
  int _instanceIndex;
  simd::float4x4 _projectionMatrix;
};
