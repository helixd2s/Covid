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
    uint instanceId, geometryId, primitiveId, reserved0;
};

//
struct PathTraceCommand {
  RayData rayData;
  PixelSurfaceInfoRef surface;
  IntersectionInfo intersection;
  vec3 normals;
  vec3 rayDir;
  vec3 PBR;
  mat3x3 tbn;
  uint pixelId;
  float reflCoef;
  vec4 diffuseColor;
  vec3 emissiveColor;
  bool hasHit;
};

//
struct PathTraceOutput {
  vec3 normals; float hitT;
  uvec4 indices;
};

//
void genTB(in vec3 N, out vec3 T, out vec3 B) {
  const float s = N.z < 0.0 ? -1.0 : 1.0;
  const float a = -1.0 / (s + N.z);
  const float b = N.x * N.y * a;
  T = vec3(1.0 + s * N.x * N.x * a, s * b, -s * N.x);
  B = vec3(b, s + N.y * N.y * a, -N.y);
};

//
vec3 coneSample(in vec3 N, in float cosTmax, in vec2 r) {
  vec3 T, B; genTB(N, T, B);
  r.x *= 2.0 * PI;
  r.y = 1.0 - r.y * (1.0 - cosTmax);
  const float s = sqrt(1.0 - r.y * r.y);
  return T * (cos(r.x) * s) + B * (sin(r.x) * s) + N * r.y;
};

//
bool intersect(in vec4 sphere, in vec3 O, in vec3 D, inout float tmax) {
  const vec3 L = sphere.xyz - O;
  const float tc = dot(D, L);
  const float t = tc - sqrt(sphere.w * sphere.w + tc * tc - dot(L, L));
  if (t > 0.0 && t < tmax) {
    tmax = t; return true;
  }
  return false;
};

//
vec3 reflective(in vec3 seed, in vec3 dir, in vec3 normal, in float roughness) {
  return normalize(mix(reflect(dir, normal), randomCosineWeightedHemispherePoint(seed, normal), roughness * random(seed)));
};



// 
IntersectionInfo traceRaysOpaque(in InstanceAddressInfo instance, in RayData rays, in float maxT) {
  rayQueryEXT rayQuery;
  rayQueryInitializeEXT(rayQuery, accelerationStructureEXT(instance.accelStruct), gl_RayFlagsOpaqueEXT, 0xff, rays.origin.xyz, 0.001f, rays.direction.xyz, maxT);

  // 
  float currentT = 10000.f;
  while(rayQueryProceedEXT(rayQuery)) {
    const float fT = rayQueryGetIntersectionTEXT(rayQuery, false);
    if (fT <= currentT) {
      currentT = fT;
      rayQueryConfirmIntersectionEXT(rayQuery);
    };
  };

  //
  IntersectionInfo result;
  {
    result.barycentric = vec3(0.f.xxx);
    result.hitT = currentT;
    result.instanceId = 0u;
    result.geometryId = 0u;
    result.primitiveId = 0u;
  };

  // 
  if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
    vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
    result.barycentric = vec3(1.f - attribs.x - attribs.y, attribs);
    result.hitT = rayQueryGetIntersectionTEXT(rayQuery, true);
    result.instanceId = rayQueryGetIntersectionInstanceIdEXT(rayQuery, true);
    result.geometryId = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, true);
    result.primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true);
  };

  // 
  return result;
};

// 
IntersectionInfo traceRaysTransparent(in InstanceAddressInfo instance, in RayData rays, in float maxT) {
  rayQueryEXT rayQuery;
  rayQueryInitializeEXT(rayQuery, accelerationStructureEXT(instance.accelStruct), gl_RayFlagsNoOpaqueEXT, 0xff, rays.origin.xyz, 0.001f, rays.direction.xyz, maxT);

  // 
  float currentT = 10000.f;
  while(rayQueryProceedEXT(rayQuery)) {
    bool isOpaque = true;

    {   // compute intersection opacity
      const float fT = rayQueryGetIntersectionTEXT(rayQuery, false);
      if (fT <= currentT) {
        const uint instanceId = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
        const uint geometryId = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, false);
        const uint primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false);
        InstanceInfo instanceInfo = getInstance(instance, instanceId);
        GeometryInfo geometryInfo = getGeometry(instance, instanceId, geometryId);
        const uvec3 indices = readTriangleIndices(geometryInfo.indices, primitiveId);
        const vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
        GeometryExtData geometry = getGeometryData(geometryInfo, primitiveId);
        GeometryExtAttrib interpol = interpolate(geometry, attribs);
        mat3x3 tbn = getTBN(interpol);
        tbn[0] = fullTransformNormal(instanceInfo, tbn[0], geometryId);
        tbn[1] = fullTransformNormal(instanceInfo, tbn[1], geometryId);
        tbn[2] = fullTransformNormal(instanceInfo, tbn[2], geometryId);
        MaterialPixelInfo material = handleMaterial(getMaterialInfo(geometryInfo), interpol.data[VERTEX_TEXCOORD].xy, tbn);

        if (material.color[MATERIAL_ALBEDO].a < 0.001f) {
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
  IntersectionInfo result;
  {
    result.barycentric = vec3(0.f.xxx);
    result.hitT = currentT;
    result.instanceId = 0u;
    result.geometryId = 0u;
    result.primitiveId = 0u;
  };

  // 
  if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionNoneEXT) {
    vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, true);
    result.barycentric = vec3(1.f - attribs.x - attribs.y, attribs);
    result.hitT = rayQueryGetIntersectionTEXT(rayQuery, true);
    result.instanceId = rayQueryGetIntersectionInstanceIdEXT(rayQuery, true);
    result.geometryId = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, true);
    result.primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, true);
  };

  // 
  return result;
};

//
vec4 directLighting(in vec3 O, in vec3 N, in vec3 tN, in vec3 r, in float t) {
  const vec3 SO = sunSphere.xyz + (vec4(0.f.xxx, 1.f) * constants.lookAtInverse);
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
  IntersectionInfo opaqueIntersection = traceRaysOpaque(instancedData.opaqueAddressInfo, rayData, t);
  IntersectionInfo translucentIntersection = traceRaysTransparent(instancedData.opaqueAddressInfo, rayData, opaqueIntersection.hitT);
  IntersectionInfo intersection = translucentIntersection.hitT <= opaqueIntersection.hitT ? translucentIntersection : opaqueIntersection;

  //
  if (hasIntersection && intersection.hitT >= t && t > 0.f) {
    rayData.emission.xyz += f16vec3(sunColor * (weight * clamp(dot( rayData.direction.xyz, N ), 0.f, 1.f)));
    //rayData.emission.w += float16_t(1.f);
  };

  //
  return vec4(clampCol(rayData.emission).xyz, 0.f);
};

// 
RayData handleIntersection(in RayData rayData, in IntersectionInfo intersection, inout PassData passed) {
  InstanceInfo instanceInfo = getInstance(instancedData.opaqueAddressInfo, intersection.instanceId);
  GeometryInfo geometryInfo = getGeometry(instanceInfo, intersection.geometryId);
  GeometryExtData geometry = getGeometryData(geometryInfo, intersection.primitiveId);
  GeometryExtAttrib attrib = interpolate(geometry, intersection.barycentric);

  //
  const vec4 texcoord = attrib.data[VERTEX_TEXCOORD];
  const vec4 vertice = fullTransform(instanceInfo, attrib.data[VERTEX_VERTICES], intersection.geometryId);

  //
  mat3x3 tbn = getTBN(attrib);
  tbn[0] = fullTransformNormal(instanceInfo, tbn[0], intersection.geometryId);
  tbn[1] = fullTransformNormal(instanceInfo, tbn[1], intersection.geometryId);
  tbn[2] = fullTransformNormal(instanceInfo, tbn[2], intersection.geometryId);

  //
  const bool inner = dot(tbn[2], rayData.direction.xyz) > 0.f;
  MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), texcoord.xy, tbn);
  const vec3 normals = inRayNormal(rayData.direction, materialPix.color[MATERIAL_NORMAL].xyz);//materialPix.color[MATERIAL_NORMAL].xyz;

  // 
  vec4 emissiveColor = toLinear(materialPix.color[MATERIAL_EMISSIVE]) * vec4(2.f.xxx, 1.f);
  vec4 diffuseColor = toLinear(materialPix.color[MATERIAL_ALBEDO]);
  float metallicFactor = materialPix.color[MATERIAL_PBR].b;
  float roughnessFactor = materialPix.color[MATERIAL_PBR].g;

  //
  float transpCoef = clamp(1.f - diffuseColor.a, 0.f, 1.f);
  float reflFactor = clamp((metallicFactor + mix(fresnel_schlick(0.f, dot(reflect(rayData.direction.xyz, normals), normals)), 0.f, roughnessFactor) * (1.f - metallicFactor)) * (1.f - luminance(emissiveColor.xyz)), 0.f, 1.f);
  vec3 originSeedXYZ = vec3(random(rayData.launchId.xy), random(rayData.launchId.xy), random(rayData.launchId.xy));

  //
  passed.alphaColor = vec4(mix(diffuseColor.xyz, 1.f.xxx, diffuseColor.a), diffuseColor.a);
  passed.normals = normals;
  //passed.origin = vertice.xyz;

  //
  rayData.origin.xyz = vertice.xyz;

  //
  if (random(rayData.launchId.xy) <= clamp(reflFactor, 0.f, 1.f) && transpCoef < 1.f) { // I currently, have no time for fresnel
    rayData.direction.xyz = reflective(originSeedXYZ, rayData.direction.xyz, normals, roughnessFactor);
    rayData.energy.xyz = f16vec3(metallicMult(rayData.energy.xyz, diffuseColor.xyz, metallicFactor));
  } else 
  if (random(rayData.launchId.xy) <= clamp(transpCoef, inner ? 1.f : 0.f, 1.f)) { // wrong diffuse if inner
    rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, passed.alphaColor.xyz));
    passed.alphaPassed = true;
  } else
  {
    if (luminance(emissiveColor.xyz) > 0.001f) {
      rayData.emission.xyz += f16vec3(trueMultColor(rayData.energy.xyz, emissiveColor.xyz));
      rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, 1.f-emissiveColor.xyz));
    };
    passed.diffusePass = true;
    rayData.direction.xyz = randomCosineWeightedHemispherePoint(originSeedXYZ, normals);
    rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, diffuseColor.xyz));
    rayData.emission.xyz += f16vec3(trueMultColor(rayData.energy.xyz, directLighting(rayData.origin.xyz, normals, tbn[2], vec3(random(rayData.launchId.xy), random(rayData.launchId.xy), random(rayData.launchId.xy)), 10000.f).xyz).xyz);
  };

  // 
  rayData.origin.xyz += outRayNormal(rayData.direction.xyz, tbn[2]) * 0.0001f;

  // 
  return rayData;
};

//
RayData pathTrace(in RayData rayData, inout float hitDist, inout vec3 firstNormal, inout uvec4 firstIndices) {
  //
  bool surfaceFound = false;
  float currentT = 0.f;
  //for (uint32_t i=0;i<3;i++) {
  uint R=0, T=0;
  while (R<3 && T<3) {
    if (luminance(rayData.energy.xyz) < 0.001f) { break; };

    // 
    IntersectionInfo opaqueIntersection = traceRaysOpaque(instancedData.opaqueAddressInfo, rayData, 10000.f);
    IntersectionInfo translucentIntersection = traceRaysTransparent(instancedData.opaqueAddressInfo, rayData, opaqueIntersection.hitT);
    IntersectionInfo intersection = translucentIntersection.hitT <= opaqueIntersection.hitT ? translucentIntersection : opaqueIntersection;

    //
    if (!all(lessThanEqual(intersection.barycentric, 0.f.xxx)) && intersection.hitT < 10000.f) {
      PassData opaquePass, pass;
      opaquePass.alphaColor = vec4(1.f.xxx, 1.f);
      opaquePass.alphaPassed = false;
      opaquePass.diffusePass = false;
      opaquePass.normals = vec3(0.f.xxx);
      pass = opaquePass;

      //
      RayData opaqueRayData = handleIntersection(rayData, opaqueIntersection, opaquePass);
      rayData = handleIntersection(rayData, intersection, pass);

      // if translucent over opaque (decals)
      if (pass.alphaPassed && opaqueIntersection.hitT <= (intersection.hitT + 0.0001f)) {
        opaqueRayData.energy.xyz = f16vec3(trueMultColor(opaqueRayData.energy.xyz, pass.alphaColor.xyz));
        rayData = opaqueRayData, intersection = opaqueIntersection, pass = opaquePass;
      };

      //
      if (pass.alphaPassed) { T++; } else { R++; };
      currentT += intersection.hitT;

      // 
      if (pass.diffusePass && !surfaceFound) { 
        hitDist = currentT;
        surfaceFound = true;
        firstIndices = uvec4(intersection.instanceId, intersection.geometryId, intersection.primitiveId, 0u);
        firstNormal = pass.normals.xyz;
      };

    } else 
    {
      const vec4 skyColor = gamma3(vec4(texture(sampler2D(textures[background], samplers[0]), lcts(rayData.direction.xyz)).xyz, 0.f));
      
      rayData.origin.xyz = vec4(0.f.xxx, 1.f) * constants.lookAtInverse + rayData.direction.xyz * 10000.f;
      rayData.emission += f16vec4(trueMultColor(rayData.energy.xyz, skyColor.xyz), 0.f);
      rayData.energy.xyz *= f16vec3(0.f.xxx);
      if (!surfaceFound) {
        hitDist = currentT = 10000.f;
        surfaceFound = true;
      };
      break;
    }
  };
  return rayData;
};

//
/*IntersectionInfo rasterize(in InstanceAddressInfo addressInfo, in RayData rayData, in float maxT) {
  const uvec4 indices = texelFetch(texturesU[framebufferAttachments[0]], ivec2(rayData.launchId), 0);
  const vec3 bary = texelFetch(textures[framebufferAttachments[1]], ivec2(rayData.launchId), 0).xyz;

  IntersectionInfo intersection;
  intersection.barycentric = bary.xyz;
  intersection.instanceId = indices[0];
  intersection.geometryId = indices[1];
  intersection.primitiveId = indices[2];
  return intersection;
};*/

//
PathTraceOutput pathTraceCommand(in PathTraceCommand cmd, in uint type) {
  RayData rayData = cmd.rayData;

  //
  vec3 originSeedXYZ = vec3(random(rayData.launchId.xy), random(rayData.launchId.xy), random(rayData.launchId.xy));
  PathTraceOutput outp;
  outp.hitT = 0.f;
  outp.indices = uvec4(0u);
  outp.normals = cmd.normals.xyz;
  outp.indices.w = type;

  //
  if (type == 0) {
    rayData.direction.xyz = normalize(reflective(originSeedXYZ, rayData.direction.xyz, cmd.normals.xyz, cmd.PBR.g));;
    rayData.energy = f16vec4(metallicMult(1.f.xxx, cmd.diffuseColor.xyz, cmd.PBR.b), 1.f);
    rayData.emission = f16vec4(0.f.xxx, 1.f);
  } else 
  if (type == 1) {
    //rayData.direction.xyz = rayData.direction.xyz;
    rayData.energy = f16vec4(1.f.xxx, 1.f);//f16vec4(metallicMult(1.f.xxx, materialPix.color[MATERIAL_ALBEDO].xyz, metallicFactor), 1.f);
    rayData.emission = f16vec4(0.f.xxx, 1.f);
  } else {
    rayData.direction.xyz = normalize(randomCosineWeightedHemispherePoint(originSeedXYZ, cmd.normals));
    rayData.energy = f16vec4(trueMultColor(rayData.energy.xyz, 1.f-cmd.emissiveColor.xyz), 1.f);
    rayData.emission = f16vec4(directLighting(rayData.origin.xyz, cmd.normals.xyz, cmd.tbn[2], vec3(random(rayData.launchId.xy), random(rayData.launchId.xy), random(rayData.launchId.xy)), 10000.f));
    rayData.emission.w = 1.hf;
  };

  // enforce typic indice
  rayData.origin += outRayNormal(rayData.direction.xyz, cmd.tbn[2].xyz) * 0.0001f;
  rayData = pathTrace(rayData, outp.hitT, outp.normals, outp.indices);

  //
  float transpCoef = clamp(1.f - cmd.diffuseColor.a, 0.f, 1.f);
  float reflCoef = clamp(cmd.reflCoef, 0.f, 1.f);

  // 
  vec4 additional = vec4(0.f.xxx, 1.f);
  if (type == 0) { additional = clampCol(rayData.emission * vec4(reflCoef.xxx, 1.f)); };
  if (type == 1) { additional = clampCol(rayData.emission * vec4(((1.f - reflCoef) * transpCoef ).xxx, 1.f)); };
  if (type == 2) { additional = clampCol(rayData.emission * vec4(((1.f - reflCoef) * (1.f - transpCoef)).xxx, 1.f)); };

  //
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(cmd.pixelId);

  // avoid critical error for skyboxed, also near have more priority... also, transparency may incorrect, so doing some exception
  PixelHitInfoRef hitInfo = getNewHit(cmd.pixelId, type);
  surfaceInfo.color[type] += additional;

  // 
  hitInfo.indices = uvec4(outp.indices.xyz, type);
  hitInfo.origin.xyz = outp.hitT >= 10000.f ? vec4(0.f.xxx, 1.f) * constants.lookAtInverse : cmd.rayData.origin.xyz;

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
void retranslateSurface(in PathTraceCommand cmd) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(cmd.pixelId);
  surfaceInfo.indices = uvec4(cmd.intersection.instanceId, cmd.intersection.geometryId, cmd.intersection.primitiveId, 0u);
  surfaceInfo.origin.xyz = cmd.rayData.origin.xyz;
  surfaceInfo.normal = cmd.normals;
  surfaceInfo.tex[EMISSION_TEX] = vec4(cmd.emissiveColor.xyz, 1.f);
  surfaceInfo.tex[DIFFUSE_TEX] = vec4(cmd.diffuseColor.xyz, 1.f);
};

// 
void blankHit(in PathTraceCommand cmd, in uint type) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(cmd.pixelId);
  surfaceInfo.color[type] += vec4(0.f.xxx, 1.f);

  //
  PixelHitInfoRef hitInfo = getNewHit(cmd.pixelId, type);
  hitInfo.origin = vec4(cmd.rayData.origin.xyz, 0.f);
};


// 
void retranslateHit(in uint pixelId, in uint type, in vec3 origin) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pixelId);
  surfaceInfo.color[type] = cvtRgb16Acc(surfaceInfo.accum[type]); 
  surfaceInfo.accum[type] = TYPE(0u);

  //
  PixelHitInfoRef newHitInfo = getNewHit(pixelId, type);
  PixelHitInfoRef hitInfo = getRpjHit(pixelId, type);
  newHitInfo.indices = hitInfo.indices, hitInfo.indices = uvec4(0u);
  newHitInfo.origin = hitInfo.origin, hitInfo.origin = vec4(0.f.xxxx);
};

// 
void backgroundHit(in uint pixelId, in uint type, in vec3 origin) {
  PixelSurfaceInfoRef surfaceInfo = getPixelSurface(pixelId);
  surfaceInfo.color[type] += ((type == 2) ? vec4(1.f.xxxx) : vec4(0.f.xxx, 1.f));

  //
  PixelHitInfoRef hitInfo = getNewHit(pixelId, type);
  hitInfo.indices = uvec4(0u.xxx, type);
  hitInfo.origin = vec4(vec4(0.f.xxx, 1.f) * constants.lookAtInverse, 10000.f);
};


#endif
