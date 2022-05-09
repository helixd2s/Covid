#ifndef PUPET_DEF
#define PUPET_DEF

#include "./native.glsl"
#include "./raytracing.glsl"

float edgeFunction(in vec3 a, in vec3 b, in vec3 c) { return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x); }

//
vec3 computeBary(in vec3 vo, in mat3 M, in mat3x4 vt) {
  mat3x3 vt3 = M * mat3x3(vt[0].xyz-vo, vt[1].xyz-vo, vt[2].xyz-vo);
  vec3 UVW = vec3(
    vt3[2].x*vt3[1].y-vt3[2].y*vt3[1].x,
    vt3[0].x*vt3[2].y-vt3[0].y*vt3[2].x,
    vt3[1].x*vt3[0].y-vt3[1].y*vt3[0].x
  );
  float T = dot(UVW, transpose(vt3)[2].xyz);
  float det = UVW.x + UVW.y + UVW.z;
  return abs(det) > 0.f ? UVW/det : vec3(0.f);
};

//
vec3 computeBary(in vec3 vo, in mat3x4 vt) {
  mat3x3 vt3 = mat3x3(vec3(vt[0].xy, 1.f), vec3(vt[1].xy, 1.f), vec3(vt[2].xy, 1.f));
  float det = determinant(vt3);
  vec3 UVW = inverse(vt3)*vec3(vo.xy,1.f);
  return abs(det) > 0.f ? UVW/transpose(vt)[2].xyz : vec3(0.f);
};

//
IntersectionInfo rasterize(in InstanceAddressInfo addressInfo, in RayData rayData, in float maxT) {
  IntersectionInfo intersection;
  intersection.barycentric = vec3(0.f.xxx);
  intersection.instanceId = 0u;
  intersection.geometryId = 0u;
  intersection.primitiveId = 0u;

  vec3 viewOrigin = vec4(rayData.origin.xyz, 1.f) * constants.lookAt;
  vec3 viewEnd = vec4(rayData.origin.xyz+rayData.direction.xyz, 1.f) * constants.lookAt;
  vec3 viewDir = normalize(viewEnd - viewOrigin);

  vec4 ss = divW(vec4(viewOrigin, 1.f) * constants.perspective);
  ivec2 sc = ivec2(((ss).xy * 0.5f + 0.5f) * extent.xy);
  uint indice = imageLoad(imagesR32UI[pingpong.images[0]], sc).x;

  float currentZ = 1.f;

  for (uint d=0;d<16;d++) {
    if (indice <= 0u) break;

    RasterInfoRef rasterInfo = getRasterInfo(indice-1);
    InstanceInfo instanceInfo = getInstance(instancedData.opaqueAddressInfo, rasterInfo.indices.x);
    GeometryInfo geometryInfo = getGeometry(instanceInfo, rasterInfo.indices.y);
    GeometryExtData geometry = getGeometryData(geometryInfo, rasterInfo.indices.z);

    mat3x3 M = mat3x3(vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f), vec3(-viewDir.xy, 1.f)/viewDir.z);

    for (uint i=0;i<3;i++) { 
      geometry.triData[VERTEX_VERTICES][i] = vec4(fullTransform(instanceInfo, vec4(geometry.triData[VERTEX_VERTICES][i].xyz, 1.f), rasterInfo.indices.y).xyz, 1.f);
      geometry.triData[VERTEX_VERTICES][i] = vec4(geometry.triData[VERTEX_VERTICES][i] * constants.lookAt, 1.f);
    };

    vec3 bary = computeBary(viewOrigin.xyz, M, geometry.triData[VERTEX_VERTICES]);
    vec4 pos = divW((geometry.triData[VERTEX_VERTICES] * bary) * constants.perspective);

    if (any(greaterThan(bary, 0.f.xxx)) && all(greaterThanEqual(bary, 0.f.xxx)) && all(lessThan(bary, 1.f.xxx)) && pos.z <= currentZ) {
      intersection.instanceId = rasterInfo.indices.x;
      intersection.geometryId = rasterInfo.indices.y;
      intersection.primitiveId = rasterInfo.indices.z;
      intersection.barycentric = bary;
      currentZ = pos.z;
    };

    indice = rasterInfo.indices.w;
  };

  // sorry, but my barycentric isn't want to working correctly
  // probably, really needs triangle intersection
  //IntersectionInfo opaqueIntersection = traceRaysOpaque(addressInfo, rayData, maxT);
  //IntersectionInfo translucentIntersection = traceRaysTransparent(addressInfo, rayData, opaqueIntersection.hitT);
  //intersection = translucentIntersection.hitT <= opaqueIntersection.hitT ? translucentIntersection : opaqueIntersection;

  return intersection;
};

#endif
