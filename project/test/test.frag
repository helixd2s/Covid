#version 460 core

// 
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

// 
layout(set = 0, binding = 0, scalar) uniform MatrixBlock
{
  uint32_t imageIndices[4];
  uint32_t textureIndices[4];
  uint32_t currentImage;
  uint32_t reserved;
};

//
layout(set = 1, binding = 0) uniform texture2D textures[];
layout(set = 2, binding = 0) uniform sampler samplers[];
layout(set = 3, binding = 0, rgba32f) uniform image2D images[];

//
layout(location = 0) in vec4 pcolor;

//
layout(location = 0) out vec4 albedo;

// 
void main() {
  albedo = pcolor;
  gl_FragDepth = 0.f;
  //imageStore(images[imageIndices[currentImage]], ivec2(gl_FragCoord.xy), albedo);
};
