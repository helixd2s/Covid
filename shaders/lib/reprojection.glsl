
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
  PixelSurfaceInfoRef SURF_SRC = getPixelSurface(pixelId);
  PixelHitInfoRef data = getRpjHit(pixelId, type);
  if (any(notEqual(data.origin.xyz, 0.f.xxx))) {

    // 
    const bool isSurface = (type == 2 || data.origin.w > 0.f) && data.origin.w < 10000.f;
    const bool isReflected = isSurface && data.origin.w < 9999.f;

    // 
    const vec3 srcPos = data.origin.xyz;
    const vec3 srcHitPos = srcPos + data.origin.w * data.direct.xyz;
    const vec3 srcNormal = data.normal.xyz;

    //
    const vec3 dstPos = isSurface ? vec4(vec4(srcPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, data.indices[0].x, 1)), 1.f) 
      * getInstanceTransform(instancedData, data.indices[0].x, 0) : srcPos;
    const vec3 dstHitPos = isReflected ? vec4(vec4(srcHitPos.xyz, 1.f) 
      * inverse(getInstanceTransform(instancedData, data.indices[1].x, 1)), 1.f) 
      * getInstanceTransform(instancedData, data.indices[1].x, 0) : srcHitPos;
    const vec3 dstNormal = isSurface ? normalize((srcNormal.xyz 
      * toNormalMat(inverse(getInstanceTransform(instancedData, data.indices[0].x, 1))))
      * toNormalMat(getInstanceTransform(instancedData, data.indices[0].x, 0))) : srcNormal;

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
      dstHitFoundIntersection = find_reflection_incident_point(
        vec4(0.f.xxx, 1.f) * constants.lookAtInverse[0],
        dstHitPos, dstPos.xyz, normalize(dstNormal.xyz));
    };

    // 
    const vec4 dstHitPersp = vec4(vec4(dstHitFoundIntersection, 1.f) * constants.lookAt[0], 1.f) * constants.perspective;
    const vec2 dstScreen = (dstHitPersp.xy/dstHitPersp.w * 0.5f + 0.5f);
    const ivec2 dstInt = ivec2(dstScreen * vec2(UR(deferredBuf.extent)));
    const uint dstId = uint(dstInt.x + dstInt.y * UR(deferredBuf.extent).x);
    const bool dstValid = dstInt.x >= 0 && dstInt.y >= 0 && dstInt.x < UR(deferredBuf.extent).x && dstInt.y < UR(deferredBuf.extent).y;

    // 
    TYPE original = SURF_SRC.accum[type];

    // 
    if (dstValid) {
      //
      PixelSurfaceInfoRef SURF_DST = getPixelSurface(dstId);
      PixelHitInfoRef HIT_DST = getNewHit(dstId, type);  

      // fallback
      vec4 dstSamplePos = dstHitPersp/dstHitPersp.w;
      bool dstValidNormal = false;

      //
      if (isSurface) { // 
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
          vec3 tbn[3]; //getTBN(attrib, tbn);
          tbn[0] = fullTransformNormal(instanceInfo, tbn[0], dstIntersection.geometryId, 0);
          tbn[1] = fullTransformNormal(instanceInfo, tbn[1], dstIntersection.geometryId, 0);
          tbn[2] = fullTransformNormal(instanceInfo, tbn[2], dstIntersection.geometryId, 0);

          //
          const MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), attrib.data[VERTEX_TEXCOORD].xy, mat3x3(tbn[0],tbn[1],tbn[2]));
          const bool inner = false;//dot(vec3(cmd.tbn[2]), cmd.rayData.direction.xyz) > 0.f;

          //
          dstValidNormal = abs(dot(normalize(tbn[2]), dstNormal)) > 0.9999f;
        };

      };

      // sorry, we doesn't save previous raster data
      const bool dstValidDist = all(lessThan(abs(dstSamplePos.xyz-(dstHitPersp.xyz/dstHitPersp.w)), vec3(2.f/vec2(UR(deferredBuf.extent)), 0.001f))) && (type == 2 ? (dstValidNormal || SURF_DST.color[type].w <= 0.f) : dstValidNormal);

      // copy to dest, and nullify source
      if ( original.w > 0.f && dstValidDist )
      {
        accumulate(SURF_DST, type, original); atomicOr(SURF_DST.flags[type], SURF_SRC.prevf[type]); //SURF_SRC.accum[type] = TYPE(0u);
        //for (uint i=0;i<3;i++) { SURF_DST.tex[i] = SURF_SRC.tex[i]; };

        HIT_DST.origin     = vec4(dstHitFoundIntersection.xyz, distance(dstHitPos.xyz, dstHitFoundIntersection.xyz));
        HIT_DST.indices    = data.indices;
        HIT_DST.direct.xyz = f16vec3(normalize(dstHitPos.xyz-dstHitFoundIntersection.xyz));
        HIT_DST.normal.xyz = f16vec3(dstNormal);
      };
    };
  };
};

#endif
