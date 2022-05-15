#version 460 core

// 
#extension GL_GOOGLE_include_directive : require

//
#include "lib/native.glsl"

//
layout(location = 0) out vec4 pColor;
layout(location = 1) out vec3 pBary;
layout(location = 2) flat out uvec4 pIndices;
layout(location = 3) out vec4 pScreen;
layout(location = 4) out vec4 pTexcoord;

//
// We prefer to use refraction and ray-tracing for transparent effects...
void main() {
  uint32_t instanceIndex = instanceDrawInfo.instanceIndex;
  uint32_t geometryIndex = gl_DrawID+instanceDrawInfo.drawIndex;

  // 
  InstanceInfo instanceInfo = InstanceInfo(instanceDrawInfo.data);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, geometryIndex);
  GeometryExtData geometry = getGeometryData(geometryInfo, uint(gl_VertexIndex/3u));

  //const vec4 vertice = vertices.data[gl_VertexIndex/3u][gl_VertexIndex%3];//geometry.triData[VERTEX_VERTICES][gl_VertexIndex%3];
  const vec4 vertice = vec4(geometry.triData[VERTEX_VERTICES][gl_VertexIndex%3].xyz, 1.f);
  const vec4 texcoord = vec4(geometry.triData[VERTEX_TEXCOORD][gl_VertexIndex%3].xyz, 1.f);
  const vec4 position = vec4(fullTransform(instanceInfo, vertice, geometryIndex) * constants.lookAt, 1.f) * constants.perspective;

  // anyways, give index data for relax and chill
  pIndices = uvec4((instanceIndex&0x7FFFFFFFu), geometryIndex, gl_VertexIndex/3u, 0u);

  // majorify
#ifdef TRANSLUCENT
  pIndices.x |= 0x80000000u;
#endif

  //
  {
    gl_Position = position;
    pColor = vec4(0.f.xxx, 0.f);
    pBary = bary[gl_VertexIndex%3u];
    pTexcoord = vec4(texcoord.xyz, 1.f);
    pScreen = position;
  };

};
