#ifndef INTERFACE_DEF
#define INTERFACE_DEF

//
#ifdef USE_ATOMIC_FLOAT
#define IMGS imagesR32F
#define TYPE vec4
#else
#define IMGS imagesR32UI
#define TYPE uvec4
#endif

//
struct Constants
{
  mat4x4 perspective;
  mat4x4 perspectiveInverse;

  //
  mat3x4 lookAt[2];
  mat3x4 lookAtInverse[2];
};


// 
layout(set = 0, binding = 0, scalar) uniform MatrixBlock
{
  SwapchainStateInfo swapchain;
  PingPongStateInfo deferredBuf;
  PingPongStateInfo rasterBuf;
  FramebufferStateInfo framebuffers[2];
  FramebufferStateInfo dynamicRaster;
  Constants constants;
  uvec2 rayCount;

  uint32_t r0;
  uint32_t frameCounter;
  uint32_t background;
  uint32_t blueNoise;
  uint64_t pixelData;
  uint64_t writeData;
  uint64_t rasterData[2];
  uint64_t surfaceData;
  //uint64_t hitData; // probably, no enough GPU memory
};

//
uvec2 UR(in uvec2 res) {
  return res; // TODO: hardware method
};

//
uvec2 UR(in uint res) {
  return uvec2(res&0xFFFF, (res&0xFFFF0000)>>16); // TODO: hardware method
};

//
uvec2 UR(in u16vec2 res) {
  return res; // TODO: hardware method
};

// 
layout(set = 0, binding = 1, scalar) buffer CounterBlock
{
  uint32_t counters[4];
  uint32_t previousCounters[4];
};

// but may not to be...
// may to be inaccurate in surface
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer PixelHitInfoRef {
  uvec4 indices[2];
  vec4 origin;
  f16vec3 direct;
  f16vec3 normal;
};

// but may not to be...
// may to be inaccurate in surface
layout(buffer_reference, scalar, buffer_reference_align = 1) buffer RayHitInfoRef {
  uvec4 indices[2];
  vec4 origin;
  f16vec3 direct;
  f16vec3 normal;
  f16vec4 color;
};

// but may not to be...
// TODO: replace by image buffers
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer PixelSurfaceInfoRef {
  f16vec4 tex[4];
  TYPE accum[3];
  TYPE color[3];
  uvec4 flags;
};

//
layout(buffer_reference, scalar, buffer_reference_align = 1) buffer RasterInfoRef {
  uvec4 indices;
  vec4 barycentric;
  uvec4 derivatives; // f16[dUx, dUy], f16[dVx, dVy], f16[dWx, dWy], f32[dDx, dDy]
};

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer TransformBlock {
  mat3x4 transform[];
};

//
//const uvec2 rayCount = uvec2(1280, 720);

//
uint subgroupAtomicAdd(in uint counterId) {
  const uvec4 allc = subgroupBallot(true);
  const uint sum = subgroupBallotBitCount(allc);
  uint latest = 0u;
  if (subgroupElect()) { latest = atomicAdd(counters[counterId], sum); };
  latest = subgroupBroadcast(latest, subgroupBallotFindLSB(allc));
  return (latest + subgroupBallotExclusiveBitCount(allc));
};

//
PixelHitInfoRef getNewHit(in uint pixelId, in uint type) { 
  const uint hitId = pixelId + UR(deferredBuf.extent).x * UR(deferredBuf.extent).y * type;
  return PixelHitInfoRef(pixelData) + hitId;
};

//
PixelHitInfoRef getRpjHit(in uint pixelId, in uint type) { 
  const uint hitId = pixelId + UR(deferredBuf.extent).x * UR(deferredBuf.extent).y * type;
  return PixelHitInfoRef(writeData) + hitId;
};

//
//RayHitInfoRef getHitInfo(in uint hitId)  { return RayHitInfoRef(hitData) + hitId; };
PixelSurfaceInfoRef getPixelSurface(in uint pixelId)  { return PixelSurfaceInfoRef(surfaceData) + pixelId; };
RasterInfoRef getRasterInfo(in uint rasterId, in uint previous)  { return RasterInfoRef(rasterData[previous]) + rasterId; };

//
float r_min_distance(in uint rasterId, in uint previous) {
  RasterInfoRef rasterInfo = getRasterInfo(rasterId, previous);
  return rasterInfo.barycentric.w + qdMin(unpackHalf2x16(rasterInfo.derivatives.w));
};

//
float r_max_distance(in uint rasterId, in uint previous) {
  RasterInfoRef rasterInfo = getRasterInfo(rasterId, previous);
  return rasterInfo.barycentric.w + qdMax(unpackHalf2x16(rasterInfo.derivatives.w));
};


//
struct GeometryExtData {
  mat3x4 triData[MAX_VERTEX_DATA];
};

//
struct GeometryExtAttrib {
  vec4 data[MAX_VERTEX_DATA];
};

//
uint blueNoiseFn(in uvec2 coord) {
  return pack32(u16vec2(coord));
};

#endif
