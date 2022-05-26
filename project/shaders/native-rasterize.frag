#version 460 core

// 
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_spirv_intrinsics : require

//
#include "lib/native.glsl"

//
layout(location = 0) in vec4 pColor;
layout(location = 1) flat in uvec4 pIndices;
//spirv_decorate (extensions = ["SPV_KHR_fragment_shader_barycentric"], capabilities = [5284], 5285) layout(location = 2) in mat3x4 pScreen_;
layout(location = 2) in vec4 pScreen;
layout(location = 3) in vec4 pTexcoord;

// needed for linear interpolation...
layout(location = 0) out uvec4 indices;
layout(location = 1) out uvec4 derivatives;
layout(location = 2) out vec4 baryData;
layout(location = 3) out vec4 position;
layout(location = 4) out vec4 tcolor;

// CONFLICT WITH CONVERVATIVE RASTERIZATION :(
#ifndef TRANSLUCENT
layout (early_fragment_tests) in;
#endif

// 
layout (depth_any) out float gl_FragDepth;

// yet another vaporware
spirv_decorate (extensions = ["SPV_KHR_fragment_shader_barycentric"], capabilities = [5284], 11, 5286) in vec3 gl_BaryCoordEXT;
spirv_decorate (extensions = ["SPV_KHR_fragment_shader_barycentric"], capabilities = [5284], 11, 5287) in vec3 gl_BaryCoordNoPerspEXT;

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
  //const vec4 pScreen = pScreen_ * pBary;

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

  //
  //gl_FragDepth = 1.f;

  // alpha and depth depth test fail
  const float dp = texelFetch(textures[framebuffers[0].attachments[0][5]], ivec2(gl_FragCoord.xy), 0).r;
  if (
#ifdef TRANSLUCENT
    materialPix.color[MATERIAL_ALBEDO].a < 0.01f || 
#endif
    false//dp <= (gl_FragCoord.z - 0.0001f)
  ) {
    discard;
  } else 
  {
    //
    indices = pIndices;
    baryData = vec4(pBary, pScreen.z/pScreen.w);
    position = vec4(pScreen.xyz/pScreen.w, 1.f);
    derivatives = uvec4(
      packHalf2x16(vec2(dFdx(pBary.x), dFdy(pBary.x))),
      packHalf2x16(vec2(dFdx(pBary.y), dFdy(pBary.y))),
      packHalf2x16(vec2(dFdx(pBary.z), dFdy(pBary.z))),
      packHalf2x16(vec2(dFdx(pScreen.z/pScreen.w), dFdy(pScreen.z/pScreen.w)))
    );
#ifdef TRANSLUCENT
    tcolor = materialPix.color[MATERIAL_ALBEDO] * vec4(materialPix.color[MATERIAL_ALBEDO].aaa, 1.f);
#else
    tcolor = vec4(0.f.xxx, 1.f);
#endif
    gl_FragDepth = gl_FragCoord.z;
  };

};
