#ifndef ATOMIC_DEF
#define ATOMIC_DEF

//
#include "./interface.glsl"

//
#ifdef USE_ATOMIC_FLOAT
vec4 cvtRgb16Acc(in vec4 color) {
  return color;
};

//
vec4 cvtRgb16Float(in vec4 sampled) {
  return sampled;
};
#else
vec4 cvtRgb16Acc(in uvec4 color) {
  const vec4 minor = (color & 0xFFFF) / 65536.f;
  const vec4 major = ((color & 0xFFFF0000) >> 16);
  return major + minor;
};

//
uvec4 cvtRgb16Float(in vec4 sampled) {
  return uvec4(sampled * 65536.f);
};
#endif

//
void accumulate(inout PixelSurfaceInfoRef surfaceInfo, in uint type, in TYPE data) {
  atomicAdd(surfaceInfo.color[type].x, data.x);
  atomicAdd(surfaceInfo.color[type].y, data.y);
  atomicAdd(surfaceInfo.color[type].z, data.z);
  atomicAdd(surfaceInfo.color[type].w, data.w);
};

#ifndef USE_ATOMIC_FLOAT
//
void accumulate(inout PixelSurfaceInfoRef surfaceInfo, in uint type, in vec4 data_) {
  const uvec4 data = cvtRgb16Float(data_);
  atomicAdd(surfaceInfo.color[type].x, data.x);
  atomicAdd(surfaceInfo.color[type].y, data.y);
  atomicAdd(surfaceInfo.color[type].z, data.z);
  atomicAdd(surfaceInfo.color[type].w, data.w);
};
#endif

#endif