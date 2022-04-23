//
vec3 reflective(in vec3 seed, in vec3 dir, in vec3 normal, in float roughness) {
  return normalize(mix(reflect(dir, normal), randomCosineWeightedHemispherePoint(seed, normal), roughness));
};

//
const vec4 skyColor = vec4(vec3(135.f,206.f,235.f)/vec3(255.f,255.f,255.f), 1.f);

// 
struct PassData {
  vec4 alphaColor;
  bool alphaPassed;
  bool diffusePass;
  vec3 normals;
  vec3 origin;
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
  MaterialPixelInfo materialPix = handleMaterial(getMaterialInfo(geometryInfo), texcoord.xy, tbn);
  const vec3 normals = inRayNormal(rayData.direction, fullTransformNormal(instanceInfo, materialPix.color[MATERIAL_NORMAL].xyz, intersection.geometryId));//materialPix.color[MATERIAL_NORMAL].xyz;

  // 
  vec4 emissiveColor = toLinear(materialPix.color[MATERIAL_EMISSIVE]);
  vec4 diffuseColor = toLinear(materialPix.color[MATERIAL_ALBEDO]);
  float metallicFactor = materialPix.color[MATERIAL_PBR].b;
  float roughnessFactor = materialPix.color[MATERIAL_PBR].g;

  //
  float reflFactor = (metallicFactor + fresnel_schlick(0.f, dot(-rayData.direction.xyz, normals)) * (1.f - metallicFactor)) * (1.f - luminance(emissiveColor.xyz));
  vec3 originSeedXYZ = vec3(random(rayData.launchId.xy), random(rayData.launchId.xy), random(rayData.launchId.xy));

  //
  passed.alphaColor = vec4(1.f.xxx, diffuseColor.a);
  passed.normals = normals;
  passed.origin = vertice.xyz;

  //
  if (random(rayData.launchId.xy) <= reflFactor && diffuseColor.a >= 0.001f) { // I currently, have no time for fresnel
    rayData.direction.xyz = reflective(originSeedXYZ, rayData.direction.xyz, normals, roughnessFactor);
    rayData.energy.xyz = f16vec3(metallicMult(rayData.energy.xyz, diffuseColor.xyz, metallicFactor));
  } else 
  if (random(rayData.launchId.xy) >= diffuseColor.a) {
    rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, mix(passed.alphaColor.xyz, 1.f.xxx, diffuseColor.a)));
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
  };

  // 
  rayData.origin.xyz = vertice.xyz + outRayNormal(rayData.direction.xyz, normals.xyz) * 0.0001f;

  // 
  return rayData;
};

//
RayData pathTrace(in RayData rayData, inout float hitDist, inout vec3 firstNormal, inout uvec4 firstIndices) {
  //
  bool surfaceFound = false;
  float currentT = 0.f;
  for (uint32_t i=0;i<3;i++) {
    if (luminance(rayData.energy.xyz) < 0.001f) { break; };

    // 
    IntersectionInfo opaqueIntersection = traceRaysOpaque(instancedData.opaqueAddressInfo, rayData, 10000.f);
    IntersectionInfo translucentIntersection = traceRaysTransparent(instancedData.opaqueAddressInfo, rayData, 10000.f);
    IntersectionInfo intersection = translucentIntersection.hitT <= opaqueIntersection.hitT ? translucentIntersection : opaqueIntersection;

    //
    if (!all(lessThanEqual(intersection.barycentric, 0.f.xxx))) {
      PassData opaquePass, pass;
      opaquePass.alphaColor = vec4(1.f.xxx, 1.f);
      opaquePass.alphaPassed = false;
      opaquePass.diffusePass = false;
      opaquePass.normals = vec3(0.f.xxx);
      opaquePass.origin = vec3(0.f.xxx);
      pass = opaquePass;

      //
      RayData opaqueRayData = handleIntersection(rayData, opaqueIntersection, opaquePass);
      rayData = handleIntersection(rayData, intersection, pass);

      // if translucent over opaque (decals)
      if (pass.alphaPassed && opaqueIntersection.hitT <= (intersection.hitT + 0.0001f)) {
        rayData = opaqueRayData;
        rayData.energy.xyz = f16vec3(trueMultColor(rayData.energy.xyz, pass.alphaColor.xyz));
        intersection = opaqueIntersection, pass = opaquePass;
      };

      //
      currentT += intersection.hitT;

      // 
      if (pass.diffusePass && !surfaceFound) { 
        hitDist = currentT;
        surfaceFound = true;
        firstIndices = uvec4(intersection.instanceId, intersection.geometryId, intersection.primitiveId, 0u);
        firstNormal = pass.normals.xyz;
      };

    } else {
      const vec3 hitOrigin = rayData.origin.xyz * rayData.direction.xyz * (10000.f - currentT);
      rayData.origin.xyz = hitOrigin;
      rayData.emission.xyz += f16vec3(trueMultColor(rayData.energy.xyz, pow(toLinear(texture(sampler2D(textures[background], samplers[0]), lcts(rayData.direction.xyz)).xyz), 1.f/2.2f.xxx)));
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
#define USE_RASTERIZE_PASS

//
IntersectionInfo rasterize(in InstanceAddressInfo addressInfo, in RayData rayData, in float maxT) {
  const uvec4 indices = texelFetch(texturesU[framebufferAttachments[1]], ivec2(rayData.launchId), 0);
  const vec3 bary = texelFetch(textures[framebufferAttachments[0]], ivec2(rayData.launchId), 0).xyz;

  IntersectionInfo intersection;
  intersection.barycentric = bary.xyz;
  intersection.instanceId = indices[0];
  intersection.geometryId = indices[1];
  intersection.primitiveId = indices[2];
  return intersection;
};
