#version 460 core

// 
#extension GL_GOOGLE_include_directive : require

//
#include "lib/native.glsl"

//
layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

//
layout(location = 0) flat in uint drawId[];
layout(location = 1) flat in uint instanceId[];

//
layout(location = 0) out flat uvec4 pIndices;
layout(location = 1) out vec4 pColor;
layout(location = 2) out vec4 pScreen;
layout(location = 3) out vec4 pTexcoord;
layout(location = 4) out mat3x3 pTbn;

  //
  const uint translucent = 
#ifdef TRANSLUCENT
  1u;
#else
  0u;
#endif

//
// We prefer to use refraction and ray-tracing for transparent effects...
void main() {

  //
  uint32_t geometryIndex = instanceDrawInfo.drawIndex + drawId[0];
  uint32_t instanceIndex = instanceDrawInfo.instanceIndex + instanceId[0];
  InstanceInfo instanceInfo = getInstance(instancedData, translucent, instanceIndex);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, geometryIndex);
  GeometryExtData geometry = getGeometryData(geometryInfo, gl_PrimitiveIDIn);

  //
  [[unroll]] for (uint i=0;i<3;i++) {
    vec4 vertice = vec4(geometry.triData[VERTEX_VERTICES][i].xyz, 1.f);
    vec4 texcoord = vec4(geometry.triData[VERTEX_TEXCOORD][i].xyz, 1.f);
    fullTransform(instanceInfo, vertice, geometryIndex, 0);
    vec4 position = vec4(vertice * constants.lookAt[0], 1.f) * constants.perspective;

    //
    gl_Position = position;

    // anyways, give index data for relax and chill
    pIndices = uvec4(instanceIndex, geometryIndex, gl_PrimitiveIDIn, 0u);

    // majorify
  #ifdef TRANSLUCENT
    pIndices.x |= 0x80000000u;
  #endif

    //
    fullTransformNormal(instanceInfo, geometry.triData[VERTEX_TANGENT][i].xyz, geometryIndex, 0);
    fullTransformNormal(instanceInfo, geometry.triData[VERTEX_BITANGENT][i].xyz, geometryIndex, 0); 
    fullTransformNormal(instanceInfo, geometry.triData[VERTEX_NORMALS][i].xyz, geometryIndex, 0);

    //
    pTbn = mat3x3(geometry.triData[VERTEX_TANGENT][i].xyz, geometry.triData[VERTEX_BITANGENT][i].xyz, geometry.triData[VERTEX_NORMALS][i].xyz);

    //
    pColor = vec4(0.f.xxx, 0.f);
    pTexcoord = vec4(texcoord.xyz, 1.f);
    pScreen = position;

    //
    EmitVertex();
  };

  EndPrimitive();
};