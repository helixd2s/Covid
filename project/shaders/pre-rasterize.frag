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
  const float depth = pScreen.z/pScreen.w;
  const uvec4 derrivative = uvec4(
    packHalf2x16(vec2(dFdx(pBary.x), dFdy(pBary.x))),
    packHalf2x16(vec2(dFdx(pBary.y), dFdy(pBary.y))),
    packHalf2x16(vec2(dFdx(pBary.z), dFdy(pBary.z))),
    packHalf2x16(vec2(dFdx(depth), dFdy(depth)))
  );

  // minimal depth shifting
  const vec2 dD = vec2(dFdx(gl_FragCoord.z), dFdy(gl_FragCoord.z));

  // near
  const float mnD = qdMin(dD);
  const float mxD = qdMax(dD);

  // 
  InstanceInfo instanceInfo = getInstance(instancedData, translucent, pIndices.x);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, pIndices.y);
  GeometryExtData geometry = getGeometryData(geometryInfo, pIndices.z);
  GeometryExtAttrib attrib = interpolate(geometry, pBary);

  //
#ifdef TRANSLUCENT
  mat3x3 tbn = getTBN(attrib);
  tbn[0] = fullTransformNormal(instanceInfo, tbn[0], pIndices.y, 0);
  tbn[1] = fullTransformNormal(instanceInfo, tbn[1], pIndices.y, 0);
  tbn[2] = fullTransformNormal(instanceInfo, tbn[2], pIndices.y, 0);
  MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), pTexcoord.xy, tbn);
#endif

  // alpha and depth depth test fail
  const float dp = texelFetch(textures[framebufferAttachments[0][translucent][5]], ivec2(gl_FragCoord.xy), 0).r + mxD;
  if (
#ifdef TRANSLUCENT
    materialPix.color[MATERIAL_ALBEDO].a < 0.01f || 
#endif
    dp <= (gl_FragCoord.z - 0.0001f) // for optimize!
  ) {
    discard;
  } else 
  {
    // 
    const uint rasterId = atomicAdd(counters[RASTER_COUNTER], 1);//subgroupAtomicAdd(RASTER_COUNTER);
    if (rasterId < extent.x * extent.y * 16) {
      const uint oldId = imageAtomicExchange(imagesR32UI[pingpong.images[0][/*translucent*/0]], ivec2(gl_FragCoord.xy), rasterId+1);

      RasterInfoRef rasterInfo = getRasterInfo(rasterId, 0);
      rasterInfo.indices = uvec4(pIndices.xyz, oldId);
      rasterInfo.barycentric = vec4(pBary, depth);
      rasterInfo.derivatives = derrivative;
    };
  };

};
