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
layout(early_fragment_tests) in;

// 
void main() {
  baryData = vec4(pBary, 1.f);
  indices = pIndices;

  //
  uint32_t instanceIndex = pIndices.x;
  uint32_t geometryIndex = pIndices.y;

  // 
  InstanceInfo instanceInfo = instanceDrawInfo.data.infos[instanceIndex];
  GeometryInfo geometryInfo = instanceInfo.data.infos[geometryIndex];
  GeometryExtData geometry = getGeometryData(geometryInfo, pIndices.z);

  // if translucent - discard!
  if ((geometryInfo.flags&1u) <= 0u) { discard; };

  //
  GeometryExtAttrib attrib = interpolate(geometry, pBary);
};
