#version 460 core

// 
#extension GL_GOOGLE_include_directive : require

//
#include "lib/native.glsl"

//
layout(location = 0) in vec4 pColor;
layout(location = 1) in vec3 pBary;
layout(location = 2) flat in uvec4 pIndices;
//
layout(location = 0) out vec4 baryData;
layout(location = 1) out uvec4 indices;

//
#ifndef TRANSLUCENT
layout(early_fragment_tests) in;
#endif

// 
void main() {

  //
  uint32_t instanceIndex = pIndices.x;
  uint32_t geometryIndex = pIndices.y;

  // 
  InstanceInfo instanceInfo = getInstance(instancedData.opaqueAddressInfo, instanceIndex); //getInstance(instanceDrawInfo.data, 0u);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, geometryIndex);

  // if translucent - discard!
#ifdef TRANSLUCENT
  if ((geometryInfo.flags&1u) == 0u) { discard; };
#else
  if ((geometryInfo.flags&1u) <= 0u) { discard; };
#endif

  //
  GeometryExtData geometry = getGeometryData(geometryInfo, pIndices.z);
  GeometryExtAttrib attrib = interpolate(geometry, pBary);

  //
  const vec4 vertice = attrib.data[VERTEX_VERTICES];
  const vec4 texcoord = attrib.data[VERTEX_TEXCOORD];

  //
  mat3x3 tbn = getTBN(attrib);
  MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), texcoord.xy, tbn);

  // alpha test failure
#ifdef TRANSLUCENT
  if (materialPix.color[MATERIAL_ALBEDO].a < 0.001f) { discard; };
#endif

  //
  baryData = vec4(pBary, 1.f);
  indices = pIndices;
};
