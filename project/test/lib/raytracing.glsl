#ifndef RAYTRACE_DEF
#define RAYTRACE_DEF

// 
#include "./native.glsl"

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
IntersectionInfo traceRaysOpaque(in InstanceAddressInfo instance, in RayData rays, in float maxT) {
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, accelerationStructureEXT(instance.accelStruct), gl_RayFlagsOpaqueEXT, 0xff, rays.origin.xyz, 0.001f, rays.direction.xyz, maxT);

    // 
    while(rayQueryProceedEXT(rayQuery)) {
        rayQueryConfirmIntersectionEXT(rayQuery);
    };

    //
    IntersectionInfo result;
    {
        result.barycentric = vec3(0.f.xxx);
        result.hitT = maxT;
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
    while(rayQueryProceedEXT(rayQuery)) {
        bool isOpaque = true;

        {   // compute intersection opacity
            uint instanceId = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
            uint geometryId = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, false);
            uint primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false);
            GeometryInfo geometryInfo = getGeometry(instance, instanceId, geometryId);
            uvec3 indices = readTriangleIndices(geometryInfo.indices, primitiveId);
            vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
            GeometryExtData geometry = getGeometryData(geometryInfo, primitiveId);
            GeometryExtAttrib interpol = interpolate(geometry, attribs);
            mat3x3 tbn = getTBN(interpol);
            MaterialPixelInfo material = handleMaterial(getMaterialInfo(geometryInfo), interpol.data[VERTEX_TEXCOORD].xy, tbn);

            if (material.color[MATERIAL_ALBEDO].a < 0.001f) {
                isOpaque = false;
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
        result.hitT = maxT;
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



#endif
