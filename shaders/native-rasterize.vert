#version 460 core

// 
#extension GL_GOOGLE_include_directive : require

//
layout(location = 0) flat out uint drawId;
layout(location = 1) flat out uint instanceId;

//
// We prefer to use refraction and ray-tracing for transparent effects...
void main() {
  drawId = gl_DrawID;
  instanceId = gl_InstanceIndex;
};