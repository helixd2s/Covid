#ifndef RAYTRACE_DEF
#define RAYTRACE_DEF

// 
#include "./native.glsl"
#include "./fresnel.glsl"
#include "./random.glsl"

//
layout(scalar) shared PathTraceCommand cmds[4][32];

//
struct PathTraceOutput {
  vec3 normal; float hitT;
  uvec4 indices; bool surfaceFound;
};

// 
#ifdef ENABLE_KHR_RAY_TRACING
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
          fullTransformNormal(instanceInfo, interpol.data[VERTEX_TANGENT].xyz, geometryId, 0);
          fullTransformNormal(instanceInfo, interpol.data[VERTEX_BITANGENT].xyz, geometryId, 0);
          fullTransformNormal(instanceInfo, interpol.data[VERTEX_NORMALS].xyz, geometryId, 0);
          MaterialPixelInfo material = handleMaterial(getMaterialInfo(geometryInfo), interpol.data[VERTEX_TEXCOORD].xy, mat3x3(interpol.data[VERTEX_TANGENT].xyz, interpol.data[VERTEX_BITANGENT].xyz, interpol.data[VERTEX_NORMALS].xyz));

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
#endif

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
#ifdef ENABLE_KHR_RAY_TRACING
  if (BRDF > 0.f && hasIntersection) {
    intersection = traceRaysOpaque(instancedData, intersection, rayData, t);
    intersection = traceRaysTransparent(instancedData, intersection, rayData, intersection.hitT, true);
  };
#endif

  //
  if (hasIntersection && (intersection.hitT + 0.0001f) >= t && t >= 0.f) {
    rayData.emission.xyz += f16vec3(sunColor * BRDF);
  };

  //
  return vec4(rayData.emission.xyz, 0.f);
};

// 
#ifdef ENABLE_KHR_RAY_TRACING
RayData handleIntersection(inout RayData rayData, inout IntersectionInfo intersection, inout PassData passed, inout uint type) {
  InstanceInfo instanceInfo = getInstance(instancedData, intersection.instanceId);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, intersection.geometryId);
  GeometryExtData geometry = getGeometryData(geometryInfo, intersection.primitiveId);
  GeometryExtAttrib attrib = interpolate(geometry, intersection.barycentric);

  //
  const vec4 texcoord = attrib.data[VERTEX_TEXCOORD];

  //
  fullTransform(instanceInfo, attrib.data[VERTEX_VERTICES], intersection.geometryId, 0);
  fullTransformNormal(instanceInfo, attrib.data[VERTEX_TANGENT].xyz, intersection.geometryId, 0);
  fullTransformNormal(instanceInfo, attrib.data[VERTEX_BITANGENT].xyz, intersection.geometryId, 0);
  fullTransformNormal(instanceInfo, attrib.data[VERTEX_NORMALS].xyz, intersection.geometryId, 0);

  //
  const bool inner = false;//dot(tbn[2], rayData.direction.xyz) > 0.f;
  MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), texcoord.xy, mat3x3(attrib.data[VERTEX_TANGENT].xyz,attrib.data[VERTEX_BITANGENT].xyz,attrib.data[VERTEX_NORMALS].xyz));
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
  //passed.origin = attrib.data[VERTEX_VERTICES].xyz;

  //
  rayData.origin.xyz += rayData.direction.xyz * intersection.hitT;//attrib.data[VERTEX_VERTICES].xyz;

  //
  const vec2 seed2 = vec2(random(rayData.launchId.xy), random(rayData.launchId.xy));
  if (random(blueNoiseFn(rayData.launchId.xy)) <= clamp(reflFactor, 0.f, 1.f) && transpCoef < 1.f) { // I currently, have no time for fresnel
    rayData.direction.xyz = reflective(seed2, rayData.direction.xyz, mat3x3(attrib.data[VERTEX_TANGENT].xyz,attrib.data[VERTEX_BITANGENT].xyz,normals), roughnessFactor);
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
    rayData.direction.xyz = cosineWeightedPoint(seed2, mat3x3(attrib.data[VERTEX_TANGENT].xyz,attrib.data[VERTEX_BITANGENT].xyz,normals));
    rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, diffuseColor.xyz));
    rayData.emission.xyz += f16vec3(trueMultColor(rayData.energy.xyz, directLighting(rayData.origin.xyz, normals, attrib.data[VERTEX_NORMALS].xyz, vec3(random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy))), 10000.f).xyz).xyz);
  };

  //
  rayData.origin.xyz += outRayNormal(rayData.direction.xyz, attrib.data[VERTEX_NORMALS].xyz) * 0.0001f;

  //
  passed.validRay = passed.alphaPassed ? true : dot(rayData.direction.xyz, attrib.data[VERTEX_NORMALS].xyz) > 0.f;

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
          outp.hitT = currentT = 9999.f;
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
#endif

//
PathTraceOutput pathTraceCommand(inout PathTraceCommand cmd, in uint type) {
  PathTraceOutput outp;
  outp.hitT = 0.f;
  outp.indices = uvec4(0u);
  outp.normal = cmd.normals.xyz;
  outp.indices.w = type;
  outp.surfaceFound = false;
  vec3 originSeedXYZ = vec3(random(blueNoiseFn(cmd.rayData.launchId.xy)), random(blueNoiseFn(cmd.rayData.launchId.xy)), random(blueNoiseFn(cmd.rayData.launchId.xy)));

  //
  InstanceInfo instanceInfo = getInstance(instancedData, cmd.intersection.instanceId);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, cmd.intersection.geometryId);
  GeometryExtData geometry = getGeometryData(geometryInfo, cmd.intersection.primitiveId);
  GeometryExtAttrib attrib = interpolate(geometry, cmd.intersection.barycentric);

  //
  fullTransform(instanceInfo, attrib.data[VERTEX_VERTICES], cmd.intersection.geometryId, 0);
  fullTransformNormal(instanceInfo, attrib.data[VERTEX_TANGENT].xyz, cmd.intersection.geometryId, 0);
  fullTransformNormal(instanceInfo, attrib.data[VERTEX_BITANGENT].xyz, cmd.intersection.geometryId, 0);
  fullTransformNormal(instanceInfo, attrib.data[VERTEX_NORMALS].xyz, cmd.intersection.geometryId, 0);

  //
  cmd.rayData.origin = attrib.data[VERTEX_VERTICES].xyz;
  RayData startRayData = cmd.rayData;

  //
  const MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), attrib.data[VERTEX_TEXCOORD].xy, mat3x3(attrib.data[VERTEX_TANGENT].xyz,attrib.data[VERTEX_BITANGENT].xyz,attrib.data[VERTEX_NORMALS].xyz));
  cmd.normals = f16vec3(inRayNormal(cmd.rayData.direction.xyz, materialPix.color[MATERIAL_NORMAL].xyz));
  cmd.diffuseColor = f16vec4(toLinear(materialPix.color[MATERIAL_ALBEDO]));
  cmd.emissiveColor = f16vec3(toLinear(materialPix.color[MATERIAL_EMISSIVE].xyz) * 4.f);
  cmd.PBR = f16vec3(materialPix.color[MATERIAL_PBR].xyz);
  cmd.reflCoef = (float(cmd.PBR.b) + mix(fresnel_schlick(0.f, dot(reflect(cmd.rayData.direction.xyz, vec3(attrib.data[VERTEX_NORMALS].xyz)), vec3(attrib.data[VERTEX_NORMALS].xyz))), 0.f, float(cmd.PBR.g)) * (1.f - float(cmd.PBR.b))) * (1.f - luminance(cmd.emissiveColor.xyz));

  //
  const bool needsDiffuse = cmd.diffuseColor.a >= 0.001f && luminance(cmd.diffuseColor.xyz) > 0.f;
  const bool needsReflection = cmd.reflCoef >= 0.f;
  const bool needsTransparency = cmd.diffuseColor.a < 1.f;

  // replace by reflection
  bool validTracing = !all(lessThanEqual(cmd.intersection.barycentric, 0.f.xxx));
  if (type == 0 && !needsReflection  ) { type = needsDiffuse      || !needsTransparency ? 2 : 1; };
  if (type == 1 && !needsTransparency) { type = needsDiffuse      || !needsReflection   ? 2 : 0; };
  if (type == 2 && !needsDiffuse     ) { type = needsTransparency && !needsReflection   ? 1 : 0; };
  if (!needsTransparency && !needsReflection && !needsDiffuse) { validTracing = false; };

  //
  const vec2 seed2 = vec2(random(cmd.rayData.launchId.xy), random(cmd.rayData.launchId.xy));

  //
  if (type == 0) {
    cmd.rayData.direction.xyz = normalize(reflective(seed2, cmd.rayData.direction.xyz, mat3x3(attrib.data[VERTEX_TANGENT].xyz,attrib.data[VERTEX_BITANGENT].xyz,cmd.normals.xyz), cmd.PBR.g));;
    cmd.rayData.energy = f16vec4(1.f.xxx, 1.f);//f16vec4(metallicMult(1.f.xxx, cmd.diffuseColor.xyz, cmd.PBR.b), 1.f);
    cmd.rayData.emission = f16vec4(0.f.xxx, 1.f);
    cmd.rayData.energy.xyz = f16vec3(metallicMult(cmd.rayData.energy.xyz, cmd.diffuseColor.xyz, cmd.PBR.b));
  } else 
  if (type == 1) {
    cmd.rayData.direction.xyz = cmd.rayData.direction.xyz;
    cmd.rayData.energy = f16vec4(1.f.xxx, 1.f);//f16vec4(metallicMult(1.f.xxx, materialPix.color[MATERIAL_ALBEDO].xyz, metallicFactor), 1.f);
    cmd.rayData.emission = f16vec4(0.f.xxx, 1.f);
    cmd.rayData.energy.xyz = f16vec3(metallicMult(cmd.rayData.energy.xyz, cmd.diffuseColor.xyz, cmd.PBR.b));
  } else {
    cmd.rayData.direction.xyz = normalize(cosineWeightedPoint(seed2, mat3x3(attrib.data[VERTEX_TANGENT].xyz,attrib.data[VERTEX_BITANGENT].xyz,cmd.normals.xyz)));
    cmd.rayData.energy = f16vec4(1.f.xxx, 1.f);
    cmd.rayData.energy.xyz = f16vec3(trueMultColor(cmd.rayData.energy.xyz, cmd.diffuseColor.xyz * (1.f - cmd.emissiveColor.xyz)));
    cmd.rayData.emission = f16vec4(trueMultColor(cmd.rayData.energy.xyz, directLighting(cmd.rayData.origin.xyz, cmd.normals.xyz, attrib.data[VERTEX_NORMALS].xyz, vec3(random(blueNoiseFn(cmd.rayData.launchId.xy)), random(blueNoiseFn(cmd.rayData.launchId.xy)), random(blueNoiseFn(cmd.rayData.launchId.xy))), 10000.f).xyz), 1.f);
  };

  // enforce typic indice
  if (validTracing) {
    cmd.rayData.origin += outRayNormal(cmd.rayData.direction.xyz, attrib.data[VERTEX_NORMALS].xyz) * 0.001f;
#ifdef ENABLE_KHR_RAY_TRACING
    cmd.rayData = pathTrace(cmd.rayData, outp, type);
#endif
  };

  //
  float transpCoef = clamp(1.f - cmd.diffuseColor.a, 0.f, 1.f);
  float reflCoef = clamp(cmd.reflCoef, 0.f, 1.f);

  // 
  vec4 additional = cmd.rayData.emission;
  uint hitId = cmd.rayData.launchId.x + cmd.rayData.launchId.y * UR(deferredBuf.extent).x;

  //
  if (
    hitId < (UR(deferredBuf.extent).x * UR(deferredBuf.extent).y) && 
    uint(cmd.rayData.launchId.x) < UR(deferredBuf.extent).x && 
    uint(cmd.rayData.launchId.y) < UR(deferredBuf.extent).y
  ) {
    PixelHitInfoRef hitInfo = getNewHit(hitId, type);
    hitInfo.indices[0] = uvec4(cmd.intersection.instanceId, cmd.intersection.geometryId, cmd.intersection.primitiveId, type);
    hitInfo.indices[1] = uvec4(outp.indices.xyz, pack32(cmd.rayData.launchId));
    hitInfo.origin.xyz = startRayData.origin;
    hitInfo.direct.xyz = f16vec3(startRayData.direction);
    hitInfo.normal.xyz = f16vec3(attrib.data[VERTEX_NORMALS].xyz);
    hitInfo.origin.w = outp.hitT;

    // dedicated distribution broken, no enough memory or GPU was broken, or `hitData` broken
    // avoid critical error for skyboxed, also near have more priority... also, transparency may incorrect, so doing some exception
    PixelSurfaceInfoRef surfaceInfo = getPixelSurface(hitId);
    accumulate(surfaceInfo, type, clampColW(additional));
    atomicOr(surfaceInfo.flags[type], 1u);
    atomicAnd(surfaceInfo.flags[type], ~2u);
  };
  
  //
  cmd.rayData = startRayData;

  // 
  return outp;
};

// 
void pathTraceEnv(inout PathTraceCommand cmd) {
  const uint pixelId = cmd.rayData.launchId.x + cmd.rayData.launchId.y * UR(deferredBuf.extent).x;
  const vec4 skyColor = vec4(texture(sampler2D(textures[background], samplers[0]), lcts(cmd.rayData.direction.xyz)).xyz, 0.f);
  RayData startRayData = cmd.rayData;

  //
  cmd.diffuseColor = f16vec4(vec4(skyColor.xyz, 1.f));
  cmd.emissiveColor = f16vec3(0.f.xxx.xyz);
  cmd.normals = f16vec3(vec3(0.f.xx, 1.f) * toNormalMat(constants.lookAtInverse[0]));
  cmd.rayData.origin = vec4(0.f.xxx, 1.f) * constants.lookAtInverse[0] + cmd.rayData.direction * 10000.f;
  cmd.PBR = f16vec3(0.f.xxx);
  cmd.reflCoef = 0.f;

  //
  if (pixelId < UR(deferredBuf.extent).x * UR(deferredBuf.extent).y && uint(cmd.rayData.launchId.x) < UR(deferredBuf.extent).x && uint(cmd.rayData.launchId.y) < UR(deferredBuf.extent).y) 
  {
    PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pixelId);
    for (uint i=0;i<3;i++) {
      accumulate(surfaceInfo, i, vec4(1.f.xxxx));
      atomicOr(surfaceInfo.flags[i], 1u|2u);

      //
      PixelHitInfoRef hitInfo = getNewHit(pixelId, i);
      hitInfo.origin = vec4(cmd.rayData.origin, 10000.f);
      hitInfo.normal.xyz = f16vec3(vec3(0.f.xx, 1.f) * toNormalMat(constants.lookAtInverse[0]));
      hitInfo.direct.xyz = f16vec3(cmd.rayData.direction.xyz);
      hitInfo.indices[0] = uvec4(0u, 0u, 0u, i);
    };
  };

  //
  cmd.rayData = startRayData;
};

//
void storeData(inout PathTraceCommand cmd) {
  const uint pixelId = cmd.rayData.launchId.x + cmd.rayData.launchId.y * UR(deferredBuf.extent).x;
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pixelId);
  if (pixelId < UR(deferredBuf.extent).x * UR(deferredBuf.extent).y && uint(cmd.rayData.launchId.x) < UR(deferredBuf.extent).x && uint(cmd.rayData.launchId.y) < UR(deferredBuf.extent).y) 
  {
    surfaceInfo.tex[EMISSION_TEX] = f16vec4(cmd.emissiveColor, 1.f);
    surfaceInfo.tex[DIFFUSE_TEX] = cmd.diffuseColor;
    surfaceInfo.tex[PBR_TEX] = f16vec4(cmd.PBR, cmd.reflCoef);

    //
    imageStore(imagesRgba16F[deferredBuf.images[0][4]], ivec2(cmd.rayData.launchId), vec4(cmd.normals * toNormalMat(constants.lookAt[0]), 1.f));
    imageStore(imagesRgba32F[deferredBuf.images[0][5]], ivec2(cmd.rayData.launchId), vec4(cmd.rayData.origin.xyz, 1.f));
  };
};

#endif
