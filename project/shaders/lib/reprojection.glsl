
#ifndef REPROJECTION_DEF
#define REPROJECTION_DEF

//
#include "./rasterizer.glsl"
//
//#define ACCOUNT_TRANSPARENCY

// incorrectly reprojected when distance more than 10000.f (i.e. skybox)
// needs re-creation skybox, or more distance (for example, 100000.f)
// also, note about there is not enough data for check reflection correctness
//void reproject3D(in PixelSurfaceInfo surface, in PixelHitInfo data, in uint pixelId, in vec3 srcRayDir, in int type) {
void reproject3D(in uint pixelId, in uint type) 
{
  PixelHitInfoRef data = getRpjHit(pixelId, type);
  if (any(notEqual(data.origin.xyz, 0.f.xxx))) {
    //
    const bool isSurface = data.origin.w > 0.f && data.origin.w < 10000.f && any(greaterThan(abs(data.origin.xyz), 0.f.xxx));

    // 
    const vec3 srcPos = data.origin.xyz;
    const vec3 srcHitPos = srcPos + data.origin.w * data.direct.xyz;
    const vec3 srcNormal = data.normal.xyz;

    //
    const vec3 dstPos = vec4(vec4(srcPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, data.indices[0].x, 1)), 1.f) 
      * getInstanceTransform(instancedData, data.indices[0].x, 0);
    const vec3 dstHitPos = vec4(vec4(srcHitPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, data.indices[1].x, 1)), 1.f) 
      * getInstanceTransform(instancedData, data.indices[1].x, 0);
    const vec3 dstNormal = normalize((srcNormal.xyz 
      * toNormalMat(inverse(getInstanceTransform(instancedData, data.indices[0].x, 1))))
      * toNormalMat(getInstanceTransform(instancedData, data.indices[0].x, 0)));

    // DON'T TOUCH!
    vec3 srcHitFoundIntersection = srcPos;
    vec3 dstHitFoundIntersection = dstPos;

    // 
    if (type == 2) {
      // no changes for diffuse
    } else
    if (type == 0 || type == 1) {
      // still have problems in curved surfaces...
      // incorrect curve when moving camera...
      // also, note about there is not enough data for check reflection correctness
      dstHitFoundIntersection = vec4(find_reflection_incident_point(
          vec4(0.f.xxx, 1.f).xyz,
          vec4(dstHitPos.xyz, 1.f) * constants.lookAt[0],
          vec4(dstPos.xyz, 1.f) * constants.lookAt[0],
          normalize(dstNormal.xyz) * toNormalMat(constants.lookAt[0])
        ), 1.f) * constants.lookAtInverse[0];
    };

    // 
    const vec4 srcHitPersp = vec4(vec4(srcHitFoundIntersection, 1.f) * constants.lookAt[1], 1.f) * constants.perspective;
    const vec2 srcScreen = (srcHitPersp.xy/srcHitPersp.w * 0.5f + 0.5f);
    const ivec2 srcInt = ivec2(srcScreen * vec2(UR(deferredBuf.extent)));
    const uint srcId = uint(srcInt.x + srcInt.y * UR(deferredBuf.extent).x);
    const bool srcValid = srcInt.x >= 0 && srcInt.y >= 0 && srcInt.x < UR(deferredBuf.extent).x && srcInt.y < UR(deferredBuf.extent).y;

    // 
    const vec4 dstHitPersp = vec4(vec4(dstHitFoundIntersection, 1.f) * constants.lookAt[0], 1.f) * constants.perspective;
    const vec2 dstScreen = (dstHitPersp.xy/dstHitPersp.w * 0.5f + 0.5f);
    const ivec2 dstInt = ivec2(dstScreen * vec2(UR(deferredBuf.extent)));
    const uint dstId = uint(dstInt.x + dstInt.y * UR(deferredBuf.extent).x);
    const bool dstValid = dstInt.x >= 0 && dstInt.y >= 0 && dstInt.x < UR(deferredBuf.extent).x && dstInt.y < UR(deferredBuf.extent).y;

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
        srcSamplePos = srcHitPersp/srcHitPersp.w,
        dstSamplePos = dstHitPersp/dstHitPersp.w;

      // 
      { // 
        RayData rayData;
        rayData.launchId = u16vec2(srcInt);
        rayData.origin = srcHitFoundIntersection.xyz;
        rayData.direction = normalize(srcHitPos.xyz-srcHitFoundIntersection.xyz);
        IntersectionInfo srcIntersection = rasterizeVector(instancedData, rayData, 10000.f, srcSamplePos, true);
      };

      //
      bool dstValidNormal = false;
      { // 
        RayData rayData;
        rayData.launchId = u16vec2(dstInt);
        rayData.origin = dstHitFoundIntersection.xyz;
        rayData.direction = normalize(dstHitPos.xyz-dstHitFoundIntersection.xyz);

        //
        IntersectionInfo dstIntersection = rasterizeVector(instancedData, rayData, 10000.f, dstSamplePos, false);
        const bool hasHit = !all(lessThanEqual(dstIntersection.barycentric, 0.f.xxx));

        //
        if (hasHit) {
          InstanceInfo instanceInfo = getInstance(instancedData, dstIntersection.instanceId);
          GeometryInfo geometryInfo = getGeometry(instanceInfo, dstIntersection.geometryId);
          GeometryExtData geometry = getGeometryData(geometryInfo, dstIntersection.primitiveId);
          GeometryExtAttrib attrib = interpolate(geometry, dstIntersection.barycentric);

          //
          mat3x3 tbn = f16mat3x3(getTBN(attrib)); //cmd.rayData.origin += outRayNormal(cmd.rayData.direction.xyz, cmd.tbn[2].xyz) * 0.0001f;
          tbn[0] = f16vec3(fullTransformNormal(instanceInfo, tbn[0], dstIntersection.geometryId, 0));
          tbn[1] = f16vec3(fullTransformNormal(instanceInfo, tbn[1], dstIntersection.geometryId, 0));
          tbn[2] = f16vec3(fullTransformNormal(instanceInfo, tbn[2], dstIntersection.geometryId, 0));

          //
          const MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), attrib.data[VERTEX_TEXCOORD].xy, tbn);
          const bool inner = false;//dot(vec3(cmd.tbn[2]), cmd.rayData.direction.xyz) > 0.f;

          //
          dstValidNormal = abs(dot(normalize(tbn[2]), dstNormal)) > 0.9999f;
        };

      };

      // sorry, we doesn't save previous raster data
      const bool dstValidDist = all(lessThan(abs(dstSamplePos.xyz-(dstHitPersp.xyz/dstHitPersp.w)), vec3(1.f/vec2(UR(deferredBuf.extent)), 0.008f))) && dstValidNormal;
      const bool srcValidDist = all(lessThan(abs(srcSamplePos.xyz-(srcHitPersp.xyz/srcHitPersp.w)), vec3(1.f/vec2(UR(deferredBuf.extent)), 0.008f))) && any(greaterThan(abs(HIT_SRC.origin.xyz), 0.f.xxx)) && (HIT_SRC.origin.w > 0.f || type == 2);

      // copy to dest, and nullify source
      TYPE original = SURF_SRC.accum[type];
      if ( dstValidDist && srcValidDist ) 
      {
        accumulate(SURF_DST, type, original);
        HIT_DST.origin = vec4(dstHitFoundIntersection.xyz, distance(dstHitPos.xyz, dstHitFoundIntersection.xyz));
        HIT_DST.indices = HIT_SRC.indices;
        HIT_DST.direct.xyz = f16vec3(normalize(dstHitPos.xyz-dstHitFoundIntersection.xyz));
        HIT_DST.normal.xyz = f16vec3(dstNormal);
      };
    };
  };
};

#endif
