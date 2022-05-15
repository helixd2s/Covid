#ifndef PUPET_DEF
#define PUPET_DEF

#include "./native.glsl"
#include "./raytracing.glsl"

float edgeFunction(in vec3 a, in vec3 b, in vec3 c) { return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x); }

//
vec3 computeBary(in vec4 vo, in mat3 M, in mat3x4 vt) {
  mat3x3 vt3 = M * mat3x3(vt[0].xyz-vo.xyz, vt[1].xyz-vo.xyz, vt[2].xyz-vo.xyz);
  vec3 UVW = vec3(
    vt3[2].x*vt3[1].y-vt3[2].y*vt3[1].x,
    vt3[0].x*vt3[2].y-vt3[0].y*vt3[2].x,
    vt3[1].x*vt3[0].y-vt3[1].y*vt3[0].x
  );
  float T = dot(UVW, transpose(vt3)[2].xyz);
  float det = UVW.x + UVW.y + UVW.z;
  return abs(det) > 0.f ? UVW/absmax(det, 1e-9) : vec3(0.f);
};

//
vec3 computeBary(in vec4 vo, in mat3x4 vt) {
  mat3x3 vt3 = mat3x3(vec3(vt[0].xy/absmax(vt[0].w, 1e-9), 1.f), vec3(vt[1].xy/absmax(vt[1].w, 1e-9), 1.f), vec3(vt[2].xy/absmax(vt[2].w, 1e-9), 1.f));
  float det = determinant(vt3);
  vec3 UVW = inverse(vt3)*vec3(vo.xy/absmax(vo.w, 1e-9),1.f);
  UVW /= absmax(transpose(vt)[3].xyz, 1e-9.xxx);
  UVW /= absmax(UVW.x+UVW.y+UVW.z, 1e-9);
  return abs(det) > 0.f ? UVW : vec3(0.f);
};

// too expensive method of rasterization
// vector sampling is generally expensive
// but it's really mostly required
  /*
IntersectionInfo rasterize(in InstanceAddressBlock addressInfo, in RayData rayData, in float maxT, inout vec4 lastPos, in bool previous) {

  IntersectionInfo intersection;
  intersection.barycentric = vec3(0.f.xxx);
  intersection.instanceId = 0u;
  intersection.geometryId = 0u;
  intersection.primitiveId = 0u;

  const mat3x4 lkAt = (previous ? constants.previousLookAt : constants.lookAt);

  //
  vec4 ssOriginal = divW(lastPos);
  vec4 viewOrigin = vec4(vec4(rayData.origin.xyz, 1.f) * lkAt, 1.f);
  vec4 viewEnd = vec4(vec4(rayData.origin.xyz+rayData.direction.xyz, 1.f) * lkAt, 1.f);
  vec4 viewDir = (viewEnd - viewOrigin);
  viewDir.xyz = normalize(viewDir.xyz);

  //
  vec4 ss = (viewOrigin * constants.perspective);
  ivec2 sc = ivec2((divW(ss).xy * 0.5f + 0.5f) * extent.xy);

  // TODO: separate translucency support
  uint indice = previous ? imageLoad(imagesR32UI[pingpong.prevImages[0]], sc).x : imageLoad(imagesR32UI[pingpong.images[0]], sc).x;

  //
  float currentZ = 1.f;
  lastPos = divW(ss);

  //
  for (uint d=0;d<32;d++) {
    if (indice <= 0u) break;

    //
    RasterInfoRef rasterInfo = previous ? getPrevRasterInfo(indice-1) : getRasterInfo(indice-1);
    InstanceInfo instanceInfo = getInstance(addressInfo, rasterInfo.indices.x);
    GeometryInfo geometryInfo = getGeometry(instanceInfo, rasterInfo.indices.y);
    GeometryExtData geometry = getGeometryData(geometryInfo, rasterInfo.indices.z);

    // no more used
    //mat3x3 M = mat3x3(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f), vec3(-viewDir.xy, 1.f)/viewDir.z);

    //
    for (uint i=0;i<3;i++) { 
      if (previous) {
        geometry.triData[VERTEX_VERTICES][i] = vec4(fullPreviousTransform(instanceInfo, vec4(geometry.triData[VERTEX_VERTICES][i].xyz, 1.f), rasterInfo.indices.y).xyz, 1.f);
      } else {
        geometry.triData[VERTEX_VERTICES][i] = vec4(fullTransform(instanceInfo, vec4(geometry.triData[VERTEX_VERTICES][i].xyz, 1.f), rasterInfo.indices.y).xyz, 1.f);
      };
      geometry.triData[VERTEX_VERTICES][i] = vec4(geometry.triData[VERTEX_VERTICES][i] * lkAt, 1.f) * constants.perspective;
      //geometry.triData[VERTEX_VERTICES][i] = vec4(geometry.triData[VERTEX_VERTICES][i] * (previous ? constants.previousLookAt : constants.lookAt), 1.f);
    };

    //
    vec3 bary = computeBary(ss, geometry.triData[VERTEX_VERTICES]);
    vec4 pos = divW(geometry.triData[VERTEX_VERTICES] * bary);

    //
    if (
      any(greaterThan(bary, 0.f.xxx)) && 
      all(greaterThan(bary, 1e-9.xxx)) && 
      all(lessThan(bary, 1.f.xxx+1e-9)) && 
      pos.z <= currentZ && ssOriginal.z < (pos.z + 0.0001f)
    ) {
      intersection.instanceId = rasterInfo.indices.x;
      intersection.geometryId = rasterInfo.indices.y;
      intersection.primitiveId = rasterInfo.indices.z;
      intersection.barycentric = bary;
      currentZ = pos.z;
      lastPos = pos;
    };

    //
    indice = rasterInfo.indices.w;
  };

  // prefer less-risky ways?
  //intersection = traceRaysTransparent(addressInfo, rayData, 10000.f, false);

  //
  return intersection;
};*/

// very cheap way - NOT RECOMMENDED!
IntersectionInfo rasterize_(in InstanceAddressBlock addressInfo, inout IntersectionInfo intersection, in RayData rayData, in float maxT, inout vec4 lastPos, in bool previous, in uint isTrasnlucent) {
  //
  const uvec4 indices = texelFetch(texturesU[framebufferAttachments[isTrasnlucent][0]], ivec2(rayData.launchId), 0);
  const vec3 bary = texelFetch(textures[framebufferAttachments[isTrasnlucent][1]], ivec2(rayData.launchId), 0).xyz;

  //
  vec4 viewOrigin = vec4(vec4(rayData.origin.xyz, 1.f) * (previous ? constants.previousLookAt : constants.lookAt), 1.f);
  vec4 viewEnd = vec4(vec4(rayData.origin.xyz+rayData.direction.xyz, 1.f) * (previous ? constants.previousLookAt : constants.lookAt), 1.f);
  vec4 viewDir = (viewEnd - viewOrigin);
  viewDir.xyz = normalize(viewDir.xyz);

  //
  vec4 ss = (viewOrigin * constants.perspective);
  vec2 sc = (divW(ss).xy * 0.5f + 0.5f);
  //vec4 sp = vec4(texture(sampler2D(textures[framebufferAttachments[isTrasnlucent][2]], samplers[0]), sc).xyz, 1.f);
  vec4 sp = vec4(texelFetch(textures[framebufferAttachments[isTrasnlucent][2]], ivec2(rayData.launchId), 0).xyz, 1.f);
  vec4 cp = vec4(texelFetch(textures[framebufferAttachments[isTrasnlucent][4]], ivec2(rayData.launchId), 0).xyz, 1.f);

  //
  if ((divW(lastPos).z <= (sp.z + 0.01f) || cp.w >= 1.f) && sp.z < 1.f) {
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
  vec4 ssPos = divW(vec4(vec4(rayData.origin.xyz, 1.f) * constants.lookAt, 1.f) * constants.perspective);
  ivec2 pxId = ivec2((ssPos.xy * 0.5f + 0.5f) * extent);

  //
  if (pxId.x >= 0 && pxId.y >= 0 && pxId.x < extent.x && pxId.y < extent.y) {
    vec4 ssSurf = ssPos; ssSurf.z = 1.f;

    // 
    { // I don't know, works it or not
      rasterize(instancedData, rayData, 10000.f, ssSurf, false);
    };

    // testing now working correctly, sorry
    if (all(lessThan(abs(ssPos.xyz-ssSurf.xyz), vec3(2.f/extent, 0.002f)))) {
      PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pxId.x + pxId.y * extent.x);
      const vec4 color = cvtRgb16Acc(surfaceInfo.accum[2]);
      rayData.emission += f16vec4(trueMultColor(color/color.w, rayData.energy));
    };
  };
  return rayData;
};

#endif
