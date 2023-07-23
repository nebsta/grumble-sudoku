//
//  MetalRenderer.cpp
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 21/07/2023.
//

#include "MetalRenderer.hpp"
//
//void MetalRenderer::MetalRenderer(MTK::View* view, MTL::CommandBuffer* commandBuffer) {
//  _commandBuffer = commandBuffer;
//  _view = view;
//}
//
//void MetalRenderer::draw() {
//  if (_commandBuffer == nullptr || _view == nullptr) {
//    logError("Trying to draw MetalRenderer before setting up!");
//    exit(0);
//    return;
//  }
//  
//  MTL::RenderPassDescriptor* renderPassDesc = _view->currentRenderPassDescriptor();
//  MTL::RenderCommandEncoder* renderCommEncoder = _commandBuffer->renderCommandEncoder(renderPassDesc);
//  
//  renderCommEncoder->setRenderPipelineState(_pipelineState);
//  renderCommEncoder->setVertexBuffer(_argumentBuffer, 0, 0);
//  renderCommEncoder->useResource(_vertexPositionsBuffer, MTL::ResourceUsageRead);
//  renderCommEncoder->useResource(_vertexColorsBuffer, MTL::ResourceUsageRead);
//  
//  renderCommEncoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(4));
//  
//  
//  renderCommEncoder->endEncoding();
//}
//
//void MetalRenderer::onPostDraw() {
//  
//}
