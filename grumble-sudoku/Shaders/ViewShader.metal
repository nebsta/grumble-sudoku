//
//  VertexShader.metal
//  grumble-sudoku
//
//  Created by Benjamin Wallis on 18/07/2023.
//

#include <metal_stdlib>
using namespace metal;

struct VertexUniforms {
  float4 tint;
  float4x4 modelMatrix;
  float4x4 projectionMatrix;
};

struct v2f {
  float4 position [[position]];
  half4 color;
  float2 texCoord;
};

struct VertexData {
  float3 position;
  float2 texCoord;
};

v2f vertex vertexMain(device const VertexData* vertexData [[buffer(0)]],
                      constant VertexUniforms &uniforms [[buffer(1)]],
                      uint vertexId [[vertex_id]]) {
  v2f o;
  
  float4 pos = float4(vertexData[vertexId].position, 1.0);
  o.position = uniforms.projectionMatrix * uniforms.modelMatrix * pos;
  o.color = half4(uniforms.tint.rgba);
  o.texCoord = vertexData[vertexId].texCoord.xy;
  return o;
}

half4 fragment fragmentMain(v2f in [[stage_in]], texture2d<half, access::sample> tex [[texture(0)]] ) {
  return half4(in.color);
  
//  constexpr sampler s(address::repeat, filter::linear );
//  return half4(tex.sample(s, in.texCoord).rgb, 1.0);
}
