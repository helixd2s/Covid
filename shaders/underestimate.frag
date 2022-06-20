#version 460 core

// 
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_spirv_intrinsics : require

//
#include "lib/native.glsl"

//
layout (location = 0) in flat uvec4 pIndices;
layout (location = 1) in vec4 pColor;
layout (location = 2) in vec4 pScreen;
layout (location = 3) in vec4 pTexcoord;
layout (location = 4) in mat3x3 pTbn;

// needed for linear interpolation...
layout (location = 0) out uvec4 oIndices;
layout (location = 1) out uvec4 oDerivative;
layout (location = 2) out vec4 oBaryData;
layout (location = 3) out vec4 oPosition;
layout (location = 4) out vec4 oColor;

// CONFLICT WITH CONVERVATIVE RASTERIZATION :(
#ifndef TRANSLUCENT
layout (early_fragment_tests) in;
#endif

// 
layout (depth_any) out float gl_FragDepth;

  //
  const uint translucent = 
#ifdef TRANSLUCENT
  1u;
#else
  0u;
#endif

//
// We prefer to use refraction and ray-tracing for transparent effects...
void main() {
  const vec3 pBary = gl_BaryCoordEXT;
  const float depth = pScreen.z/pScreen.w;
  const uvec4 derrivative = uvec4(
    packHalf2x16(vec2(dFdx(pBary.x), dFdy(pBary.x))),
    packHalf2x16(vec2(dFdx(pBary.y), dFdy(pBary.y))),
    packHalf2x16(vec2(dFdx(pBary.z), dFdy(pBary.z))),
    packHalf2x16(vec2(dFdx(depth), dFdy(depth)))
  );

  // minimal depth shifting
  const vec2 dD = vec2(dFdx(gl_FragCoord.z), dFdy(gl_FragCoord.z));
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

  //
  const float dp = texelFetch(textures[framebuffers[0].attachments[0][5]], ivec2(gl_FragCoord.xy), 0).r;

  //
  if (
#ifdef TRANSLUCENT
    materialPix.color[MATERIAL_ALBEDO].a < 0.01f || 
#endif
    (dp + 0.0001f) <= (gl_FragCoord.z + mxD)
  ) { discard; } else 
  { //
    oIndices = pIndices;
    oBaryData = vec4(pBary, depth);;
    oPosition = vec4(pScreen.xyz/pScreen.w, 1.f);
    oDerivative = derrivative;
#ifdef TRANSLUCENT
    oColor = materialPix.color[MATERIAL_ALBEDO];
#else
    oColor = vec4(0.f.xxx, 1.f);
#endif
    gl_FragDepth = gl_FragCoord.z + mxD;
  };

};