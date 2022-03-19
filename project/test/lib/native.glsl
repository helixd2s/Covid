// 
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require
#extension GL_EXT_ray_query : enable
#extension GL_EXT_ray_tracing : enable
#extension GL_ARB_gpu_shader_int64 : require


// 
layout(set = 0, binding = 0, scalar) uniform MatrixBlock
{
  uint32_t textureIndices[4]; // framebuffers
};

//
layout(set = 1, binding = 0) uniform texture2D textures[];
layout(set = 2, binding = 0) uniform sampler samplers[];
layout(set = 3, binding = 0, rgb10_a2) uniform image2D images[];

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 16) buffer TransformBlock {
  mat3x4 transform[];
};

//
struct TexOrDef {
  uint32_t textureIdPOne; // starts from 1u, count as Id-1u, zero are Null
  uint32_t samplerIdPOne; // starts from 1u, count as Id-1u, zero are Null
  vec4 defValue;
};

//
struct MaterialInfo {
  TexOrDef albedo;
  TexOrDef normal;
  TexOrDef pbr;
};


//
struct BufferViewRegion {
  uint64_t deviceAddress;
  uint32_t stride;
  uint32_t size;
};

//
struct BufferViewInfo {
  BufferViewRegion region;
  uint32_t format;
  uint32_t flags;
};

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer GeometryExtension {
  BufferViewInfo texcoord;
  BufferViewInfo normals;
  BufferViewInfo tangets;
};

//
struct GeometryInfo {
  BufferViewInfo vertices;
  BufferViewInfo indices;
  BufferViewInfo transform;

  //
  uint64_t extensionRef;
  uint64_t materialRef;

  //
  uint32_t primitiveCount;
  uint32_t flags;
};

//
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer GeometryData {
  GeometryInfo infos[];
};

//
struct InstanceInfo {
  mat3x4 transform;
  GeometryData data;
};

//
layout(buffer_reference, scalar, buffer_reference_align = 8) buffer InstanceData {
  InstanceInfo infos[];
};

//
struct InstanceAddressInfo {
  InstanceData data;
  uint64_t accelStruct;
};

// ALWAYS USE ZERO INDEX OF `InstanceDrawDatas`
struct PushConstantData {
  InstanceData data;
  uint32_t drawIndex;
  uint32_t reserved;
};

//
struct InstanceAddressBlock {
  InstanceAddressInfo opaqueAddressInfo;
  InstanceAddressInfo transparentAddressInfo;
};

//
struct SwapchainStateInfo {
  uint32_t image;
  uint32_t index;
};

//
layout(buffer_reference, scalar, buffer_reference_align = 16, align=16) buffer Float4 { vec4 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4, align=4) buffer Float3 { vec3 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 8, align=8) buffer Float2 { vec2 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4, align=4) buffer Float { float data[]; };

//
layout(buffer_reference, scalar, buffer_reference_align = 16, align=16) buffer Uint4 { uvec4 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4, align=4) buffer Uint3 { uvec3 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 8, align=8) buffer Uint2 { uvec2 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4, align=4) buffer Uint { uint data[]; };

//
layout(buffer_reference, scalar, buffer_reference_align = 8, align=8) buffer Half4 { f16vec4 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 2, align=2) buffer Half3 { f16vec3 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4, align=4) buffer Half2 { f16vec2 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 2, align=2) buffer Half { float16_t data[]; };

//
layout(buffer_reference, scalar, buffer_reference_align = 8, align=8) buffer Ushort4 { u16vec4 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 2, align=2) buffer Ushort3 { u16vec3 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4, align=4) buffer Ushort2 { u16vec2 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 2, align=2) buffer Ushort { uint16_t data[]; };

// instead of `0u` (zero) should to be `firstVertex`
uvec4 readAsUint4(in BufferViewInfo bufferViewInfo, in uint32_t index) {
  const uint cCnt = bufferViewInfo.format&3u;
  const uint isHalf = (bufferViewInfo.format>>2)&1u;
  const uint isUint = (bufferViewInfo.format>>3)&1u;
  const uint stride = (bufferViewInfo.region.stride > 0) ? bufferViewInfo.region.stride : ((isHalf == 1u ? 2 : 4) * (cCnt + 1));
  const uint64_t address = bufferViewInfo.region.deviceAddress + index * stride;
  uvec4 uVec4 = uvec4(0u.xxxx);
  if (isHalf == 1u) {
    if (cCnt==0) { uVec4.x    = Ushort (address).data[0u]; };
    if (cCnt==1) { uVec4.xy   = Ushort2(address).data[0u]; };
    if (cCnt==2) { uVec4.xyz  = Ushort3(address).data[0u]; };
    if (cCnt==3) { uVec4.xyzw = Ushort4(address).data[0u]; };
  } else {
    if (cCnt==0) { uVec4.x    = Uint (address).data[0u]; };
    if (cCnt==1) { uVec4.xy   = Uint2(address).data[0u]; };
    if (cCnt==2) { uVec4.xyz  = Uint3(address).data[0u]; };
    if (cCnt==3) { uVec4.xyzw = Uint4(address).data[0u]; };
  };
  return uVec4;
};

//
uvec3 readAsUint3(in BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsUint4(bufferViewInfo, index).xyz; };
uvec2 readAsUint2(in BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsUint4(bufferViewInfo, index).xy; };
uint readAsUint(in BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsUint4(bufferViewInfo, index).x; };

//
vec4 readAsFloat4(in BufferViewInfo bufferViewInfo, in uint32_t index) { return uintBitsToFloat(readAsUint4(bufferViewInfo, index)); };
vec3 readAsFloat3(in BufferViewInfo bufferViewInfo, in uint32_t index) { return uintBitsToFloat(readAsUint3(bufferViewInfo, index)); };
vec2 readAsFloat2(in BufferViewInfo bufferViewInfo, in uint32_t index) { return uintBitsToFloat(readAsUint2(bufferViewInfo, index)); };
float readAsFloat(in BufferViewInfo bufferViewInfo, in uint32_t index) { return uintBitsToFloat(readAsUint(bufferViewInfo, index)); };

//
layout(push_constant) uniform PConstBlock {
  InstanceAddressBlock instancedData;
  PushConstantData instanceDrawInfo;
  SwapchainStateInfo swapchain;
};

//
uvec3 readTriangleIndices(in BufferViewInfo indices, in uint32_t primitiveId) {
  if (indices.region.deviceAddress > 0u) { return readAsUint3(indices, primitiveId*3u); };
  return uvec3(primitiveId*3u+0u,primitiveId*3u+1u,primitiveId*3u+2u);
};

//
mat3x4 readTriangleVertices(in BufferViewInfo vertices, in uvec3 indices) {
  return mat3x4(readAsUint4(vertices,indices.x), readAsUint4(vertices,indices.y), readAsUint4(vertices,indices.z));
};

//
vec4 interpolate(in mat3x4 vertices, in vec3 barycentric) {
  return vertices * barycentric;
};

//
vec4 interpolate(in mat3x4 vertices, in vec2 barycentric) {
  return interpolate(vertices, vec3(1.f-barycentric.x-barycentric.y, barycentric.xy));
};

//
const vec3 bary[3] = { vec3(1.f,0.f,0.f), vec3(0.f,1.f,0.f), vec3(0.f,0.f,1.f) };

//
InstanceInfo getInstance(in InstanceAddressInfo info, in uint32_t index) {
  return info.data.infos[index];
};

//
GeometryInfo getGeometry(in InstanceInfo info, in uint32_t index) {
  return info.data.infos[index];
};

//
mat3x4 getInstanceTransform(in InstanceInfo info) {
  return info.transform;
};

//
mat3x4 getGeometryTransform(in GeometryInfo info) {
  return info.transform.region.deviceAddress > 0 ? TransformBlock(info.transform.region.deviceAddress).transform[0u] : mat3x4(1.f);
};

//
vec4 fullTransform(in InstanceAddressInfo info, in vec4 vertices, in uint32_t instanceId, in uint32_t geometryId) {
  InstanceInfo instance = getInstance(info, instanceId);
  GeometryInfo geometry = getGeometry(instance, geometryId);
  return vec4(vec4(vertices * getGeometryTransform(geometry), 1.f) * getInstanceTransform(instance), 1.f);
};

//
vec4 fullTransform(in InstanceAddressInfo info, in vec3 vertices, in uint32_t instanceId, in uint32_t geometryId) {
  return fullTransform(info, vec4(vertices, 1.f), instanceId, geometryId);
};

//
vec3 fullTransformNormal(in InstanceAddressInfo info, in vec3 normals, in uint32_t instanceId, in uint32_t geometryId) {
  InstanceInfo instance = getInstance(info, instanceId);
  GeometryInfo geometry = getGeometry(instance, geometryId);
  return vec4(vec4(normals, 0.f) * getGeometryTransform(geometry), 0.f) * getInstanceTransform(instance);
};
