#ifndef RAYTRACE_DEF
#define RAYTRACE_DEF

// 
#include "./native.glsl"
#include "./fresnel.glsl"
#include "./random.glsl"
#include "./sphere.glsl"

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
vec3 reflective(in vec3 seed, in vec3 dir, in vec3 normal, in float roughness) {
  return normalize(mix(reflect(dir, normal), randomCosineWeightedHemispherePoint(seed, normal), roughness * random(seed)));
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
  rayData.origin.xyz = O + outRayNormal(rayData.direction.xyz, tN) * 0.0001f;
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
  if (hasIntersection && intersection.hitT >= t && t > 0.f) {
    rayData.emission.xyz += f16vec3(sunColor * BRDF);
  };

  //
  return vec4(rayData.emission.xyz, 0.f);
};

//
RayData reuseLight(inout RayData rayData);

// 
RayData handleIntersection(inout RayData rayData, inout IntersectionInfo intersection, inout PassData passed, inout uint type) {
  InstanceInfo instanceInfo = getInstance(instancedData, intersection.instanceId);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, intersection.geometryId);
  GeometryExtData geometry = getGeometryData(geometryInfo, intersection.primitiveId);
  GeometryExtAttrib attrib = interpolate(geometry, intersection.barycentric);

  //
  const vec4 texcoord = attrib.data[VERTEX_TEXCOORD];
  const vec4 vertice = fullTransform(instanceInfo, attrib.data[VERTEX_VERTICES], intersection.geometryId, 0);

  //
  mat3x3 tbn = getTBN(attrib);
  tbn[0] = fullTransformNormal(instanceInfo, tbn[0], intersection.geometryId, 0);
  tbn[1] = fullTransformNormal(instanceInfo, tbn[1], intersection.geometryId, 0);
  tbn[2] = fullTransformNormal(instanceInfo, tbn[2], intersection.geometryId, 0);

  //
  const bool inner = false;//dot(tbn[2], rayData.direction.xyz) > 0.f;
  MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), texcoord.xy, tbn);
  const vec3 normals = inRayNormal(rayData.direction, materialPix.color[MATERIAL_NORMAL].xyz);//materialPix.color[MATERIAL_NORMAL].xyz;

  // 
  vec4 emissiveColor = toLinear(materialPix.color[MATERIAL_EMISSIVE]) * vec4(4.f.xxx, 1.f);
  vec4 diffuseColor = toLinear(materialPix.color[MATERIAL_ALBEDO]);
  float metallicFactor = materialPix.color[MATERIAL_PBR].b;
  float roughnessFactor = materialPix.color[MATERIAL_PBR].g;

  //
  float transpCoef = clamp(1.f - diffuseColor.a, 0.f, 1.f);
  float reflFactor = clamp((metallicFactor + mix(fresnel_schlick(0.f, dot(reflect(rayData.direction.xyz, normals), normals)), 0.f, roughnessFactor) * (1.f - metallicFactor)) * (1.f - luminance(emissiveColor.xyz)), 0.f, 1.f);
  vec3 originSeedXYZ = vec3(random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy)));

  //
  passed.alphaColor = vec4(mix(diffuseColor.xyz, 1.f.xxx, diffuseColor.a), diffuseColor.a);
  passed.normals = normals;
  //passed.origin = vertice.xyz;

  //
  rayData.origin.xyz += rayData.direction.xyz * intersection.hitT;//vertice.xyz;

  //
  if (random(blueNoiseFn(rayData.launchId.xy)) <= clamp(reflFactor, 0.f, 1.f) && transpCoef < 1.f) { // I currently, have no time for fresnel
    rayData.direction.xyz = reflective(originSeedXYZ, rayData.direction.xyz, normals, roughnessFactor);
    rayData.energy.xyz = f16vec3(metallicMult(rayData.energy.xyz, diffuseColor.xyz, metallicFactor));
    if (reflFactor < 0.1f || type == 1) { passed.diffusePass = true; };
    //passed.diffusePass = true;
  } else 
  if (random(blueNoiseFn(rayData.launchId.xy)) <= clamp(transpCoef, inner ? 1.f : 0.f, 1.f)) { // wrong diffuse if inner
    rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, passed.alphaColor.xyz));
    passed.alphaPassed = true;
  } else
  {
    if (luminance(emissiveColor.xyz) > 0.001f) {
      rayData.emission.xyz += f16vec3(trueMultColor(rayData.energy.xyz, emissiveColor.xyz));
      rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, max(1.f-emissiveColor.xyz, 0.f.xxx)));
    };
    passed.diffusePass = true;
    rayData.direction.xyz = randomCosineWeightedHemispherePoint(originSeedXYZ, normals);
    rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, diffuseColor.xyz));
    rayData.emission.xyz += f16vec3(trueMultColor(rayData.energy.xyz, directLighting(rayData.origin.xyz, normals, tbn[2], vec3(random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy))), 10000.f).xyz).xyz);
  };

  //
  //reuseLight(rayData);
  rayData.origin.xyz += outRayNormal(rayData.direction.xyz, tbn[2]) * 0.0001f;

  //
  passed.validRay = passed.alphaPassed ? true : dot(rayData.direction.xyz, tbn[2]) > 0.f;

  // 
  return rayData;
};

//
RayData pathTrace(inout RayData rayData, inout PathTraceOutput outp, inout uint type) {
  //for (uint32_t i=0;i<3;i++) {
  uint R=0, T=0;

  // sorry, I hadn't choice
  float currentT = 0.f;
  uvec4 lastIndices = outp.indices;
  vec3 lastNormal = outp.normal;

  //
  while (R<2 && T<2) {
    if (luminance(rayData.energy.xyz) < 0.001f) { break; };

    // 
    IntersectionInfo intersection;
    {
      intersection.barycentric = vec3(0.f.xxx);
      intersection.hitT = 10000.f;
      intersection.instanceId = 0u;
      intersection.geometryId = 0u;
      intersection.primitiveId = 0u;
    };

    //
    intersection = traceRaysOpaque(instancedData, intersection, rayData, 10000.f);
    intersection = traceRaysTransparent(instancedData, intersection, rayData, intersection.hitT, true);

    //
    if (!all(lessThanEqual(intersection.barycentric, 0.f.xxx)) && intersection.hitT < 10000.f) {
      PassData pass;
      pass.alphaColor = vec4(1.f.xxx, 1.f);
      pass.alphaPassed = false;
      pass.diffusePass = false;
      pass.normals = vec3(0.f.xxx);
      pass.validRay = false;

      //
      rayData = handleIntersection(rayData, intersection, pass, type);
      R++; currentT += intersection.hitT;

      // 
      lastNormal = pass.normals.xyz;
      lastIndices = uvec4(intersection.instanceId, intersection.geometryId, intersection.primitiveId, 0u);

      //
      if (pass.diffusePass && !outp.surfaceFound) { 
        outp.surfaceFound = true;
        outp.hitT = currentT;
        outp.indices = lastIndices;
        outp.normal = lastNormal;
      };

      //
      if (!pass.validRay) { break; };

    } else 
    {
      const vec4 skyColor = vec4(texture(sampler2D(textures[background], samplers[0]), lcts(rayData.direction.xyz)).xyz, 0.f);

      // suppose last possible hit-point
      rayData.emission += f16vec4(trueMultColor(rayData.energy.xyz, skyColor.xyz), 0.f);
      rayData.energy.xyz *= f16vec3(0.f.xxx);
      if (!outp.surfaceFound) {
        // sorry, I hadn't choice
        outp.surfaceFound = true;
        outp.indices = lastIndices;
        outp.normal = lastNormal;
        if (R == 0) {
          outp.hitT = currentT = 10000.f;
          rayData.origin.xyz = vec4(0.f.xxx, 1.f) * constants.lookAtInverse[0];
        } else {
          outp.hitT = currentT;
        };
      };
      break;
    }
  };
  return rayData;
};

//
PathTraceOutput pathTraceCommand(inout PathTraceCommand cmd, inout uint type) {
  const bool needsDiffuse = cmd.diffuseColor.a >= 0.001f && luminance(cmd.diffuseColor.xyz) > 0.f;
  const bool needsReflection = cmd.reflCoef >= 0.001f;
  const bool needsTransparency = cmd.diffuseColor.a < 1.f;

  // replace by reflection
  bool validTracing = !all(lessThanEqual(cmd.intersection.barycentric, 0.f.xxx));
  if (type == 0 && !needsReflection) { type = needsDiffuse ? 2 : 1; };
  if (type == 1 && !needsTransparency) { type = needsDiffuse ? 2 : 0; };
  if (type == 2 && !needsDiffuse) { type = needsTransparency ? 1 : 0; };
  if (!needsTransparency && !needsReflection && !needsDiffuse) { validTracing = false; };

  //
  vec3 originSeedXYZ = vec3(random(blueNoiseFn(cmd.rayData.launchId.xy)), random(blueNoiseFn(cmd.rayData.launchId.xy)), random(blueNoiseFn(cmd.rayData.launchId.xy)));
  PathTraceOutput outp;
  outp.hitT = 0.f;
  outp.indices = uvec4(0u);
  outp.normal = cmd.normals.xyz;
  outp.indices.w = type;
  outp.surfaceFound = false;

  //
  vec3 rayDirection = cmd.rayData.direction.xyz;
  vec3 rayOrigin = cmd.rayData.origin.xyz;

  //
  if (type == 0) {
    cmd.rayData.direction.xyz = normalize(reflective(originSeedXYZ, cmd.rayData.direction.xyz, cmd.normals.xyz, cmd.PBR.g));;
    cmd.rayData.energy = f16vec4(1.f.xxx, 1.f);//f16vec4(metallicMult(1.f.xxx, cmd.diffuseColor.xyz, cmd.PBR.b), 1.f);
    cmd.rayData.emission = f16vec4(0.f.xxx, 1.f);
    cmd.rayData.energy.xyz = f16vec3(metallicMult(cmd.rayData.energy.xyz, cmd.diffuseColor.xyz, cmd.PBR.b));
  } else 
  if (type == 1) {
    //cmd.rayData.direction.xyz = cmd.rayData.direction.xyz;
    cmd.rayData.energy = f16vec4(1.f.xxx, 1.f);//f16vec4(metallicMult(1.f.xxx, materialPix.color[MATERIAL_ALBEDO].xyz, metallicFactor), 1.f);
    cmd.rayData.emission = f16vec4(0.f.xxx, 1.f);
    //cmd.rayData.energy.xyz = f16vec3(metallicMult(cmd.rayData.energy.xyz, cmd.diffuseColor.xyz, cmd.PBR.b));
  } else {
    cmd.rayData.direction.xyz = normalize(randomCosineWeightedHemispherePoint(originSeedXYZ, cmd.normals));
    cmd.rayData.energy = f16vec4(1.f.xxx, 1.f);
    cmd.rayData.energy.xyz = f16vec3(trueMultColor(cmd.rayData.energy.xyz, cmd.diffuseColor.xyz * (1.f - cmd.emissiveColor.xyz)));
    cmd.rayData.emission = f16vec4(trueMultColor(cmd.rayData.energy.xyz, directLighting(cmd.rayData.origin.xyz, cmd.normals.xyz, cmd.tbn[2], vec3(random(blueNoiseFn(cmd.rayData.launchId.xy)), random(blueNoiseFn(cmd.rayData.launchId.xy)), random(blueNoiseFn(cmd.rayData.launchId.xy))), 10000.f).xyz), 1.f);
  };

  // enforce typic indice
  if (validTracing) {
    cmd.rayData.origin += outRayNormal(cmd.rayData.direction.xyz, cmd.tbn[2].xyz) * 0.0001f;
    cmd.rayData = pathTrace(cmd.rayData, outp, type);
  };

  //
  float transpCoef = clamp(1.f - cmd.diffuseColor.a, 0.f, 1.f);
  float reflCoef = clamp(cmd.reflCoef, 0.f, 1.f);

  //
  /*if (type == 0) {
    cmd.rayData.emission.xyz = f16vec3(metallicMult(cmd.rayData.emission.xyz, cmd.diffuseColor.xyz, cmd.PBR.b));
  } else 
  if (type == 1) {
    //cmd.rayData.emission.xyz = f16vec3(metallicMult(cmd.rayData.emission.xyz, cmd.diffuseColor.xyz, cmd.PBR.b));
  } else {
    
  };*/

  //
  //if (type == 2) {
    //cmd.rayData.emission.xyz = f16vec3(trueMultColor(cmd.rayData.emission.xyz, cmd.diffuseColor.xyz * (1.f - cmd.emissiveColor.xyz)));
  //};

  // 
  vec4 additional = cmd.rayData.emission;
  if (type == 0) { additional *= vec4(reflCoef.xxx, 1.f); };
  if (type == 1) { additional *= vec4(((1.f - reflCoef) ).xxx, 1.f); };
  if (type == 2) { additional *= vec4(((1.f - reflCoef) ).xxx, 1.f); };

  //
  // My GPU probably is broken, `hitData` doesn't supported correctly, or not enough memory

  uint hitId = cmd.rayData.launchId.x + cmd.rayData.launchId.y * UR(deferredBuf.extent).x;
  //uint hitId = atomicAdd(counters[HIT_COUNTER], 1u);
  //RayHitInfoRef hitInfo = getHitInfo(hitId);//getHitInfo(hitId);
  PixelHitInfoRef hitInfo = getNewHit(hitId, type);

  //
  if (
    //hitId < (rayCount.x * rayCount.y) &&
    hitId < (UR(deferredBuf.extent).x * UR(deferredBuf.extent).y) && 
    uint(cmd.rayData.launchId.x) < UR(deferredBuf.extent).x && 
    uint(cmd.rayData.launchId.y) < UR(deferredBuf.extent).y
  ) {
    //hitInfo.color = clampColW(additional);
    hitInfo.indices[0] = uvec4(cmd.intersection.instanceId, cmd.intersection.geometryId, cmd.intersection.primitiveId, type);
    hitInfo.indices[1] = uvec4(outp.indices.xyz, pack32(cmd.rayData.launchId));
    hitInfo.origin.xyz = rayOrigin;
    hitInfo.direct.xyz = f16vec3(rayDirection);
    hitInfo.normal.xyz = f16vec3(cmd.tbn[2]);
    hitInfo.origin.w = outp.hitT;

    // dedicated distribution broken, no enough memory or GPU was broken, or `hitData` broken
    // avoid critical error for skyboxed, also near have more priority... also, transparency may incorrect, so doing some exception
    PixelSurfaceInfoRef surfaceInfo = getPixelSurface(cmd.rayData.launchId.x + cmd.rayData.launchId.y * UR(deferredBuf.extent).x);
    accumulate(surfaceInfo, type, clampColW(additional));
    atomicOr(surfaceInfo.flags[type], 1u);
  };

  // 
  return outp;
};

// 
void prepareHit(in uint pixelId, inout uint type) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pixelId);

  //
  surfaceInfo.accum[type] = cvtRgb16Float(clampCol(cvtRgb16Acc(surfaceInfo.color[type])));
  surfaceInfo.color[type] = (surfaceInfo.flags[type]&1) > 0 ? TYPE(0u) : surfaceInfo.color[type];
  surfaceInfo.flags[type] = 0;

  //
  PixelHitInfoRef hitInfo = getNewHit(pixelId, type);
  PixelHitInfoRef newHitInfo = getRpjHit(pixelId, type);
  newHitInfo.indices = hitInfo.indices; hitInfo.indices[0] = uvec4(0u), hitInfo.indices[1] = uvec4(0u);
  newHitInfo.origin = hitInfo.origin; hitInfo.origin = vec4(0.f);
  newHitInfo.direct = hitInfo.direct; hitInfo.direct = f16vec3(0.f);
  newHitInfo.normal = hitInfo.normal; hitInfo.normal = f16vec3(0.f);
};

#endif
