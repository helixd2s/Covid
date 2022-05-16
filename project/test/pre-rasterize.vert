#version 460 core

// 
#extension GL_GOOGLE_include_directive : require

//
#include "lib/native.glsl"

//
layout(location = 0) flat out uint drawId;

//
// We prefer to use refraction and ray-tracing for transparent effects...
void main() {
  uint32_t instanceIndex = instanceDrawInfo.instanceIndex;
  drawId = gl_DrawID;
};
