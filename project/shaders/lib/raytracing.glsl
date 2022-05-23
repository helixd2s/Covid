#ifndef RAYTRACE_DEF
#define RAYTRACE_DEF

// 
#include "./native.glsl"
#include "./fresnel.glsl"
#include "./random.glsl"
#include "./sphere.glsl"

//
const vec4 sunSphere = vec4(1000.f, 5000.f, 1000.f, 200.f);
const vec3 sunColor = vec3(0.95f, 0.9f, 0.8f) * 20000.f;
const vec4 skyColor = vec4(vec3(135.f,206.f,235.f)/vec3(255.f,255.f,255.f), 1.f);

// 
struct PassData {
  vec4 alphaColor;
  bool alphaPassed;
  bool diffusePass;
  vec3 normals;
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
  vec3 normals; float hitT;
  uvec4 indices;
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
  const bool hasIntersection = intersect(vec4(SO, sunSphere.w), rayData.origin.xyz, rayData.direction.xyz, t);
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
  return vec4(clampCol(rayData.emission).xyz, 0.f);
};

//
RayData reuseLight(inout RayData rayData);

// 
RayData handleIntersection(inout RayData rayData, inout IntersectionInfo intersection, inout PassData passed, in uint type) {
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
  return rayData;
};

//
RayData pathTrace(inout RayData rayData, inout float hitDist, inout vec3 firstNormal, inout uvec4 firstIndices, in uint type) {
  //
  bool surfaceFound = false;
  float currentT = 0.f;
  //for (uint32_t i=0;i<3;i++) {
  uint R=0, T=0;

  // sorry, I hadn't choice
  uvec4 lastIndices = firstIndices;
  vec3 lastNormal = firstNormal;

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
    //IntersectionInfo intersection = traceRaysOpaque(instancedData, rayData, 10000.f);

    //
    if (!all(lessThanEqual(intersection.barycentric, 0.f.xxx)) && intersection.hitT < 10000.f) {
      //PassData opaquePass, pass;
      PassData pass;
      pass.alphaColor = vec4(1.f.xxx, 1.f);
      pass.alphaPassed = false;
      pass.diffusePass = false;
      pass.normals = vec3(0.f.xxx);
      //opaquePass = pass;

      //
      rayData = handleIntersection(rayData, intersection, pass, type);

      // if translucent over opaque (decals)
      //if (pass.alphaPassed && opaqueIntersection.hitT <= (intersection.hitT + 0.0001f)) {
        //intersection = opaqueIntersection;
        //rayData = handleIntersection(rayData, intersection, opaquePass);
        //rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, pass.alphaColor.xyz));
        //pass = opaquePass;
      //};

      //
      //if (pass.alphaPassed) { T++; } else { R++; };
      R++; currentT += intersection.hitT;

      // 
      lastNormal = pass.normals.xyz;
      lastIndices = uvec4(intersection.instanceId, intersection.geometryId, intersection.primitiveId, 0u);

      //
      if (pass.diffusePass && !surfaceFound) { 
        hitDist = currentT;
        surfaceFound = true;
        firstIndices = uvec4(intersection.instanceId, intersection.geometryId, intersection.primitiveId, 0u);
        firstNormal = pass.normals.xyz;
      };

    } else 
    {
      const vec4 skyColor = fromLinear(vec4(texture(sampler2D(textures[background], samplers[0]), lcts(rayData.direction.xyz)).xyz, 0.f));

      // suppose last possible hit-point
      rayData.emission += f16vec4(trueMultColor(rayData.energy.xyz, gamma3(toLinear(skyColor.xyz))), 0.f);
      rayData.energy.xyz *= f16vec3(0.f.xxx);
      if (!surfaceFound) {
        // sorry, I hadn't choice
        firstIndices = lastIndices;
        firstNormal = lastNormal;
        if (type == 1 || R == 0) {
          hitDist = currentT = 10000.f;
          rayData.origin.xyz = vec4(0.f.xxx, 1.f) * constants.lookAtInverse[0];
        } else {
          hitDist = currentT;
        };
        surfaceFound = true;
      };
      break;
    }
  };
  return rayData;
};

//
PathTraceOutput pathTraceCommand(inout PathTraceCommand cmd, in uint type) {
  RayData rayData = cmd.rayData;

  //
  vec3 originSeedXYZ = vec3(random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy)));
  PathTraceOutput outp;
  outp.hitT = 0.f;
  outp.indices = uvec4(0u);
  outp.normals = cmd.normals.xyz;
  outp.indices.w = type;

  //
  if (type == 0) {
    rayData.direction.xyz = normalize(reflective(originSeedXYZ, rayData.direction.xyz, cmd.normals.xyz, cmd.PBR.g));;
    rayData.energy = f16vec4(1.f.xxx, 1.f);//f16vec4(metallicMult(1.f.xxx, cmd.diffuseColor.xyz, cmd.PBR.b), 1.f);
    rayData.emission = f16vec4(0.f.xxx, 0.f);
  } else 
  if (type == 1) {
    //rayData.direction.xyz = rayData.direction.xyz;
    rayData.energy = f16vec4(1.f.xxx, 1.f);//f16vec4(metallicMult(1.f.xxx, materialPix.color[MATERIAL_ALBEDO].xyz, metallicFactor), 1.f);
    rayData.emission = f16vec4(0.f.xxx, 0.f);
  } else {
    rayData.direction.xyz = normalize(randomCosineWeightedHemispherePoint(originSeedXYZ, cmd.normals));
    rayData.energy = f16vec4(1.f.xxx, 1.f);
    rayData.emission = f16vec4(directLighting(rayData.origin.xyz, cmd.normals.xyz, cmd.tbn[2], vec3(random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy)), random(blueNoiseFn(rayData.launchId.xy))), 10000.f).xyz, 1.f);
  };

  //
  vec3 rayDirection = cmd.rayData.direction.xyz;
  vec3 rayOrigin = cmd.rayData.origin.xyz;

  //
  //reuseLight(rayData); // already reprojected!

  // enforce typic indice
  rayData.origin += outRayNormal(rayData.direction.xyz, cmd.tbn[2].xyz) * 0.0001f;
  rayData = pathTrace(rayData, outp.hitT, outp.normals, outp.indices, type);

  //
  float transpCoef = clamp(1.f - cmd.diffuseColor.a, 0.f, 1.f);
  float reflCoef = clamp(cmd.reflCoef, 0.f, 1.f);

  //
  if (type == 0) {
    rayData.emission.xyz = f16vec3(metallicMult(rayData.emission.xyz, cmd.diffuseColor.xyz, cmd.PBR.b));
  } else 
  if (type == 1) {
    //rayData.emission.xyz = f16vec3(metallicMult(rayData.emission.xyz, cmd.diffuseColor.xyz, cmd.PBR.b));
  } else {
    rayData.emission.xyz = f16vec3(trueMultColor(rayData.emission.xyz, cmd.diffuseColor.xyz * (1.f - cmd.emissiveColor.xyz)));
  };

  // 
  vec4 additional = vec4(0.f.xxx, 1.f);
  if (type == 0) { additional = clampCol(rayData.emission * vec4(reflCoef.xxx, 1.f)); };
  if (type == 1) { additional = clampCol(rayData.emission * vec4(((1.f - reflCoef) ).xxx, 1.f)); };
  if (type == 2) { additional = clampCol(rayData.emission * vec4(((1.f - reflCoef) ).xxx, 1.f)); };

  //
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(cmd.rayData.launchId.x + cmd.rayData.launchId.y * extent.x);

  // avoid critical error for skyboxed, also near have more priority... also, transparency may incorrect, so doing some exception
  PixelHitInfoRef hitInfo = getNewHit(cmd.rayData.launchId.x + cmd.rayData.launchId.y * extent.x, type);
  surfaceInfo.color[type] += cvtRgb16Float(additional);

  // 
  hitInfo.indices = uvec4(outp.indices.xyz, type);
  hitInfo.origin.xyz = rayOrigin;
  hitInfo.direct.xyz = rayDirection;

  //
  if (outp.hitT > 0.f && 
    (hitInfo.origin.w <= 0.f || hitInfo.origin.w >= 10000.f || 
      hitInfo.origin.w > 0.f && (
        type == 1 && outp.hitT >= hitInfo.origin.w && outp.hitT < 10000.f || 
        type == 0 && outp.hitT <= hitInfo.origin.w || 
        type == 2 && outp.hitT <= hitInfo.origin.w
      )
    )) {
      hitInfo.origin.w = outp.hitT;
    };

  // 
  return outp;
};

//
void retranslateSurface(inout PathTraceCommand cmd) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(cmd.rayData.launchId.x + cmd.rayData.launchId.y * extent.x);
  surfaceInfo.indices = uvec4(cmd.intersection.instanceId, cmd.intersection.geometryId, cmd.intersection.primitiveId, 0u);
  surfaceInfo.origin.xyz = cmd.rayData.origin.xyz;
  surfaceInfo.normal = cmd.normals;
  surfaceInfo.tex[EMISSION_TEX] = vec4(cmd.emissiveColor, 1.f);
  surfaceInfo.tex[DIFFUSE_TEX] = cmd.diffuseColor;
};

// 
void blankHit(inout PathTraceCommand cmd, in uint type) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(cmd.rayData.launchId.x + cmd.rayData.launchId.y * extent.x);
  surfaceInfo.color[type] += cvtRgb16Float(vec4(0.f.xxx, 1.f));
  
  //
  PixelHitInfoRef hitInfo = getNewHit(cmd.rayData.launchId.x + cmd.rayData.launchId.y * extent.x, type);
  hitInfo.origin = vec4(cmd.rayData.origin.xyz, 0.f);
  hitInfo.direct = vec4(cmd.rayData.direction.xyz, 0.f);
};

// 
void retranslateHit(in uint pixelId, in uint type, in vec3 origin) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pixelId);
  //surfaceInfo.color[type] = vec4(0.f);
  //surfaceInfo.color[type] = cvtRgb16Float(surfaceInfo.accum[type]);

  //
  PixelHitInfoRef newHitInfo = getNewHit(pixelId, type);
  PixelHitInfoRef hitInfo = getRpjHit(pixelId, type);
  
  //newHitInfo.indices = hitInfo.indices;
  //newHitInfo.origin = hitInfo.origin;
};

// 
void retranslateBackHit(in uint pixelId, in uint type) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pixelId);
  if (surfaceInfo.color[type].w > 0.f) {
    surfaceInfo.accum[type] = surfaceInfo.color[type];
    surfaceInfo.color[type] = cvtRgb16Float(vec4(0.f));

    //
    PixelHitInfoRef hitInfo = getNewHit(pixelId, type);
    PixelHitInfoRef newHitInfo = getRpjHit(pixelId, type);
    newHitInfo.indices = hitInfo.indices; hitInfo.indices = uvec4(0u);
    newHitInfo.origin = hitInfo.origin; hitInfo.origin = vec4(0.f);
    newHitInfo.direct = hitInfo.direct; hitInfo.direct = vec4(0.f);
  };
};

// 
void reprojHit(in uint pixelId, in uint type) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pixelId);
  PixelHitInfoRef newHitInfo = getRpjHit(pixelId, type);
  PixelHitInfoRef hitInfo = getNewHit(pixelId, type);
  if (surfaceInfo.color[type].w > 0.f) {
    newHitInfo.indices = hitInfo.indices;
    newHitInfo.origin = hitInfo.origin;
    newHitInfo.direct = hitInfo.direct;
  };
};

// 
void backgroundHit(in uint pixelId, in uint type, in vec3 origin, in vec4 color) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pixelId);
  surfaceInfo.color[type] += cvtRgb16Float((type == 2) ? /*vec4(1.f.xxxx)*/color : vec4(0.f.xxx, 1.f));
  if (type == 2) { surfaceInfo.origin = origin; };

  //
  PixelHitInfoRef hitInfo = getNewHit(pixelId, type);
  hitInfo.indices = uvec4(0u.xxx, type);
  hitInfo.origin = vec4(0.f.xxxx);//vec4(vec4(0.f.xxx, 1.f) * constants.lookAtInverse[0], 10000.f);
};

#endif
