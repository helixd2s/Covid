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
TYPE accumulate(inout PixelSurfaceInfoRef surfaceInfo, in uint type, in TYPE data) {
  return TYPE(
    atomicAdd(surfaceInfo.color[type].x, data.x),
    atomicAdd(surfaceInfo.color[type].y, data.y),
    atomicAdd(surfaceInfo.color[type].z, data.z),
    atomicAdd(surfaceInfo.color[type].w, data.w)
  );
};

/*
//
TYPE accumulateDebug(inout PixelSurfaceInfoRef surfaceInfo, in uint type, in TYPE data) {
  return TYPE(
    atomicAdd(surfaceInfo.debug[type].x, data.x),
    atomicAdd(surfaceInfo.debug[type].y, data.y),
    atomicAdd(surfaceInfo.debug[type].z, data.z),
    atomicAdd(surfaceInfo.debug[type].w, data.w)
  );
};*/

//
TYPE exchange(inout PixelSurfaceInfoRef surfaceInfo, in uint type, in TYPE data) {
  return TYPE(
    atomicExchange(surfaceInfo.color[type].x, data.x),
    atomicExchange(surfaceInfo.color[type].y, data.y),
    atomicExchange(surfaceInfo.color[type].z, data.z),
    atomicExchange(surfaceInfo.color[type].w, data.w)
  );
};

#ifndef USE_ATOMIC_FLOAT
//
vec4 exchange(inout PixelSurfaceInfoRef surfaceInfo, in uint type, in vec4 data_) {
  const uvec4 data = cvtRgb16Float(data_);
  return cvtRgb16Acc(uvec4(
    atomicAdd(surfaceInfo.color[type].x, data.x),
    atomicAdd(surfaceInfo.color[type].y, data.y),
    atomicAdd(surfaceInfo.color[type].z, data.z),
    atomicAdd(surfaceInfo.color[type].w, data.w)
  ));
};

//
vec4 exchange(inout PixelSurfaceInfoRef surfaceInfo, in uint type, in vec4 data_) {
  const uvec4 data = cvtRgb16Float(data_);
  return cvtRgb16Acc(uvec4(
    atomicExchange(surfaceInfo.color[type].x, data.x),
    atomicExchange(surfaceInfo.color[type].y, data.y),
    atomicExchange(surfaceInfo.color[type].z, data.z),
    atomicExchange(surfaceInfo.color[type].w, data.w)
  ));
};
#endif

#endif