#version 460 core

// 
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require
#extension GL_ARB_gpu_shader_int64 : require

// 
layout(set = 0, binding = 0, scalar, row_major) uniform MatrixBlock
{
  uint32_t imageIndices[4];
  uint32_t textureIndices[4];
  uint32_t currentImage;
  uint32_t reserved;
  //vec2 positions[6];
};

//
layout(set = 1, binding = 0) uniform texture2D textures[];
layout(set = 2, binding = 0) uniform sampler samplers[];
layout(set = 3, binding = 0, rgba32f) uniform image2D images[];

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
