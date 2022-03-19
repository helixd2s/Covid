#ifndef RAYTRACE_DEF
#define RAYTRACE_DEF

#include "./native.glsl"

struct RayData
{
    vec3 origin; float r0;
    vec3 direction; float r1;
    u16vec2 launchId;
    u16vec2 reserved;
    f16vec4 energy;
};

struct IntersectionInfo 
{
    vec3 barycentric; float hitT;
    uint instanceId, geometryId, primitiveId, reserved0;
};

IntersectionInfo traceRaysOpaque(in InstanceAddressInfo instance, in RayData rays, in float maxT) {
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, accelerationStructureEXT(instance.accelStruct), gl_RayFlagsOpaqueEXT, 0xff, rays.origin.xyz, 0.001f, rays.direction.xyz, maxT);

    // 
    while(rayQueryProceedEXT(rayQuery)) {
        bool isOpaque = true;

        {   // compute intersection opacity
            uint instanceId = rayQueryGetIntersectionInstanceIdEXT(rayQuery, false);
            uint geometryId = rayQueryGetIntersectionGeometryIndexEXT(rayQuery, false);
            uint primitiveId = rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery, false);
            GeometryInfo geometryInfo = getGeometry(instance, instanceId, geometryId);
            uvec3 indices = readTriangleIndices(geometryInfo.indices, primitiveId);
            //AttributeMap attributeMap = readAttributes3x4(geometryInfo.attributes, indices);
            vec2 attribs = rayQueryGetIntersectionBarycentricsEXT(rayQuery, false);
            //AttributeInterpolated attributes = interpolateAttributes(attributeMap, vec3(1.f - attribs.x - attribs.y, attribs));
            //MaterialInfo material = handleMaterial(geometryInfo.primitive.materials, attributes);

            //if (material.baseColorFactor.a < 0.0001f) {
                //isOpaque = false;
            //};

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
