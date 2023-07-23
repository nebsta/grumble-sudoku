//
//  VertexShader.metal
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#include <metal_stdlib>
using namespace metal;

struct VertexUniforms {
  float4x4 projectionMatrix;
  float4x4 modelMatrix;
  float4 tint;
};

struct v2f {
  float4 position [[position]];
  half3 color;
};

struct VertexData {
  float3 position;
};

v2f vertex vertexMain(device const VertexData* vertexData [[buffer(0)]],
                      constant VertexUniforms &uniforms [[buffer(1)]],
                      uint vertexId [[vertex_id]]) {
  v2f o;
  
  float4 pos = float4(vertexData[vertexId].position, 1.0);
  o.position = uniforms.projectionMatrix * uniforms.modelMatrix * pos;
  o.color = half3(uniforms.tint.rgb);
  return o;
}

half4 fragment fragmentMain(v2f in [[stage_in]]) {
  return half4(in.color, 1.0);
}
