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
// currentlt, works BUGGY and bit-incorrectlly, and laggy
IntersectionInfo rasterizeVector(in InstanceAddressBlock addressInfo, in RayData rayData, in float maxT, inout vec4 lastPos, in bool previous) {
  IntersectionInfo intersection;
  intersection.barycentric = vec3(0.f.xxx);
  intersection.instanceId = 0u;
  intersection.geometryId = 0u;
  intersection.primitiveId = 0u;

  //
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
    mat3x4 vertices = readTriangleVertices3One(geometryInfo.vertices, readTriangleIndices(geometryInfo.indices, rasterInfo.indices.z));

    //
    for (uint i=0;i<3;i++) { 
      if (previous) {
        vertices[i] = vec4(fullPreviousTransform(instanceInfo, vec4(vertices[i].xyz, 1.f), rasterInfo.indices.y).xyz, 1.f);
      } else {
        vertices[i] = vec4(fullTransform(instanceInfo, vec4(vertices[i].xyz, 1.f), rasterInfo.indices.y).xyz, 1.f);
      };
      vertices[i] = vec4(vertices[i] * lkAt, 1.f) * constants.perspective;
      //vertices[i] = vec4(vertices[i] * (previous ? constants.previousLookAt : constants.lookAt), 1.f);
    };

    //
    vec3 bary = computeBary(ss, vertices);
    vec4 pos = divW(vertices * bary);

    //
    if (
      any(greaterThan(bary, 0.f.xxx)) && 
      all(greaterThan(bary, 1e-9.xxx)) && 
      all(lessThan(bary, 1.f.xxx+1e-9)) && 
      pos.z <= currentZ && (ssOriginal.z < (pos.z + 0.0001f) || (intersection.instanceId&0x80000000u) == 0u)
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
};

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
  if ((divW(lastPos).z <= (divW(sp).z + 0.001f) || cp.a >= 1.f) && sp.z < 1.f) {
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

//
//#define OUTSOURCE

//
//void reproject3D(in PixelSurfaceInfo surface, in PixelHitInfo data, in uint pixelId, in vec3 srcRayDir, in int type) {
#ifdef OUTSOURCE
void reproject3D(in uint pixelId, in vec3 srcRayDir, in int type) 
#else
void reproject3D(in uint pixelId, in vec3 dstRayDir, in int type) 
#endif
{
  PixelSurfaceInfoRef surface = getPixelSurface(pixelId);
#ifdef OUTSOURCE
  PixelHitInfoRef data = getRpjHit(pixelId, type);
#else
  PixelHitInfoRef data = getNewHit(pixelId, type);
#endif

  //
  //if (data.origin.x != 0.f || data.origin.y != 0.f || data.origin.z != 0.f) {
    // 

    // 
#ifdef OUTSOURCE
    // 
    const vec3 srcHitPos = data.origin.xyz + data.origin.w * srcRayDir;
    const vec3 srcPos = data.origin.xyz;
    const vec3 srcNormal = surface.normal.xyz;

    //
    const vec3 dstPos = vec4(vec4(srcPos.xyz, 1.f) 
      * inverse(getPreviousInstanceTransform(instancedData, surface.indices.x)), 1.f) 
      * getInstanceTransform(instancedData, surface.indices.x);
    const vec3 dstHitPos = vec4(vec4(srcHitPos.xyz, 1.f) 
      * inverse(getPreviousInstanceTransform(instancedData, data.indices.x)), 1.f) 
      * getInstanceTransform(instancedData, data.indices.x);
    const vec3 dstNormal = normalize(srcNormal.xyz 
      * toNormalMat(getPreviousInstanceTransform(instancedData, surface.indices.x)) 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x))));
    const vec3 dstRayDir = normalize(srcRayDir.xyz 
      * toNormalMat(getPreviousInstanceTransform(instancedData, surface.indices.x)) 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x))));
#else 
    //
    const vec3 dstHitPos = data.origin.xyz + data.origin.w * dstRayDir;
    const vec3 dstPos = data.origin.xyz;
    const vec3 dstNormal = surface.normal.xyz;

    // 
    const vec3 srcHitPos = vec4(vec4(dstHitPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, data.indices.x)), 1.f) 
      * getPreviousInstanceTransform(instancedData, data.indices.x);
    const vec3 srcPos = vec4(vec4(dstPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, surface.indices.x)), 1.f) 
      * getPreviousInstanceTransform(instancedData, surface.indices.x);
    const vec3 srcNormal = normalize(dstNormal.xyz 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x))
      * toNormalMat(getPreviousInstanceTransform(instancedData, surface.indices.x))));
    const vec3 srcRayDir = normalize(dstRayDir.xyz 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x))
      * toNormalMat(getPreviousInstanceTransform(instancedData, surface.indices.x))));
#endif

    // 
    vec3 srcHitFoundIntersection = srcPos;
    vec3 dstHitFoundIntersection = dstPos;
    if (type == 2) {
      // no changes for diffuse
    } else
#ifdef ACCOUNT_TRANSPARENCY
    if (type == 1) {
      dstHitFoundIntersection = dstHitPos;
      srcHitFoundIntersection = srcHitPos;
    } else
#endif
    { // if reflection
      dstHitFoundIntersection = vec4(find_reflection_incident_point( 
          vec4(dstPos.xyz, 1.f) * constants.lookAt, 
          vec4(srcHitPos.xyz, 1.f) * constants.previousLookAt, 
          vec4(srcPos.xyz, 1.f) * constants.previousLookAt, 
          normalize(srcNormal.xyz) * toNormalMat(constants.previousLookAt)
        ), 1.f) * constants.lookAtInverse;
      srcHitFoundIntersection = vec4(vec4(dstHitFoundIntersection, 1.f)
        * getInstanceTransform(instancedData, surface.indices.x), 1.f)
        * inverse(getPreviousInstanceTransform(instancedData, surface.indices.x));

      /*
      srcHitFoundIntersection = vec4(find_reflection_incident_point( 
          vec4(dstPos.xyz, 1.f) * constants.lookAt, 
          vec4(srcHitPos.xyz, 1.f) * constants.previousLookAt, 
          vec4(srcPos.xyz, 1.f) * constants.previousLookAt, 
          normalize(srcNormal.xyz) * toNormalMat(constants.previousLookAt)
        ), 1.f) * constants.previousLookAtInverse;
      dstHitFoundIntersection = vec4(vec4(srcHitFoundIntersection, 1.f)
        * inverse(getPreviousInstanceTransform(instancedData, surface.indices.x)), 1.f)
        * getInstanceTransform(instancedData, surface.indices.x);*/
    };

    // 
    const vec4 srcHitPersp = vec4(vec4(srcHitFoundIntersection, 1.f) * constants.previousLookAt, 1.f) * constants.perspective;
    const vec2 srcScreen = (srcHitPersp.xy/srcHitPersp.w * 0.5f + 0.5f);
    const ivec2 srcInt = ivec2(srcScreen * vec2(extent));
    const uint srcId = uint(srcInt.x + srcInt.y * extent.x);
    const bool srcValid = srcInt.x >= 0 && srcInt.y >= 0 && srcInt.x < extent.x && srcInt.y < extent.y;

    // 
    const vec4 dstHitPersp = vec4(vec4(dstHitFoundIntersection, 1.f) * constants.lookAt, 1.f) * constants.perspective;
    const vec2 dstScreen = (dstHitPersp.xy/dstHitPersp.w * 0.5f + 0.5f);
    const ivec2 dstInt = ivec2(dstScreen * vec2(extent));
    const uint dstId = uint(dstInt.x + dstInt.y * extent.x);
    const bool dstValid = dstInt.x >= 0 && dstInt.y >= 0 && dstInt.x < extent.x && dstInt.y < extent.y;

    // 
    if (srcValid && dstValid) {
      //
      PixelSurfaceInfoRef SURF_SRC = getPixelSurface(srcId);
      PixelHitInfoRef HIT_SRC = getRpjHit(srcId, type);

      //
      PixelSurfaceInfoRef SURF_DST = getPixelSurface(dstId);
      PixelHitInfoRef HIT_DST = getNewHit(dstId, type);  

      // fallback
      vec4 
        srcSamplePos = srcHitPersp/srcHitPersp.w,//vec4(linearSrcPos(srcScreen), 1.f), 
        dstSamplePos = dstHitPersp/dstHitPersp.w;//texture(sampler2D(textures[framebufferAttachments[2]], samplers[0u]), dstScreen);

      // 
      { // 
        RayData rayData;
        rayData.launchId = u16vec2(srcInt);
        rayData.origin = srcHitFoundIntersection.xyz;
        rayData.direction = normalize(srcHitFoundIntersection.xyz-srcPos.xyz);
        rasterizeVector(instancedData, rayData, 10000.f, srcSamplePos, true);
      };

      //
      { // 
        RayData rayData;
        rayData.launchId = u16vec2(dstInt);
        rayData.origin = dstHitFoundIntersection.xyz;
        rayData.direction = normalize(dstHitFoundIntersection.xyz-dstPos.xyz);
        rasterizeVector(instancedData, rayData, 10000.f, dstSamplePos, false);
      };

      // sorry, we doesn't save previous raster data
      const bool dstValidDist = all(lessThan(abs(dstSamplePos.xyz-(dstHitPersp.xyz/dstHitPersp.w)), vec3(1.f/extent, 0.001f)));
      const bool srcValidDist = all(lessThan(abs(srcSamplePos.xyz-(srcHitPersp.xyz/srcHitPersp.w)), vec3(1.f/extent, 0.001f)));

      // copy to dest, and nullify source
      if ( SURF_SRC.accum[type].w > 0.f && dstValidDist && (srcValidDist || SURF_DST.color[type].w <= 0.f) ) 
      {
        HIT_DST.origin = vec4(dstHitFoundIntersection, distance(dstHitPos, dstHitFoundIntersection));
        HIT_DST.indices = HIT_SRC.indices;
        accumulate(SURF_DST, type, SURF_SRC.accum[type]);
      };

    };
  //};
};

//
#ifdef OUTSOURCE
void reprojectDiffuse(in uint pixelId, in vec3 srcRayDir) 
#else
void reprojectDiffuse(in uint pixelId, in vec3 dstRayDir) 
#endif
{
  PixelSurfaceInfoRef surface = getPixelSurface(pixelId);
  //if (data.origin.x != 0.f || data.origin.y != 0.f || data.origin.z != 0.f) {

#ifdef OUTSOURCE
    //
    const vec3 srcPos = surface.origin.xyz;
    const vec3 srcNormal = surface.normal.xyz;

    //
    const vec3 dstPos = vec4(vec4(srcPos.xyz, 1.f) 
      * inverse(getPreviousInstanceTransform(instancedData, surface.indices.x)), 1.f) 
      * getInstanceTransform(instancedData, surface.indices.x);
    const vec3 dstNormal = normalize(srcNormal.xyz 
      * toNormalMat(getPreviousInstanceTransform(instancedData, surface.indices.x)) 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x))));
    const vec3 dstRayDir = normalize(srcRayDir.xyz 
      * toNormalMat(getPreviousInstanceTransform(instancedData, surface.indices.x)) 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x))));

#else
    //
    const vec3 dstPos = surface.origin.xyz;
    const vec3 dstNormal = surface.normal.xyz;

    //
    const vec3 srcPos = vec4(vec4(dstPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, surface.indices.x)), 1.f) 
      * getPreviousInstanceTransform(instancedData, surface.indices.x);
    const vec3 srcNormal = normalize(dstNormal.xyz 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x))
      * toNormalMat(getPreviousInstanceTransform(instancedData, surface.indices.x))));
    const vec3 srcRayDir = normalize(dstRayDir.xyz 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x))
      * toNormalMat(getPreviousInstanceTransform(instancedData, surface.indices.x))));
#endif

    //
    const vec4 srcPerspPos = vec4(vec4(srcPos.xyz, 1.f) * constants.previousLookAt, 1.f) * constants.perspective;
    const vec2 srcScreen = (srcPerspPos.xy/srcPerspPos.w) * 0.5f + 0.5f;
    const ivec2 srcScreenPos = ivec2(srcScreen * vec2(extent));

    //
    const vec4 dstPerspPos = vec4(vec4(dstPos.xyz, 1.f) * constants.lookAt, 1.f) * constants.perspective;
    const vec2 dstScreen = (dstPerspPos.xy/dstPerspPos.w) * 0.5f + 0.5f;
    const ivec2 dstScreenPos = ivec2(dstScreen * vec2(extent));

    //
    vec4 
      srcSamplePos = srcPerspPos/srcPerspPos.w,//vec4(linearSrcPos(srcScreen), 1.f), 
      dstSamplePos = dstPerspPos/dstPerspPos.w;//texture(sampler2D(textures[framebufferAttachments[2]], samplers[0u]), dstScreen);

    // 
    { //
      RayData rayData;
      rayData.launchId = u16vec2(srcScreenPos);
      rayData.origin = srcPos.xyz;
      rayData.direction = vec3(0.f);
      rasterizeVector(instancedData, rayData, 10000.f, srcSamplePos, true);
    };

    { //
      RayData rayData;
      rayData.launchId = u16vec2(dstScreenPos);
      rayData.origin = dstPos.xyz;
      rayData.direction = vec3(0.f);
      rasterizeVector(instancedData, rayData, 10000.f, dstSamplePos, false);
    };

    //
    if (
      dstScreenPos.x >= 0 && dstScreenPos.y >= 0 && dstScreenPos.x < extent.x && dstScreenPos.y < extent.y && 
      srcScreenPos.x >= 0 && srcScreenPos.y >= 0 && srcScreenPos.x < extent.x && srcScreenPos.y < extent.y
    ) {
      // DST
      const uint dstId = uint(dstScreenPos.x + dstScreenPos.y * extent.x);
      const bool dstValidDist = all(lessThan(abs(dstSamplePos.xyz-(dstPerspPos.xyz/dstPerspPos.w)), vec3(1.f/extent, 0.001f)));
      PixelSurfaceInfoRef dstSurface = getPixelSurface(dstId);

      // SRC
      const uint srcId = uint(srcScreenPos.x + srcScreenPos.y * extent.x);
      const bool srcValidDist = all(lessThan(abs(srcSamplePos.xyz-(srcPerspPos.xyz/srcPerspPos.w)), vec3(1.f/extent, 0.001f)));
      PixelSurfaceInfoRef srcSurface = getPixelSurface(srcId);

      // Up-Filling
      if (srcSurface.accum[2].w > 0.f && srcValidDist && dstValidDist) {
        accumulate(dstSurface, 2, srcSurface.accum[2]);
      };
    };
  //};
};



#endif
