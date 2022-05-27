#version 460 core

// 
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_spirv_intrinsics : require
#extension GL_ARB_fragment_shader_interlock : require

//
#include "lib/native.glsl"

//
layout (location = 0) in vec4 pColor;
layout (location = 1) in flat uvec4 pIndices;
layout (location = 2) in vec4 pScreen;
layout (location = 3) in vec4 pTexcoord;
layout (location = 4) in mat3x3 pTbn;

//
//layout (early_fragment_tests) in;
layout (depth_any) out float gl_FragDepth;
layout (pixel_interlock_ordered) in;

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
  const vec3 pBary = gl_BaryCoordEXT;

  //
  //const uvec4 pIndices = pIndices_[0];
  //const vec4 pScreen = cvt3x4(pScreen_) * pBary;
  //const vec4 pColor = cvt3x4(pColor_) * pBary;
  //const vec4 pTexcoord = cvt3x4(pTexcoord_) * pBary;

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

  //
#ifdef TRANSLUCENT
  const mat3x3 tbn = mat3x3(normalize(pTbn[0]), normalize(pTbn[1]), normalize(pTbn[2]));
  MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), pTexcoord.xy, tbn);
#endif

  // alpha and depth depth (manually)
beginInvocationInterlockARB();
  const float dc = gl_FragCoord.z + mxD;
  const float dp = imageLoad(imagesR32F[rasterBuf.images[0][1]], ivec2(gl_FragCoord.xy)).r;//texelFetch(textures[framebuffers[2].attachments[0][5]], ivec2(gl_FragCoord.xy), 0).r;
  const bool cm = (dc - 0.001f <= dp);//|| !(all(greaterThan(pBary, 1e-9.xxx)) && all(lessThan(pBary, 1.f.xxx+1e-9)));
#ifdef TRANSLUCENT
  const bool invalidHit = materialPix.color[MATERIAL_ALBEDO].a < 0.01f || !cm;
#else
  const bool invalidHit = !cm;
#endif
  if (!invalidHit) {
    imageStore(imagesR32F[rasterBuf.images[0][1]], ivec2(gl_FragCoord.xy), vec4(dc, 0.f.xxx)); 
  };
endInvocationInterlockARB();

  //
  if (!invalidHit) {
    // 
    const uint rasterId = atomicAdd(counters[RASTER_COUNTER], 1);//subgroupAtomicAdd(RASTER_COUNTER);
    if (rasterId < UR(rasterBuf.extent).x * UR(rasterBuf.extent).y * 32) {
      const uint oldId = imageAtomicExchange(imagesR32UI[rasterBuf.images[0][0]], ivec2(gl_FragCoord.xy), rasterId+1);

      //
      RasterInfoRef rasterInfo = getRasterInfo(rasterId, 0);
      rasterInfo.indices = uvec4(pIndices.xyz, oldId);
      rasterInfo.barycentric = vec4(pBary, depth);
      rasterInfo.derivatives = derrivative;
    };
  };
  
  //
  discard;
};
