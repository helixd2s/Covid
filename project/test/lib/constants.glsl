#ifndef CONSTANTS_DEF
#define CONSTANTS_DEF

//
const float PI = 3.1415926535897932384626422832795028841971f;
const float TWO_PI = 6.2831853071795864769252867665590057683943f;
const float SQRT_OF_ONE_THIRD = 0.5773502691896257645091487805019574556476f;
const float E = 2.7182818284590452353602874713526624977572f;
const float INV_PI = 0.3183098861837907f;
const float TWO_INV_PI = 0.6366197723675814f;
const float INV_TWO_PI = 0.15915494309189535f;

//
const uint32_t VERTEX_VERTICES = 0u;
const uint32_t VERTEX_TEXCOORD = 1u;
const uint32_t VERTEX_NORMALS = 2u;
const uint32_t VERTEX_TANGENT = 3u;
const uint32_t VERTEX_BITANGENT = 4u;
const uint32_t MAX_VERTEX_DATA = 5u;

// 
const uint32_t VERTEX_EXT_TEXCOORD = 0u;
const uint32_t VERTEX_EXT_NORMALS = 1u;
const uint32_t VERTEX_EXT_TANGENT = 2u;
const uint32_t MAX_EXT_VERTEX_DATA = 3u;

//
const uint32_t MATERIAL_ALBEDO = 0u;
const uint32_t MATERIAL_NORMAL = 1u;
const uint32_t MATERIAL_PBR = 2u;
const uint32_t MATERIAL_EMISSIVE = 3u;
const uint32_t MAX_MATERIAL_BIND = 4u;

//
const uint DIFFUSE_TEX = 0u;
const uint EMISSION_TEX = 1u;

//
const uint DIFFUSE_TYPE = 2u;
const uint REFLECTION_TYPE = 0u;
const uint TRANSPARENCY_TYPE = 1u;

//
const uint PIXEL_COUNTER = 0u;
const uint WRITE_COUNTER = 1u;
const uint RASTER_COUNTER = 2u;
const uint SURFACE_COUNTER = 3u;

//
const vec3 bary[3] = { vec3(1.f,0.f,0.f), vec3(0.f,1.f,0.f), vec3(0.f,0.f,1.f) };
const float HDR_GAMMA = 2.2f;


#endif
