#version 460 core

// 
#extension GL_GOOGLE_include_directive : require

//
#include "lib/native.glsl"

//
layout(location = 0) in vec4 pColor;
layout(location = 1) in vec3 pBary;
layout(location = 2) flat in uvec4 pIndices;
layout(location = 3) in vec4 pScreen;
layout(location = 4) in vec4 pTexcoord;

// needed for linear interpolation...
layout(location = 0) out uvec4 indices;
layout(location = 1) out vec4 baryData;
layout(location = 2) out vec4 position;
layout(location = 3) out vec4 texcoord;
layout(location = 4) out vec4 tcolor;

// CONFLICT WITH CONVERVATIVE RASTERIZATION :(
//#ifndef TRANSLUCENT
//layout (early_fragment_tests) in;
//#endif

//
// We prefer to use refraction and ray-tracing for transparent effects...
void main() {
  //
  const uint translucent = 
#ifdef TRANSLUCENT
  1u;
#else
  0u;
#endif

  // 
  InstanceInfo instanceInfo = getInstance(instancedData, translucent, pIndices.x);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, pIndices.y);
  GeometryExtData geometry = getGeometryData(geometryInfo, pIndices.z);
  GeometryExtAttrib attrib = interpolate(geometry, pBary);

  //
#ifdef TRANSLUCENT
  mat3x3 tbn = getTBN(attrib);
  tbn[0] = fullTransformNormal(instanceInfo, tbn[0], pIndices.y);
  tbn[1] = fullTransformNormal(instanceInfo, tbn[1], pIndices.y);
  tbn[2] = fullTransformNormal(instanceInfo, tbn[2], pIndices.y);
  MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), pTexcoord.xy, tbn);
#endif

  // alpha and depth depth test fail
  const float dp = texelFetch(textures[framebufferAttachments[translucent][5]], ivec2(gl_FragCoord.xy), 0).r;
  if (
#ifdef TRANSLUCENT
    materialPix.color[MATERIAL_ALBEDO].a < 0.01f || 
#endif
    dp <= (gl_FragCoord.z - 0.0001f) 

  ) {
    discard;
  } else 
  {
    // 
    const uint rasterId = atomicAdd(counters[RASTER_COUNTER], 1);//subgroupAtomicAdd(RASTER_COUNTER);
    if (rasterId < extent.x * extent.y * 16) {
      const uint oldId = imageAtomicExchange(imagesR32UI[pingpong.images[/*translucent*/0]], ivec2(gl_FragCoord.xy), rasterId+1);
      RasterInfoRef rasterInfo = getRasterInfo(rasterId);
      rasterInfo.indices = uvec4(pIndices.xyz, oldId);
      rasterInfo.barycentric = vec4(pBary, 1.f);
    };
  };

};
