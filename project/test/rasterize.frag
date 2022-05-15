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
layout (depth_any) out float gl_FragDepth;

//
// We prefer to use refraction and ray-tracing for transparent effects...
void main() {
  uint32_t instanceIndex = pIndices.x;
  uint32_t geometryIndex = pIndices.y;

  // 
  InstanceInfo instanceInfo = getInstance_(instanceDrawInfo.data, 0u);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, geometryIndex);

  //
  GeometryExtData geometry = getGeometryData(geometryInfo, pIndices.z);
  GeometryExtAttrib attrib = interpolate(geometry, pBary);

  //
#ifdef TRANSLUCENT
  mat3x3 tbn = getTBN(attrib);
  tbn[0] = fullTransformNormal(instanceInfo, tbn[0], geometryIndex);
  tbn[1] = fullTransformNormal(instanceInfo, tbn[1], geometryIndex);
  tbn[2] = fullTransformNormal(instanceInfo, tbn[2], geometryIndex);
  MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), pTexcoord.xy, tbn);
#endif

  //
  const uint translucent = 
#ifdef TRANSLUCENT
  1u;
#else
  0u;
#endif

  //
  gl_FragDepth = 1.f;

  // alpha and depth depth test fail
  if (
#ifdef TRANSLUCENT
    materialPix.color[MATERIAL_ALBEDO].a < 0.01f || 
#endif
    texelFetch(textures[framebufferAttachments[0][5]], ivec2(gl_FragCoord.xy), 0).r <= (gl_FragCoord.z - 0.0001f)
  ) {
    discard;
  } else 
  {
    //
    indices = pIndices;
    baryData = vec4(pBary, 1.f);
    position = vec4(pScreen.xyz/pScreen.w, 1.f);
    texcoord = vec4(pTexcoord.xyz,1.f);
#ifdef TRANSLUCENT
    tcolor = materialPix.color[MATERIAL_ALBEDO];
#else
    tcolor = vec4(0.f.xxxx);
#endif
    gl_FragDepth = gl_FragCoord.z;

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
