#version 460 core

// 
#extension GL_GOOGLE_include_directive : require

//
#include "lib/native.glsl"

//
layout(location = 0) out vec4 pcolor;

//
const vec2 positions[6] = {
    vec2(0.f, 0.f), vec2(1.f, 0.f), vec2(0.f, 1.f),
    vec2(1.f, 1.f), vec2(0.f, 1.f), vec2(1.f, 0.f),
};

// 
void main() {
  uint32_t instanceIndex = 0u;
  uint32_t geometryIndex = gl_DrawID+instanceDrawInfo.drawIndex;

  // 
  InstanceInfo instanceInfo = instanceDrawInfo.data.infos[instanceIndex];
  GeometryInfo geometryInfo = instanceInfo.data.infos[geometryIndex];
  GeometryExtData geometry = getGeometryData(geometryInfo, uint(gl_VertexIndex/3u));

  //const vec4 vertice = vertices.data[gl_VertexIndex/3u][gl_VertexIndex%3];//geometry.triData[VERTEX_VERTICES][gl_VertexIndex%3];
  const vec4 vertice = vec4(geometry.triData[VERTEX_VERTICES][gl_VertexIndex%3].xyz, 1.f);
  const vec4 texcoord = vec4(geometry.triData[VERTEX_TEXCOORD][gl_VertexIndex%3].xyz, 1.f);
  const vec4 position = vec4(fullTransform(instanceInfo, vertice, geometryIndex) * constants.lookAt, 1.f) * constants.perspective;

  // 
  gl_Position = position;
  pcolor = vec4(texcoord.xyz, 1.f);
};
