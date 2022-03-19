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
  InstanceInfo instanceInfo = instanceDrawInfo.data.infos[0u];
  GeometryInfo geometryInfo = instanceInfo.data.infos[/*gl_DrawID+instanceDrawInfo.drawIndex*/0u];
  GeometryExtData geometry = getGeometryData(geometryInfo, uint(gl_VertexIndex/3u));

  //const vec4 vertice = vertices.data[gl_VertexIndex/3u][gl_VertexIndex%3];//geometry.triData[VERTEX_VERTICES][gl_VertexIndex%3];
  const vec4 vertice = vec4(geometry.triData[VERTEX_VERTICES][gl_VertexIndex%3].xyz, 1.f);

  gl_Position = vertice;
  pcolor = vec4(vertice.xyz*0.5f+0.5f, 1.f);
};
