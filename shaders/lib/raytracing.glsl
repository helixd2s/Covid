#ifndef RAYTRACE_DEF
#define RAYTRACE_DEF

// 
#include "./native.glsl"
#include "./fresnel.glsl"
#include "./random.glsl"

//
const vec4 sunSphere = vec4(1000.f, 5000.f, 1000.f, 200.f);
const vec3 sunColor = vec3(0.95f, 0.9f, 0.8f) * 10000.f;
const vec4 skyColor = vec4(vec3(135.f,206.f,235.f)/vec3(255.f,255.f,255.f), 1.f);

// 
struct PassData {
  vec4 alphaColor;
  bool alphaPassed;
  bool diffusePass;
  vec3 normals;
  bool validRay;
  //vec3 origin;
};

// 
struct RayData
{
    vec3 origin; u16vec2 launchId;
    vec3 direction; u16vec2 reserved;
    f16vec4 energy; f16vec4 emission;
};

// 
struct IntersectionInfo 
{
    vec3 barycentric; float hitT;
    uint instanceId, geometryId, primitiveId;
};

//
struct PathTraceCommand {
  RayData rayData;
  PixelSurfaceInfoRef surface;
  IntersectionInfo intersection;
  f16vec4 diffuseColor;
  f16vec3 emissiveColor;
  f16vec3 normals;
  f16vec3 PBR;
  f16mat3x3 tbn;
  float reflCoef;
};

//
layout(scalar) shared PathTraceCommand cmds[4][32];

//
struct PathTraceOutput {
  vec3 normal; float hitT;
  uvec4 indices; bool surfaceFound;
};

// 
IntersectionInfo traceRaysOpaque(in InstanceAddressBlock instance, inout IntersectionInfo result, in RayData rays, in float maxT) {
  // 
  float currentT = 10000.f;

  //
  if (instance.addressInfos[0].accelStruct > 0) {//
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, accelerationStructureEXT(instance.addressInfos[0].accelStruct), gl_RayFlagsOpaqueEXT, 0xff, rays.origin.xyz, 0.001f, rays.direction.xyz, maxT);

    //
    while(rayQueryProceedEXT(rayQuery)) {
      const float fT = rayQueryGetIntersectionTEXT(rayQuery, false);
      if (fT <= currentT) {
        currentT = fT;
        rayQueryConfirmIntersectionEXT(rayQuery);
      };
    };

    // 
    if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
      vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
      result.barycentric = vec3(1.f - attribs.x - attribs.y, attribs);
      result.hitT = rayQueryGetIntersectionTEXT(rayQuery, true);
      result.instanceId = (rayQueryGetIntersectionInstanceIdEXT(rayQuery, true) & 0x7FFFFFFF);
      result.geometryId = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, true);
      result.primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true);
    };
  };

  // 
  return result;
};

// version without over-phasing
IntersectionInfo traceRaysTransparent(in InstanceAddressBlock instance, inout IntersectionInfo result, in RayData rays, in float maxT, in bool hasRandom) {
  //
  float currentT = 10000.f;

  //
  if (instance.addressInfos[1].accelStruct > 0) {
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, accelerationStructureEXT(instance.addressInfos[1].accelStruct), gl_RayFlagsNoOpaqueEXT, 0xff, rays.origin.xyz, 0.001f, rays.direction.xyz, maxT);

    // 
    while(rayQueryProceedEXT(rayQuery)) {
      bool isOpaque = true;

      { // compute intersection opacity
        const float fT = rayQueryGetIntersectionTEXT(rayQuery, false);
        if (fT <= currentT) {
          const uint instanceId = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
          const uint geometryId = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, false);
          const uint primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false);
          InstanceInfo instanceInfo = getInstance(instance, 1, instanceId);
          GeometryInfo geometryInfo = getGeometry(instanceInfo, geometryId);
          const uvec3 indices = readTriangleIndices(geometryInfo.indices, primitiveId);
          const vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
          GeometryExtData geometry = getGeometryData(geometryInfo, primitiveId);
          GeometryExtAttrib interpol = interpolate(geometry, attribs);
          mat3x3 tbn = getTBN(interpol);
          tbn[0] = fullTransformNormal(instanceInfo, tbn[0], geometryId, 0);
          tbn[1] = fullTransformNormal(instanceInfo, tbn[1], geometryId, 0);
          tbn[2] = fullTransformNormal(instanceInfo, tbn[2], geometryId, 0);
          MaterialPixelInfo material = handleMaterial(getMaterialInfo(geometryInfo), interpol.data[VERTEX_TEXCOORD].xy, tbn);

          if (material.color[MATERIAL_ALBEDO].a < (hasRandom ? random(blueNoiseFn(rays.launchId.xy)) : 0.01f)) {
            isOpaque = false;
          } else {
            currentT = fT;
          };
        };
      };

      if (isOpaque) {
        rayQueryConfirmIntersectionEXT(rayQuery);
      };
    };

    // 
    if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
      vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
      result.barycentric = vec3(1.f - attribs.x - attribs.y, attribs);
      result.hitT = rayQueryGetIntersectionTEXT(rayQuery, true);
      result.instanceId = 0x80000000 | (rayQueryGetIntersectionInstanceIdEXT(rayQuery, true) & 0x7FFFFFFF);
      result.geometryId = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, true);
      result.primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true);
    };
  };

  // 
  return result;
};

//
vec4 directLighting(in vec3 O, in vec3 N, in vec3 tN, in vec3 r, in float t) {
  const vec3 SO = sunSphere.xyz + (vec4(0.f.xxx, 1.f) * constants.lookAtInverse[0]);
  const vec3 LC = SO - O;
  const float dt = dot(LC, LC);
  const float cosL = sqrt(1.f - clamp((sunSphere.w * sunSphere.w) / dt, 0.f, 1.f));
  const float weight = 2.f * (1.f - cosL);

  //
  RayData rayData;
  rayData.direction.xyz = coneSample(LC * inversesqrt(dt), cosL, r.xy);
  rayData.origin.xyz = O + outRayNormal(rayData.direction.xyz, tN) * 0.001f;
  rayData.energy.xyzw = f16vec4(1.f.xxx, 0.f);
  rayData.emission.xyzw = f16vec4(0.f.xxx, 0.f);

  // 
  const bool hasIntersection = intersect(vec4(SO, sunSphere.w), rayData.origin.xyz, rayData.direction.xyz, t) && dot( rayData.direction.xyz, tN ) > 0.f;
  IntersectionInfo intersection;
  {
    intersection.barycentric = vec3(0.f.xxx);
    intersection.hitT = t;
    intersection.instanceId = 0u;
    intersection.geometryId = 0u;
    intersection.primitiveId = 0u;
  };

  //
  const float BRDF = hasIntersection ? (weight * clamp(dot( rayData.direction.xyz, N ), 0.f, 1.f)) : 0.f;

  //
  if (BRDF > 0.f && hasIntersection) {
    intersection = traceRaysOpaque(instancedData, intersection, rayData, t);
    intersection = traceRaysTransparent(instancedData, intersection, rayData, intersection.hitT, true);
  };

  //
  if (hasIntersection && (intersection.hitT + 0.0001f) >= t && t >= 0.f) {
    rayData.emission.xyz += f16vec3(sunColor * BRDF);
  };

  //
  return vec4(rayData.emission.xyz, 0.f);
};

#endif
