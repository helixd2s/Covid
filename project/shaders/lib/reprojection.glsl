
#ifndef REPROJECTION_DEF
#define REPROJECTION_DEF

//
#include "./rasterizer.glsl"
//
//#define OUTSOURCE
#define ACCOUNT_TRANSPARENCY

// incorrectly reprojected when distance more than 10000.f (i.e. skybox)
// needs re-creation skybox, or more distance (for example, 100000.f)
//void reproject3D(in PixelSurfaceInfo surface, in PixelHitInfo data, in uint pixelId, in vec3 srcRayDir, in int type) {
#ifdef OUTSOURCE
void reproject3D(in uint pixelId, in uint type) 
#else
void reproject3D(in uint pixelId, in uint type) 
#endif
{
  PixelSurfaceInfoRef surface = getPixelSurface(pixelId);
  PixelHitInfoRef data = getRpjHit(pixelId, type);

  //
  const bool isSurface = data.origin.w > 0.f && data.origin.w < 10000.f && any(greaterThan(abs(data.origin.xyz), 0.f.xxx));

    // 
#ifdef OUTSOURCE
    // 
    const vec3 srcPos = data.origin.xyz;
    const vec3 srcHitPos = srcPos + data.origin.w * data.direct.xyz;
    const vec3 srcNormal = surface.normal.xyz;

    //
    const vec3 dstPos = vec4(vec4(srcPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, surface.indices.x, 1)), 1.f) 
      * getInstanceTransform(instancedData, surface.indices.x, 0);
    const vec3 dstHitPos = vec4(vec4(srcHitPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, data.indices.x, 1)), 1.f) 
      * getInstanceTransform(instancedData, data.indices.x, 0);
    const vec3 dstNormal = normalize(srcNormal.xyz 
      * toNormalMat(getInstanceTransform(instancedData, surface.indices.x, 1)) 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x, 0))));
#else 
    // REQUIRED CURRENT RAY AND HIT SOURCE, NOT ANY PREVIOUS, NOT ANY HOLES!
    const vec3 dstPos = data.origin.xyz;
    const vec3 dstHitPos = dstPos + data.origin.w * data.direct.xyz;
    const vec3 dstNormal = surface.normal.xyz;

    // 
    const vec3 srcHitPos = vec4(vec4(dstHitPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, data.indices.x, 0)), 1.f) 
      * getInstanceTransform(instancedData, data.indices.x, 1);
    const vec3 srcPos = vec4(vec4(dstPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, surface.indices.x, 0)), 1.f) 
      * getInstanceTransform(instancedData, surface.indices.x, 1);
    const vec3 srcNormal = normalize(dstNormal.xyz 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x, 0))
      * toNormalMat(getInstanceTransform(instancedData, surface.indices.x, 1))));
#endif

    // DON'T TOUCH!
    vec3 srcHitFoundIntersection = srcPos;
    vec3 dstHitFoundIntersection = dstPos;

    // 
    if (type == 2) {
      // no changes for diffuse
    } else
#ifdef ACCOUNT_TRANSPARENCY
    if (type == 1) {
      srcHitFoundIntersection = srcHitPos;
      dstHitFoundIntersection = dstHitPos;
    } else
#endif
    //if (isSurface) 
    {
#ifdef OUTSOURCE
      // incorrect way or arguments
      // there is NOT any correct way to reproject from source to destination
      // there is only one way to reprojection
      // for correct way, needs have reflection sample itself
      // for me needs destionation point of reflection!
      dstHitFoundIntersection = vec4(find_reflection_incident_point( 
          vec4(dstPos.xyz, 1.f) * constants.lookAt[0],
          vec4(srcHitPos.xyz, 1.f) * constants.lookAt[1],
          vec4(srcPos.xyz, 1.f) * constants.lookAt[1],
          normalize(srcNormal.xyz) * toNormalMat(constants.lookAt[1])
        ), 1.f) * constants.lookAtInverse[0];

      /*dstHitFoundIntersection = vec4(find_reflection_incident_point( 
          vec4(srcPos.xyz, 1.f) * constants.lookAt[1],
          vec4(dstHitPos.xyz, 1.f) * constants.lookAt[0],
          vec4(dstPos.xyz, 1.f) * constants.lookAt[0],
          normalize(dstNormal.xyz) * toNormalMat(constants.lookAt[0])
        ), 1.f) * constants.lookAtInverse[0];*/
#else
      // REQUIRED current ray reflection and hit source! 
      // CHECKERBOARD ISN'T SUPPORTED, AS ANY HOLES! 
      srcHitFoundIntersection = vec4(find_reflection_incident_point( 
          vec4(dstPos.xyz, 1.f) * constants.lookAt[0],
          vec4(srcHitPos.xyz, 1.f) * constants.lookAt[1], 
          vec4(srcPos.xyz, 1.f) * constants.lookAt[1], 
          normalize(srcNormal.xyz) * toNormalMat(constants.lookAt[1])
        ), 1.f) * constants.lookAtInverse[1];
#endif
    };

    // 
    const vec4 srcHitPersp = vec4(vec4(srcHitFoundIntersection, 1.f) * constants.lookAt[1], 1.f) * constants.perspective;
    const vec2 srcScreen = (srcHitPersp.xy/srcHitPersp.w * 0.5f + 0.5f);
    const ivec2 srcInt = ivec2(srcScreen * vec2(extent));
    const uint srcId = uint(srcInt.x + srcInt.y * extent.x);
    const bool srcValid = srcInt.x >= 0 && srcInt.y >= 0 && srcInt.x < extent.x && srcInt.y < extent.y;

    // 
    const vec4 dstHitPersp = vec4(vec4(dstHitFoundIntersection, 1.f) * constants.lookAt[0], 1.f) * constants.perspective;
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
        rayData.direction = normalize(srcHitPos.xyz-srcHitFoundIntersection.xyz);
        rasterizeVector(instancedData, rayData, 10000.f, srcSamplePos, true);
      };

      //
      { // 
        RayData rayData;
        rayData.launchId = u16vec2(dstInt);
        rayData.origin = dstHitFoundIntersection.xyz;
        rayData.direction = normalize(dstHitPos.xyz-dstHitFoundIntersection.xyz);
        rasterizeVector(instancedData, rayData, 10000.f, dstSamplePos, false);
      };

      // sorry, we doesn't save previous raster data
      const bool dstValidDist = (isSurface ? all(lessThan(abs(dstSamplePos.xyz-(dstHitPersp.xyz/dstHitPersp.w)), vec3(2.f/extent, 0.008f))) : true);
      const bool srcValidDist = (isSurface ? all(lessThan(abs(srcSamplePos.xyz-(srcHitPersp.xyz/srcHitPersp.w)), vec3(2.f/extent, 0.008f))) : true) && HIT_SRC.origin.w > 0.f && any(greaterThan(abs(HIT_SRC.origin.xyz), 0.f.xxx));

      // copy to dest, and nullify source
      TYPE original = SURF_SRC.accum[type];
      if ( original.w > 0.f && dstValidDist && srcValidDist ) 
      {
        accumulate(SURF_DST, type, original);
        HIT_DST.origin = vec4(dstHitFoundIntersection, distance(dstHitPos, dstHitFoundIntersection));
        HIT_DST.indices = HIT_SRC.indices;
        HIT_DST.direct.xyz = normalize(dstHitPos.xyz-dstHitFoundIntersection.xyz);
      };
    };
  //};
};

//
#ifdef OUTSOURCE
void reprojectDiffuse(in uint pixelId, in uint type) 
#else
void reprojectDiffuse(in uint pixelId, in uint type) 
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
      * inverse(getInstanceTransform(instancedData, surface.indices.x, 1)), 1.f) 
      * getInstanceTransform(instancedData, surface.indices.x, 0);
    const vec3 dstNormal = normalize(srcNormal.xyz 
      * toNormalMat(getInstanceTransform(instancedData, surface.indices.x, 1)) 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x, 0))));

#else
    //
    const vec3 dstPos = surface.origin.xyz;
    const vec3 dstNormal = surface.normal.xyz;

    //
    const vec3 srcPos = vec4(vec4(dstPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, surface.indices.x, 0)), 1.f) 
      * getInstanceTransform(instancedData, surface.indices.x, 1);
    const vec3 srcNormal = normalize(dstNormal.xyz 
      * toNormalMat(inverse(getInstanceTransform(instancedData, surface.indices.x, 0))
      * toNormalMat(getInstanceTransform(instancedData, surface.indices.x, 1))));

#endif

    //
    const vec4 srcPerspPos = vec4(vec4(srcPos.xyz, 1.f) * constants.lookAt[1], 1.f) * constants.perspective;
    const vec2 srcScreen = (srcPerspPos.xy/srcPerspPos.w) * 0.5f + 0.5f;
    const ivec2 srcScreenPos = ivec2(srcScreen * vec2(extent));

    //
    const vec4 dstPerspPos = vec4(vec4(dstPos.xyz, 1.f) * constants.lookAt[0], 1.f) * constants.perspective;
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
      const bool dstValidDist = all(lessThan(abs(dstSamplePos.xyz-(dstPerspPos.xyz/dstPerspPos.w)), vec3(2.f/extent, 0.008f)));
      PixelSurfaceInfoRef dstSurface = getPixelSurface(dstId);

      // SRC
      const uint srcId = uint(srcScreenPos.x + srcScreenPos.y * extent.x);
      const bool srcValidDist = all(lessThan(abs(srcSamplePos.xyz-(srcPerspPos.xyz/srcPerspPos.w)), vec3(2.f/extent, 0.008f)));
      PixelSurfaceInfoRef srcSurface = getPixelSurface(srcId);

      // Up-Filling
      if (srcSurface.accum[type].w > 0.f && srcValidDist && dstValidDist) {
        accumulate(dstSurface, type, srcSurface.accum[type]);
      };
    };
  //};
};

#endif
