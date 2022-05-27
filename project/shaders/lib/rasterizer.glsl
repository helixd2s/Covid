#ifndef RASTERIZER_DEF
#define RASTERIZER_DEF

#include "./native.glsl"
#include "./raytracing.glsl"

// too expensive method of rasterization
// vector sampling is generally expensive
// but it's really mostly required
// currentlt, works BUGGY and bit-incorrectlly, and laggy
IntersectionInfo rasterizeVector(in InstanceAddressBlock addressInfo, in RayData rayData, in float maxT, inout vec4 lastPos, in bool previous) {
  IntersectionInfo intersection;
  intersection.barycentric = vec3(0.f.xxx);
  intersection.instanceId = 0u;
  intersection.geometryId = 0u;
  intersection.primitiveId = 0u;

  //
  const mat3x4 lkAt = constants.lookAt[previous?1:0];

  //
  vec4 ssOriginal = divW(lastPos);
  vec4 viewOrigin = vec4(vec4(rayData.origin.xyz, 1.f) * lkAt, 1.f);
  vec4 viewEnd = vec4(vec4(rayData.origin.xyz+rayData.direction.xyz, 1.f) * lkAt, 1.f);
  vec4 viewDir = (viewEnd - viewOrigin);
  viewDir.xyz = normalize(viewDir.xyz);

  //
  vec4 ss = (viewOrigin * constants.perspective);
  vec2 ssc = (divW(ss).xy * 0.5f + 0.5f) * UR(framebuffers[0].extent).xy;
  vec2 ssh = floor(ssc.xy) + 0.5f;
  vec2 ssf = ssc - ssh;
  ivec2 sc = ivec2(ssc);

  // TODO: separate translucency support
  uint indice = imageLoad(imagesR32UI[rasterBuf.images[previous?1:0][0]], sc).x;

  //
  float currentZ = 1.f;
  lastPos = divW(ss);

  //
  for (uint d=0;d<32;d++) {
    if (indice <= 0u) break;

    // compute derrivative
    RasterInfoRef rasterInfo = getRasterInfo(indice-1, previous?1:0);
    const vec3 bary = vec3(
      rasterInfo.barycentric.x + dot(unpackHalf2x16(rasterInfo.derivatives.x),ssf), 
      rasterInfo.barycentric.y + dot(unpackHalf2x16(rasterInfo.derivatives.y),ssf), 
      rasterInfo.barycentric.z + dot(unpackHalf2x16(rasterInfo.derivatives.z),ssf)
    );
    const vec4 pos = vec4(divW(ss).xy, rasterInfo.barycentric.w + dot(unpackHalf2x16(rasterInfo.derivatives.w),ssf), 1.f);

    //
    if (
      any(greaterThan(bary, 0.f.xxx)) && 
      all(greaterThan(bary, 1e-9.xxx)) && 
      all(lessThan(bary, 1.f.xxx+1e-9)) && 
      pos.z <= currentZ && (ssOriginal.z < (pos.z + 0.001f) || (intersection.instanceId&0x80000000u) == 0u)
    ) {
      intersection.instanceId = rasterInfo.indices.x;
      intersection.geometryId = rasterInfo.indices.y;
      intersection.primitiveId = rasterInfo.indices.z;
      intersection.barycentric = bary;
      currentZ = pos.z;
      lastPos = pos;
      break; // due sorted!
    };

    //
    indice = rasterInfo.indices.w;
  };

  // prefer less-risky ways?
  //intersection = traceRaysTransparent(addressInfo, rayData, 10000.f, false);

  //
  return intersection;
};

// very cheap way - NOT RECOMMENDED!
IntersectionInfo rasterize_(in InstanceAddressBlock addressInfo, inout IntersectionInfo intersection, in RayData rayData, in float maxT, inout vec4 lastPos, in bool previous, in uint isTrasnlucent) {
  //
  vec4 viewOrigin = vec4(vec4(rayData.origin.xyz, 1.f) * constants.lookAt[previous?1:0], 1.f);
  vec4 viewEnd = vec4(vec4(rayData.origin.xyz+rayData.direction.xyz, 1.f) * constants.lookAt[previous?1:0], 1.f);
  vec4 viewDir = (viewEnd - viewOrigin);
  viewDir.xyz = normalize(viewDir.xyz);

  //
  vec4 ss = (viewOrigin * constants.perspective);
  vec2 ssc = (divW(ss).xy * 0.5f + 0.5f) * UR(framebuffers[0].extent).xy;
  vec2 ssh = floor(ssc.xy) + 0.5f;
  vec2 ssf = ssc - ssh;
  ivec2 sc = ivec2(ssc);

  //
  const uvec4 indices = texelFetch(texturesU[framebuffers[isTrasnlucent].attachments[uint(previous)][0]], sc, 0);
  const uvec4 dr      = texelFetch(texturesU[framebuffers[isTrasnlucent].attachments[uint(previous)][1]], sc, 0);
  const vec3 br =       texelFetch(textures [framebuffers[isTrasnlucent].attachments[uint(previous)][2]], sc, 0).xyz;
  const vec4 sp =  vec4(texelFetch(textures [framebuffers[isTrasnlucent].attachments[uint(previous)][3]], sc, 0).xyz, 1.f);
  const vec4 cp =       texelFetch(textures [framebuffers[isTrasnlucent].attachments[uint(previous)][4]], sc, 0);

  // compute derrivative
  const vec3 bary = vec3(
    br.x + dot(unpackHalf2x16(dr.x),ssf), 
    br.y + dot(unpackHalf2x16(dr.y),ssf), 
    br.z + dot(unpackHalf2x16(dr.z),ssf)
  );
  const vec4 pos = vec4(divW(ss).xy, sp.z + dot(unpackHalf2x16(dr.w),ssf), 1.f);

  //
  if ((divW(lastPos).z <= (divW(pos).z + 0.004f) || cp.a >= 1.f) && pos.z < 1.f) {
    intersection.barycentric = bary.xyz;
    intersection.instanceId = indices[0];
    intersection.geometryId = indices[1];
    intersection.primitiveId = indices[2];
    lastPos = sp;
  };

  //
  return intersection;
};

//
IntersectionInfo rasterize(in InstanceAddressBlock addressInfo, in RayData rayData, in float maxT, inout vec4 lastPos, in bool previous) {
  IntersectionInfo intersection;
  intersection.barycentric = vec3(0.f.xxx);
  intersection.instanceId = 0u;
  intersection.geometryId = 0u;
  intersection.primitiveId = 0u;

  //
  rasterize_(addressInfo, intersection, rayData, maxT, lastPos, previous, 0);
  rasterize_(addressInfo, intersection, rayData, maxT, lastPos, previous, 1);

  //
  return intersection;
};

//
RayData reuseLight(inout RayData rayData) {
  // screen space reuse already lighted pixels
  vec4 ssPos = divW(vec4(vec4(rayData.origin.xyz, 1.f) * constants.lookAt[0], 1.f) * constants.perspective);
  ivec2 pxId = ivec2((ssPos.xy * 0.5f + 0.5f) * UR(rasterBuf.extent));

  //
  if (pxId.x >= 0 && pxId.y >= 0 && pxId.x < UR(rasterBuf.extent).x && pxId.y < UR(rasterBuf.extent).y) {
    vec4 ssSurf = ssPos; ssSurf.z = 1.f;

    // 
    { // I don't know, works it or not
      rasterize(instancedData, rayData, 10000.f, ssSurf, false);
    };

    // testing now working correctly, sorry
    if (all(lessThan(abs(ssPos.xyz-ssSurf.xyz), vec3(2.f/UR(rasterBuf.extent), 0.002f)))) {
      PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pxId.x + pxId.y * UR(rasterBuf.extent).x);
      const vec4 color = cvtRgb16Acc(surfaceInfo.accum[2]);
      rayData.emission += f16vec4(trueMultColor(color/color.w, rayData.energy));
    };
  };
  return rayData;
};



#endif
