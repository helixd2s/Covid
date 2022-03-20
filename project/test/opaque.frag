#version 460 core

// 
#extension GL_GOOGLE_include_directive : require

//
#include "lib/native.glsl"

//
layout(location = 0) in vec4 pcolor;

//
layout(location = 0) out vec4 albedo;
layout(location = 1) out uvec4 indices;

//
layout(early_fragment_tests) in;

// 
void main() {
  albedo = pcolor;
  indices = uvec4(0u.xxxx);
};
