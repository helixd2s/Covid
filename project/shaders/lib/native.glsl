#ifndef NATIVE_DEF
#define NATIVE_DEF

//
#define USE_ATOMIC_FLOAT

// 
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_buffer_reference_uvec2 : require
//#extension GLSL_EXT_buffer_reference : require
//#extension GLSL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require
#extension GL_EXT_ray_query : enable
#extension GL_EXT_ray_tracing : enable
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_EXT_debug_printf : require
#extension GL_EXT_control_flow_attributes : require
#extension GL_EXT_shader_realtime_clock : require
#extension GL_KHR_shader_subgroup_ballot : require
//#extension GL_ARB_conservative_depth : require
#extension GL_EXT_shared_memory_block : require
#extension GL_EXT_fragment_shader_barycentric : require

#ifdef USE_ATOMIC_FLOAT
#extension GL_EXT_shader_atomic_float : require
#endif

//
struct InstanceAddressInfo {
  //InstanceData data;
  uint64_t data;
  uint64_t accelStruct;
  uint32_t instanceCount;
  uint32_t reserved;
};

// ALWAYS USE Alter INDEX OF `InstanceDrawDatas`
struct InstanceDrawInfo {
  //InstanceData data;
  uint32_t instanceCount;
  uint32_t instanceIndex;
  uint32_t reserved0;
  uint32_t drawIndex;
};

//
struct InstanceAddressBlock {
  InstanceAddressInfo addressInfos[2];
};

//
struct PingPongStateInfo {
  uvec2 extent;
  uint32_t images[2][6];
  uint32_t previous;
  uint32_t index;
};

//
struct SwapchainStateInfo {
  uvec2 extent;
  uint32_t image;
  uint32_t index;
};

//
struct FramebufferStateInfo {
  uvec2 extent;
  uint32_t attachments[2][8]; // framebuffers
};

//
layout(push_constant, scalar, buffer_reference_align = 1) uniform PConstBlock {
  InstanceAddressBlock instancedData;
  InstanceDrawInfo instanceDrawInfo;
};

//
layout(set = 1, binding = 0) uniform texture2D textures[];
layout(set = 1, binding = 0) uniform utexture2D texturesU[];
layout(set = 2, binding = 0) uniform sampler samplers[];
layout(set = 3, binding = 0, rgb10_a2) uniform image2D images[];
layout(set = 3, binding = 0, rgba32f) uniform image2D imagesRgba32F[];
layout(set = 3, binding = 0, rgba16f) uniform image2D imagesRgba16F[];
layout(set = 3, binding = 0, rgba32ui) uniform uimage2D imagesRgba32UI[];
layout(set = 3, binding = 0, r32ui) uniform uimage2D imagesR32UI[];
layout(set = 3, binding = 0, r32f) uniform image2D imagesR32F[];

//
vec4 textureBilinear( in uint image, in vec2 uv )
{
  const vec2 res = imageSize( imagesRgba32F[image] );
  const vec2 st = uv*res - 0.5;
  const vec2 iuv = floor( st );
  const vec2 fuv = fract( st );
  const vec4 a = imageLoad( imagesRgba32F[image], ivec2(iuv+vec2(0.5,0.5)) );
  const vec4 b = imageLoad( imagesRgba32F[image], ivec2(iuv+vec2(1.5,0.5)) );
  const vec4 c = imageLoad( imagesRgba32F[image], ivec2(iuv+vec2(0.5,1.5)) );
  const vec4 d = imageLoad( imagesRgba32F[image], ivec2(iuv+vec2(1.5,1.5)) );
  return mix( mix( a, b, fuv.x), mix( c, d, fuv.x), fuv.y );
};

//
#include "./constants.glsl"
#include "./utils.glsl"
#include "./interface.glsl"
#include "./atomic.glsl"
#include "./geometry.glsl"
#include "./material.glsl"

#endif
