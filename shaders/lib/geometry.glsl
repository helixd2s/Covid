#ifndef GEOMETRY_DEF
#define GEOMETRY_DEF

//
#include "./interface.glsl"

//
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer InstanceInfo {
  uint64_t data; uint32_t geometryCount; uint32_t reserved;
  mat3x4 transform[2];
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

//
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer GeometryInfo {
  BufferViewInfo bufferViews[4u];

  BufferViewInfo indices;
  BufferViewInfo transform;

  //
  uint64_t previousRef;
  //uint64_t extensionRef;
  uint64_t materialRef;

  //
  uint32_t primitiveCount;
  uint32_t flags;
};

//
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Float4 { vec4 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Float3 { vec3 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Float2 { vec2 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Float { float data[]; };

//
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Uint4 { uvec4 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Uint3 { uvec3 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Uint2 { uvec2 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Uint { uint data[]; };

//
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Half4 { f16vec4 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Half3 { f16vec3 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Half2 { f16vec2 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Half { float16_t data[]; };

//
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Ushort4 { u16vec4 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Ushort3 { u16vec3 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Ushort2 { u16vec2 data[]; };
layout(buffer_reference, scalar, buffer_reference_align = 1, align = 1) readonly buffer Ushort { uint16_t data[]; };

// instead of `0u` (zero) should to be `firstVertex`
uvec4 readAsUint4(inout BufferViewInfo bufferViewInfo, in uint32_t index) {
  const uint cCnt = bufferViewInfo.format&3u;
  const uint isHalf = (bufferViewInfo.format>>2)&1u;
  const uint isUint = (bufferViewInfo.format>>3)&1u;
  const uint local = index * (bufferViewInfo.region.stride > 0 ? bufferViewInfo.region.stride : (isHalf == 1u ? 2 : 4) * (cCnt + 1));
  const uint realCnt = tiled(min(max(bufferViewInfo.region.stride, (isHalf == 1u ? 2 : 4) * (cCnt + 1)), max(bufferViewInfo.region.size, 0u)), (isHalf == 1u ? 2 : 4)) - 1u;

  const uint64_t address = bufferViewInfo.region.deviceAddress + local;

  uvec4 uVec4 = uvec4(0u.xxxx);
  if (bufferViewInfo.region.deviceAddress > 0u) {
    if (isHalf == 1u) {
      if (realCnt==0xFFFFFFFFu || realCnt < 0) {} else
      if (realCnt==0) { uVec4.x    = Ushort (address).data[0u]; } else
      if (realCnt==1) { uVec4.xy   = Ushort2(address).data[0u]; } else
      if (realCnt==2) { uVec4.xyz  = Ushort3(address).data[0u]; } else
                      { uVec4.xyzw = Ushort4(address).data[0u]; };
    } else {
      if (realCnt==0xFFFFFFFFu || realCnt < 0) {} else
      if (realCnt==0) { uVec4.x    = Uint (address).data[0u]; } else
      if (realCnt==1) { uVec4.xy   = Uint2(address).data[0u]; } else
      if (realCnt==2) { uVec4.xyz  = Uint3(address).data[0u]; } else
                      { uVec4.xyzw = Uint4(address).data[0u]; };
    };
  };

  return uVec4;
};

//
vec4 readAsFloat4(inout BufferViewInfo bufferViewInfo, in uint32_t index) {
  const uint cCnt = bufferViewInfo.format&3u;
  const uint isHalf = (bufferViewInfo.format>>2)&1u;
  const uint isUint = (bufferViewInfo.format>>3)&1u;
  const uint local = index * (bufferViewInfo.region.stride > 0 ? bufferViewInfo.region.stride : (isHalf == 1u ? 2 : 4) * (cCnt + 1));
  const uint realCnt = tiled(min(max(bufferViewInfo.region.stride, (isHalf == 1u ? 2 : 4) * (cCnt + 1)), bufferViewInfo.region.size-local), (isHalf == 1u ? 2 : 4)) - 1u;

  const uint64_t address = bufferViewInfo.region.deviceAddress + local;

  vec4 fVec4 = vec4(0.f.xxxx);
  if (bufferViewInfo.region.deviceAddress > 0u) {
    if (isHalf == 1u) {
      if (realCnt==0xFFFFFFFFu || realCnt < 0) {} else
      if (realCnt==0) { fVec4.x    = Half (address).data[0u]; } else
      if (realCnt==1) { fVec4.xy   = Half2(address).data[0u]; } else
      if (realCnt==2) { fVec4.xyz  = Half3(address).data[0u]; } else
                      { fVec4.xyzw = Half4(address).data[0u]; };
    } else {
      if (realCnt==0xFFFFFFFFu || realCnt < 0) {} else
      if (realCnt==0) { fVec4.x    = Float (address).data[0u]; } else
      if (realCnt==1) { fVec4.xy   = Float2(address).data[0u]; } else
      if (realCnt==2) { fVec4.xyz  = Float3(address).data[0u]; } else
                      { fVec4.xyzw = Float4(address).data[0u]; };
    };
  };
  return fVec4;
};

//
uvec3 readAsUint3(inout BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsUint4(bufferViewInfo, index).xyz; };
uvec2 readAsUint2(inout BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsUint4(bufferViewInfo, index).xy; };
uint readAsUint(inout BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsUint4(bufferViewInfo, index).x; };

//
//vec4 readAsFloat4(in BufferViewInfo bufferViewInfo, in uint32_t index) { return uintBitsToFloat(readAsUint4(bufferViewInfo, index)); };
vec3 readAsFloat3(inout BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsFloat4(bufferViewInfo, index).xyz; };
vec2 readAsFloat2(inout BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsFloat4(bufferViewInfo, index).xy; };
float readAsFloat(inout BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsFloat4(bufferViewInfo, index).x; };

//
uvec3 readTriangleIndices(in BufferViewInfo indices, in uint32_t primitiveId) {
  if (indices.region.deviceAddress > 0u) { return readAsUint3(indices, primitiveId*3u); };
  return primitiveId*3u+uvec3(0u,1u,2u);
};

//
mat3x4 readTriangleVertices(in BufferViewInfo vertices, in uvec3 indices) {
  return mat3x4(readAsFloat4(vertices,indices.x), readAsFloat4(vertices,indices.y), readAsFloat4(vertices,indices.z));
};

//
mat3x4 readTriangleVertices3One(in BufferViewInfo vertices, in uvec3 indices) {
  return mat3x4(vec4(readAsFloat3(vertices,indices.x), 1.f), vec4(readAsFloat3(vertices,indices.y), 1.f), vec4(readAsFloat3(vertices,indices.z), 1.f));
};

//
InstanceInfo getInstance_(in InstanceAddressInfo addressInfo, in uint32_t index) {
  InstanceInfo info = InstanceInfo(0ul);
  //info.data = 0u;
  index &= 0x7FFFFFFFu;
  if (index >= 0 && index < addressInfo.instanceCount) {
    info = InstanceInfo(addressInfo.data) + index;//getInstance_(addressInfo.data, index);
  };
  return info;
};

//
InstanceInfo getInstance(in InstanceAddressBlock addressInfo, in uint32_t instanceId) {
  return getInstance_(addressInfo.addressInfos[(instanceId&0x80000000u)>>31u], (instanceId&0x7FFFFFFFu));
};

//
InstanceInfo getInstance(in InstanceAddressBlock addressInfo, in uint32_t type, in uint32_t instanceId) {
  return getInstance_(addressInfo.addressInfos[type], instanceId&0x7FFFFFFFu);
};

//
GeometryInfo getGeometry_(in uint64_t data, in uint32_t index) {
  GeometryInfo info = GeometryInfo(0ul);
  if (data > 0) { info = GeometryInfo(data) + index; };
  return info;
};

//
GeometryInfo getGeometry(in InstanceInfo instanceInfo, in uint32_t index) {
  GeometryInfo info = GeometryInfo(0ul);
  if (index >= 0u && index < instanceInfo.geometryCount) {
    info = getGeometry_(instanceInfo.data, index); 
  };
  return info;
};

//
GeometryInfo getGeometry(in InstanceAddressBlock info, in uint32_t instanceId, in uint32_t index) {
  return getGeometry(getInstance(info, instanceId), index);
};

//
GeometryExtData getGeometryData(in GeometryInfo geometryInfo, in uvec3 indices) {
  GeometryExtData result;

  //
  [[unroll]] for (uint i=0;i<3;i++) {
    result.triData[VERTEX_VERTICES][i] = vec4(0.f.xxx, 1.f);
    result.triData[VERTEX_TEXCOORD][i] = vec4(0.f.xxxx);
    result.triData[VERTEX_BITANGENT][i] = vec4(0.f.xxxx);
    result.triData[VERTEX_NORMALS][i] = vec4(0.f.xxxx);
    result.triData[VERTEX_TANGENT][i] = vec4(0.f.xxxx);
  };

  //
  [[unroll]] for (uint i=0u;i<4u;i++) { 
    result.triData[i] = i == 0u ? readTriangleVertices3One(geometryInfo.bufferViews[i], indices) : readTriangleVertices(geometryInfo.bufferViews[i], indices);
  };

  //
  const mat3x4 vp = result.triData[VERTEX_VERTICES];
  const mat3x4 tp = result.triData[VERTEX_TEXCOORD];
  [[unroll]] for (uint32_t i=0;i<3;i++) {
     vec3 T = result.triData[VERTEX_TANGENT][i].xyz;
     vec3 B = result.triData[VERTEX_BITANGENT][i].xyz;
     vec3 N = result.triData[VERTEX_NORMALS][i].xyz;
     float W = result.triData[VERTEX_TANGENT][i].w;

     // if wrong value
     W = W < 0.001 && W > -0.001 ? 1.f : W;

    // 
    const vec3 dp1 = vp[1].xyz - vp[0].xyz, dp2 = vp[2].xyz - vp[0].xyz;
    const vec2 tx1 = tp[1].xy - tp[0].xy, tx2 = tp[2].xy - tp[0].xy;
    const float coef = 1.f / (tx1.x * tx2.y - tx2.x * tx1.y);

    //
    if (length(N.xyz) < 0.001f) { // if N not defined
      N = cross(dp2,dp1);
    };
    N = normalize(N);
    if (length(T.xyz) < 0.001f) { // if T not defined
      T = (dp1.xyz * tx2.yyy - dp2.xyz * tx1.yyy) * coef;
      B = (dp1.xyz * tx2.xxx - dp2.xyz * tx1.xxx) * coef;
    };
    T = normalize(T - dot(T, N) * N);
    if (length(B.xyz) < 0.001f) {  // if B not defined
      B = cross(N,T) * W;
    };
    B = normalize(B - dot(B, N) * N);

    // 
    result.triData[VERTEX_TANGENT][i].xyz = T;
    result.triData[VERTEX_BITANGENT][i].xyz = B;
    result.triData[VERTEX_NORMALS][i].xyz = N;
  };

  // 
  return result;
};

// 
mat3x3 getTBN(in GeometryExtAttrib attrib) {
  return mat3(
    attrib.data[VERTEX_TANGENT].xyz,
    attrib.data[VERTEX_BITANGENT].xyz,
    attrib.data[VERTEX_NORMALS].xyz
  );
};

//
GeometryExtData getGeometryData(in GeometryInfo geometryInfo, in uint32_t primitiveId) {
  return getGeometryData(geometryInfo, readTriangleIndices(geometryInfo.indices, primitiveId));
};

//
GeometryExtAttrib interpolate(in GeometryExtData data, in vec3 barycentric) {
  GeometryExtAttrib result;
  [[unroll]] for (uint i=0u;i<MAX_VERTEX_DATA;i++) { result.data[i] = interpolate(data.triData[i], barycentric); };

  result.data[VERTEX_NORMALS].xyz = normalize(result.data[VERTEX_NORMALS].xyz);
  result.data[VERTEX_TANGENT].xyz = normalize(result.data[VERTEX_TANGENT].xyz);
  result.data[VERTEX_BITANGENT].xyz = normalize(result.data[VERTEX_BITANGENT].xyz);

  return result;
};

//
GeometryExtAttrib interpolate(in GeometryExtData data, in vec2 barycentric) {
  return interpolate(data, vec3(1.f-barycentric.x-barycentric.y, barycentric));
};



//
mat3x4 getInstanceTransform(in InstanceInfo info, in uint previous) {
  return uint64_t(info) > 0 ? info.transform[previous] : mat3x4(1.f);
};

//
mat3x4 getPreviousInstanceTransform(in InstanceInfo info, in uint previous) {
  return uint64_t(info) > 0 ? info.transform[previous] : mat3x4(1.f);
};

//
mat3x4 getGeometryTransform(in GeometryInfo info) {
  TransformBlock tblock = TransformBlock(info.transform.region.deviceAddress);
  if (uint64_t(info) > 0) {
    return info.transform.region.deviceAddress > 0ul ? tblock.transform[0u] : mat3x4(1.f);
  };
  return mat3x4(1.f);
};

//
mat3x4 getInstanceTransform(in InstanceAddressBlock addressInfo, in uint32_t instanceId, in uint previous) {
  InstanceInfo instanceInfo = getInstance(addressInfo, instanceId);
  return uint64_t(instanceInfo) > 0 ? instanceInfo.transform[previous] : mat3x4(1.f);
};

//
vec4 fullTransform(in InstanceInfo instance, in vec4 vertices, in uint32_t geometryId, in uint previous) {
  GeometryInfo geometry = getGeometry(instance, geometryId);
  return vec4(vec4(vertices * getGeometryTransform(geometry), 1.f) * getInstanceTransform(instance, previous), 1.f);
};

//
vec3 fullTransformNormal(in InstanceInfo instance, in vec3 normals, in uint32_t geometryId, in uint previous) {
  GeometryInfo geometry = getGeometry(instance, geometryId);
  return normalize((normalize(normals) * toNormalMat(getGeometryTransform(geometry)) * toNormalMat(getInstanceTransform(instance, previous))));
};

#endif
