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

  // TODO: make array of
  mat3x4 lookAt;
  mat3x4 previousLookAt;

  // TODO: make array of
  mat3x4 lookAtInverse;
  mat3x4 previousLookAtInverse;
};

// 
layout(set = 0, binding = 0, scalar) uniform MatrixBlock
{
  uint32_t framebufferAttachments[2][2][8]; // framebuffers
  uvec2 extent; uint frameCounter, reserved0;
  Constants constants;
  uint64_t pixelData;
  uint64_t writeData;
  uint64_t rasterData;
  uint64_t surfaceData;
  uint64_t prevRasterData;
  uint32_t background;
  uint32_t blueNoise;
};

// 
layout(set = 0, binding = 1, scalar) buffer CounterBlock
{
  uint32_t counters[4];
  uint32_t previousCounters[4];
};

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer PixelHitInfoRef {
  uvec4 indices;
  vec4 origin;
  //vec4 direct;
};

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer PixelSurfaceInfoRef {
  uvec4 indices;
  vec3 origin;
  vec3 normal;
  vec4 tex[2];
  TYPE accum[3];
  TYPE color[3];
};

//
layout(buffer_reference, scalar, buffer_reference_align = 1) buffer RasterInfoRef {
  uvec4 indices;
  vec4 barycentric;
};

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer TransformBlock {
  mat3x4 transform[];
};

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
//#define sizeof(Type) uint64_t(Type(uint64_t(0))+1)

//
PixelSurfaceInfoRef getPixelSurface(in uint pixelId)  { return PixelSurfaceInfoRef(surfaceData) + pixelId; };

//
PixelHitInfoRef getNewHit(in uint pixelId, in uint type) { 
  const uint hitId = pixelId + extent.x * extent.y * type;
  return PixelHitInfoRef(pixelData) + hitId;
};

//
PixelHitInfoRef getRpjHit(in uint pixelId, in uint type) { 
  const uint hitId = pixelId + extent.x * extent.y * type;
  return PixelHitInfoRef(writeData) + hitId;
};

//
RasterInfoRef getRasterInfo(in uint rasterId)  { return RasterInfoRef(rasterData) + rasterId; };
RasterInfoRef getPrevRasterInfo(in uint rasterId)  { return RasterInfoRef(prevRasterData) + rasterId; };

//
struct GeometryExtData {
  mat3x4 triData[MAX_VERTEX_DATA];
};

//
struct GeometryExtAttrib {
  vec4 data[MAX_VERTEX_DATA];
};

//
uvec2 blueNoiseFn(in uvec2 coord) {
  //const ivec2 texSize = textureSize(texturesU[blueNoise], 0);
  //return texelFetch(texturesU[blueNoise], ivec2(coord) % texSize, 0).r;
  return coord;
};

#endif
