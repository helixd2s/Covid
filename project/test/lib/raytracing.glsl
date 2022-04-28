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



#endif
