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
  gl_Position = vec4(positions[gl_VertexIndex] * 2.f - 1.f, 0.1f, 1.f);
  pcolor = vec4(positions[gl_VertexIndex], 0.1f, 1.f);
};
