#ifndef NATIVE_DEF
#define NATIVE_DEF

//
const float PI = 3.1415926535897932384626422832795028841971f;
const float TWO_PI = 6.2831853071795864769252867665590057683943f;
const float SQRT_OF_ONE_THIRD = 0.5773502691896257645091487805019574556476f;
const float E = 2.7182818284590452353602874713526624977572f;
const float INV_PI = 0.3183098861837907f;
const float TWO_INV_PI = 0.6366197723675814f;
const float INV_TWO_PI = 0.15915494309189535f;

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
#extension GL_EXT_debug_printf : require
#extension GL_EXT_control_flow_attributes : require
#extension GL_EXT_shader_realtime_clock : require

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
struct Constants
{
  mat4x4 perspective;
  mat4x4 perspectiveInverse;
  mat3x4 lookAt;
  mat3x4 lookAtInverse;
  mat3x4 previousLookAt;
  mat3x4 previousLookAtInverse;
};

//
struct PixelHitInfo {
  vec4 color;
  vec4 direction;
  vec4 actualDirection;
  uvec4 accum;
  uvec4 indices;
};

//
struct PixelSurfaceInfo {
  vec3 origin;
  vec3 normal;
  vec3 actualOrigin;
  vec3 actualNormal;
  uvec4 indices;
  vec4 emission;
  vec4 diffuse;
};

//
struct PixelInfo {
  PixelHitInfo diffuse;
  PixelHitInfo reflection;
  PixelHitInfo transparency;
  PixelSurfaceInfo surface;
};

//
vec2 lcts(in vec3 direct) { return vec2(fma(atan(direct.z,direct.x),INV_TWO_PI,0.5f), acos(direct.y)*INV_PI); };
vec3 dcts(in vec2 hr) { hr = fma(hr,vec2(TWO_PI,PI),vec2(-PI,0.f)); const float up=-cos(hr.y),over=sqrt(fma(up,-up,1.f)); return vec3(cos(hr.x)*over,up,sin(hr.x)*over); };

//
const float HDR_GAMMA = 2.2f;
vec3 fromLinear(in vec3 linearRGB) { return mix(vec3(1.055)*pow(linearRGB, vec3(1.0/2.4)) - vec3(0.055), linearRGB * vec3(12.92), lessThan(linearRGB, vec3(0.0031308))); }
vec4 fromLinear(in vec4 linearRGB) { return vec4(fromLinear(linearRGB.xyz), linearRGB.w); }
vec3 toLinear(in vec3 sRGB) { return mix(pow((sRGB + vec3(0.055))/vec3(1.055), vec3(2.4)), sRGB/vec3(12.92), lessThan(sRGB, vec3(0.04045))); }
vec4 toLinear(in vec4 sRGB) { return vec4(toLinear(sRGB.xyz), sRGB.w); }

//
vec4 divW(in vec4 coord) {
  return coord.xyzw/coord.w;
};

//
uint sgn(in uint val) { return uint(0 < val) - uint(val < 0); }
uint tiled(in uint sz, in uint gmaxtile) {
  return sz <= 0 ? 0 : (sz / gmaxtile + sgn(sz % gmaxtile));
};

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 1) coherent buffer PixelData {
  PixelInfo pixels[];
};

// 
layout(set = 0, binding = 0, scalar) uniform MatrixBlock
{
  uint32_t framebufferAttachments[4]; // framebuffers
  uvec2 extent; uint frameCounter, reserved0;
  Constants constants;
  PixelData pixelData;
  uint32_t background;

  //TestVertices vertices;
  // for test
  //uint64_t verticesAddress;
};

//
layout(set = 1, binding = 0) uniform texture2D textures[];
layout(set = 1, binding = 0) uniform utexture2D texturesU[];
layout(set = 2, binding = 0) uniform sampler samplers[];
layout(set = 3, binding = 0, rgb10_a2) uniform image2D images[];
layout(set = 3, binding = 0, rgba32f) uniform image2D imagesRgba32F[];
layout(set = 3, binding = 0, rgba16f) uniform image2D imagesRgba16F[];
layout(set = 3, binding = 0, rgba32ui) uniform uimage2D imagesRgba32UI[];
layout(set = 3, binding = 0, r32ui) uniform uimage2D imagesR32UI[];

//
uvec4 readSplit(in uint image, in ivec2 coord) {
  return uvec4(
    imageLoad(imagesR32UI[image], coord * ivec2(4u,1u) + ivec2(0u,0u)).x, 
    imageLoad(imagesR32UI[image], coord * ivec2(4u,1u) + ivec2(1u,0u)).x, 
    imageLoad(imagesR32UI[image], coord * ivec2(4u,1u) + ivec2(2u,0u)).x, 
    imageLoad(imagesR32UI[image], coord * ivec2(4u,1u) + ivec2(3u,0u)).x
  );
};

//
void accumulateSplit(in uint image, in ivec2 coord, in uvec4 data) {
  imageAtomicAdd(imagesR32UI[image], coord * ivec2(4u,1u) + ivec2(0u,0u), data.x); 
  imageAtomicAdd(imagesR32UI[image], coord * ivec2(4u,1u) + ivec2(1u,0u), data.y); 
  imageAtomicAdd(imagesR32UI[image], coord * ivec2(4u,1u) + ivec2(2u,0u), data.z);
  imageAtomicAdd(imagesR32UI[image], coord * ivec2(4u,1u) + ivec2(3u,0u), data.w);
};

//
void accumulateDiffuse(in uint pixelId, in uvec4 data) {
  atomicAdd(pixelData.pixels[pixelId].diffuse.accum.x, data.x);
  atomicAdd(pixelData.pixels[pixelId].diffuse.accum.y, data.y);
  atomicAdd(pixelData.pixels[pixelId].diffuse.accum.z, data.z);
  atomicAdd(pixelData.pixels[pixelId].diffuse.accum.w, data.w);
};

//
void accumulateReflection(in uint pixelId, in uvec4 data) {
  atomicAdd(pixelData.pixels[pixelId].reflection.accum.x, data.x);
  atomicAdd(pixelData.pixels[pixelId].reflection.accum.y, data.y);
  atomicAdd(pixelData.pixels[pixelId].reflection.accum.z, data.z);
  atomicAdd(pixelData.pixels[pixelId].reflection.accum.w, data.w);
};

//
void accumulateTransparency(in uint pixelId, in uvec4 data) {
  atomicAdd(pixelData.pixels[pixelId].transparency.accum.x, data.x);
  atomicAdd(pixelData.pixels[pixelId].transparency.accum.y, data.y);
  atomicAdd(pixelData.pixels[pixelId].transparency.accum.z, data.z);
  atomicAdd(pixelData.pixels[pixelId].transparency.accum.w, data.w);
};

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer TransformBlock {
  mat3x4 transform[];
};

// starts from 1u, count as Id-1u, alter  are Null
struct CTexture { uint32_t textureId, samplerId; };
struct TexOrDef { CTexture texture; vec4 defValue; };

//
struct MaterialInfo {
  TexOrDef texCol[MAX_MATERIAL_BIND];
};

//
struct MaterialPixelInfo {
  vec4 color[MAX_MATERIAL_BIND];
};

// TODO: Parallax Mapping Support...

//
vec4 sampleTex(CTexture tex, in vec2 texcoord, int lod) {
  return textureLod(
    nonuniformEXT(sampler2D(
      nonuniformEXT(textures[nonuniformEXT(tex.textureId)]), 
      nonuniformEXT(samplers[nonuniformEXT(tex.samplerId)])
    )), vec2(texcoord.x,texcoord.y), float(lod)/float(textureQueryLevels(nonuniformEXT(textures[nonuniformEXT(tex.textureId)]))));
};

//
vec4 sampleTex(CTexture tex, in vec2 texcoord) {
  return sampleTex(tex, texcoord, 0);
};

//
vec4 handleTexture(in TexOrDef tex, in vec2 texcoord) {
  if (tex.texture.textureId > 0u && tex.texture.textureId != -1) {
    return sampleTex(tex.texture, texcoord);
  };
  return tex.defValue;
};

// without parallax mapping or normal mapping
MaterialPixelInfo handleMaterial(in MaterialInfo materialInfo, in vec2 texcoord, in mat3x3 tbn) {
  MaterialPixelInfo result;
  [[unroll]] for (uint32_t i=0;i<MAX_MATERIAL_BIND;i++) {
    result.color[i] = handleTexture(materialInfo.texCol[i], texcoord);
  };
  //result.color[MATERIAL_ALBEDO] = vec4(1.f); // for debug
  //result.color[MATERIAL_NORMAL].xyz = tbn[2];
  result.color[MATERIAL_NORMAL].xyz = normalize(tbn * normalize(result.color[MATERIAL_NORMAL].xyz * 2.f - 1.f));
  return result;
};

// but may not to be...
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer MaterialData {
  MaterialInfo infos[];
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
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer GeometryExtension {
  BufferViewInfo bufferViews[MAX_EXT_VERTEX_DATA];
};

//
struct GeometryExtData {
  mat3x4 triData[MAX_VERTEX_DATA];
};

//
struct GeometryExtAttrib {
  vec4 data[MAX_VERTEX_DATA];
};

//
struct GeometryInfo {
  BufferViewInfo vertices;
  BufferViewInfo indices;
  BufferViewInfo transform;

  //
  uint64_t previousRef;
  uint64_t extensionRef;
  uint64_t materialRef;

  //
  uint32_t primitiveCount;
  uint32_t flags;
};

//
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer GeometryData {
  GeometryInfo infos[];
};

//
struct InstanceInfo {
  uint64_t data; uint32_t geometryCount; uint32_t reserved;
  mat3x4 transform;
  mat3x4 prevTransform;
  //mat3x3 normalTransform;
  //uint32_t align;
};

//
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer InstanceData {
  InstanceInfo infos[];
};



//
struct InstanceAddressInfo {
  //InstanceData data;
  uint64_t data;
  uint64_t accelStruct;
  uint32_t instanceCount;
  uint32_t reserved;
};

// ALWAYS USE Alter INDEX OF `InstanceDrawDatas`
struct PushConstantData {
  //InstanceData data;
  uint64_t data;
  uint32_t instanceIndex;
  uint32_t drawIndex;
};

//
struct InstanceAddressBlock {
  InstanceAddressInfo opaqueAddressInfo;
  InstanceAddressInfo transparentAddressInfo;
};

//
struct PingPongStateInfo {
  uint32_t images[12];
  uint32_t previous;
  uint32_t index;
};

//
struct SwapchainStateInfo {
  uint32_t image;
  uint32_t index;
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
uvec4 readAsUint4(in BufferViewInfo bufferViewInfo, in uint32_t index) {
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
vec4 readAsFloat4(in BufferViewInfo bufferViewInfo, in uint32_t index) {
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
uvec3 readAsUint3(in BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsUint4(bufferViewInfo, index).xyz; };
uvec2 readAsUint2(in BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsUint4(bufferViewInfo, index).xy; };
uint readAsUint(in BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsUint4(bufferViewInfo, index).x; };

//
//vec4 readAsFloat4(in BufferViewInfo bufferViewInfo, in uint32_t index) { return uintBitsToFloat(readAsUint4(bufferViewInfo, index)); };
vec3 readAsFloat3(in BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsFloat4(bufferViewInfo, index).xyz; };
vec2 readAsFloat2(in BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsFloat4(bufferViewInfo, index).xy; };
float readAsFloat(in BufferViewInfo bufferViewInfo, in uint32_t index) { return readAsFloat4(bufferViewInfo, index).x; };

//
layout(push_constant, scalar, buffer_reference_align = 1) uniform PConstBlock {
  InstanceAddressBlock instancedData;
  PushConstantData instanceDrawInfo;
  SwapchainStateInfo swapchain;
  PingPongStateInfo pingpong;
};

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
vec4 interpolate(in mat3x4 vertices, in vec3 barycentric) {
  return vertices * barycentric;
};

//
vec4 interpolate(in mat3x4 vertices, in vec2 barycentric) {
  return interpolate(vertices, vec3(1.f-barycentric.x-barycentric.y, barycentric.xy));
};



//
InstanceInfo getInstance(in InstanceData data, in uint32_t index) {
  return data.infos[index];
};

//
InstanceInfo getInstance(in uint64_t data, in uint32_t index) {
  InstanceInfo info;
  info.data = 0u;

  if (data > 0) { info = getInstance(InstanceData(data), index); }; 
  return info;
};

//
InstanceInfo getInstance(in InstanceAddressInfo addressInfo, in uint32_t index) {
  InstanceInfo info;
  info.data = 0u;
  if (index >= 0 && index < addressInfo.instanceCount) {
    info = getInstance(addressInfo.data, index);
  };
  return info;
};

// 
InstanceInfo getInstance(in uint64_t data) { return getInstance(data, 0u); };
InstanceInfo getInstance(in InstanceData data) { return getInstance(data, 0u); };



//
GeometryInfo getGeometry(in GeometryData data, in uint32_t index) {
  return data.infos[index];
};

//
GeometryInfo getGeometry(in uint64_t data, in uint32_t index) {
  // 
  BufferViewRegion nullViewRegion;
  nullViewRegion.deviceAddress = 0u;
  nullViewRegion.stride = 0u;
  nullViewRegion.size = 0u;

  //
  BufferViewInfo nullView;
  nullView.region = nullViewRegion;

  //
  GeometryInfo info;
  info.extensionRef = 0u;
  info.materialRef = 0u;
  info.vertices = nullView;
  info.indices = nullView;
  info.transform = nullView;
  
  // 
  if (data > 0) { info = getGeometry(GeometryData(data), index); };

  // 
  return info;
};

//
GeometryInfo getGeometry(in InstanceInfo instanceInfo, in uint32_t index) {
  // 
  BufferViewRegion nullViewRegion;
  nullViewRegion.deviceAddress = 0u;
  nullViewRegion.stride = 0u;
  nullViewRegion.size = 0u;

  //
  BufferViewInfo nullView;
  nullView.region = nullViewRegion;

  //
  GeometryInfo info;
  info.extensionRef = 0u;
  info.materialRef = 0u;
  info.vertices = nullView;
  info.indices = nullView;
  info.transform = nullView;

  //
  if (index >= 0u && index < instanceInfo.geometryCount) {
    info = getGeometry(instanceInfo.data, index); 
  };

  // 
  return info;
};

//
GeometryInfo getGeometry(in InstanceData data, in uint32_t instanceId, in uint32_t index) {
  return getGeometry(getInstance(data, instanceId), index);
};

//
GeometryInfo getGeometry(in InstanceAddressInfo info, in uint32_t instanceId, in uint32_t index) {
  return getGeometry(getInstance(info, instanceId), index);
};


//
const vec3 bary[3] = { vec3(1.f,0.f,0.f), vec3(0.f,1.f,0.f), vec3(0.f,0.f,1.f) };


//
mat3x4 getInstanceTransform(in InstanceAddressInfo addressInfo, in uint32_t instanceId) {
  InstanceInfo instanceInfo = getInstance(addressInfo, instanceId);
  return instanceInfo.transform;
};

//
mat3x4 getInstanceTransform(in InstanceInfo info) {
  return info.transform;
};

//
mat3x4 getPrevInstanceTransform(in InstanceInfo info) {
  return info.transform;
};

//
mat3x4 getPrevInstanceTransform(in InstanceAddressInfo addressInfo, in uint32_t instanceId) {
  InstanceInfo instanceInfo = getInstance(addressInfo, instanceId);
  return instanceInfo.prevTransform;
};

//
mat3x4 getGeometryTransform(in GeometryInfo info) {
  return info.transform.region.deviceAddress > 0 ? TransformBlock(info.transform.region.deviceAddress).transform[0u] : mat3x4(1.f);
};


//
mat3x4 inverse(in mat3x4 inmat) {
  const mat4x4 temp = transpose(inverse(transpose(mat4x4(inmat[0],inmat[1],inmat[2],vec4(0.f.xxx,1.f)))));
  //const mat4x4 temp = inverse(mat4x4(inmat[0],inmat[1],inmat[2],vec4(0.f.xxx,1.f)));
  return mat3x4(temp[0],temp[1],temp[2]);
};

//
mat3x3 toNormalMat(in mat3x4 inmat) {
  //return transpose(inverse(transpose(mat3x3(inmat))));
  return inverse(transpose(mat3x3(inmat)));
};


//
vec4 fullTransform(in InstanceInfo instance, in vec4 vertices, in uint32_t geometryId) {
  GeometryInfo geometry = getGeometry(instance, geometryId);
  return vec4(vec4(vertices * getGeometryTransform(geometry), 1.f) * getInstanceTransform(instance), 1.f);
};

//
vec4 fullTransform(in InstanceData data, in vec4 vertices, in uint32_t instanceId, in uint32_t geometryId) {
  return fullTransform(getInstance(data, instanceId), vertices, geometryId);
};

//
vec4 fullTransform(in InstanceData data, in vec3 vertices, in uint32_t instanceId, in uint32_t geometryId) {
  return fullTransform(data, vec4(vertices, 1.f), instanceId, geometryId);
};

//
vec4 fullTransform(in InstanceAddressInfo info, in vec4 vertices, in uint32_t instanceId, in uint32_t geometryId) {
  return fullTransform(getInstance(info, instanceId), vertices, geometryId);
};

//
vec4 fullTransform(in InstanceAddressInfo info, in vec3 vertices, in uint32_t instanceId, in uint32_t geometryId) {
  return fullTransform(info, vec4(vertices, 1.f), instanceId, geometryId);
};


//
vec3 fullTransformNormal(in InstanceInfo instance, in vec3 normals, in uint32_t geometryId) {
  GeometryInfo geometry = getGeometry(instance, geometryId);
  return normalize(normalize(normals) * toNormalMat(getInstanceTransform(instance)) * toNormalMat(getGeometryTransform(geometry)));
};

//
vec3 fullTransformNormal(in InstanceData data, in vec3 normals, in uint32_t instanceId, in uint32_t geometryId) {
  return fullTransformNormal(getInstance(data, instanceId), normals, geometryId);
};

//
vec3 fullTransformNormal(in InstanceAddressInfo info, in vec3 normals, in uint32_t instanceId, in uint32_t geometryId) {
  return fullTransformNormal(getInstance(info, instanceId), normals, geometryId);
};

//
vec3 fullPreviousTransformNormal(in InstanceInfo instance, in vec3 normals, in uint32_t geometryId) {
  GeometryInfo geometry = getGeometry(instance, geometryId);
  return normalize(toNormalMat(getPrevInstanceTransform(instance)) * normalize(toNormalMat(getGeometryTransform(geometry)) * normalize(normals)));
};

//
GeometryExtData getGeometryData(in GeometryInfo geometryInfo, in uvec3 indices) {
  GeometryExtData result;

  //
  [[unroll]] for (uint i=0;i<3;i++) { 
    result.triData[VERTEX_TEXCOORD][i] = vec4(0.f.xxxx);
    result.triData[VERTEX_VERTICES][i] = vec4(0.f.xxx, 1.f);
    result.triData[VERTEX_BITANGENT][i] = vec4(0.f.xxxx);
    result.triData[VERTEX_NORMALS][i] = vec4(0.f.xxxx);
    result.triData[VERTEX_TANGENT][i] = vec4(0.f.xxxx);
  };

  //
  result.triData[VERTEX_VERTICES] = readTriangleVertices3One(geometryInfo.vertices, indices);

  // 
  if (geometryInfo.extensionRef > 0u) {
    GeometryExtension extension = GeometryExtension(geometryInfo.extensionRef);
    [[unroll]] for (uint i=0u;i<MAX_EXT_VERTEX_DATA;i++) { result.triData[i+1u] = readTriangleVertices(extension.bufferViews[i], indices); };
  };

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
MaterialInfo getMaterialInfo(in GeometryInfo geometryInfo, in uint32_t materialId) {
  MaterialInfo materialInfo;
  if (geometryInfo.materialRef > 0) {
    materialInfo = MaterialData(geometryInfo.materialRef).infos[materialId];
  };
  return materialInfo;
};

//
MaterialInfo getMaterialInfo(in GeometryInfo geometryInfo) {
  return getMaterialInfo(geometryInfo, 0u);
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
float luminance(in vec3 color) {
  return dot(vec3(color), vec3(0.3f, 0.59f, 0.11f));
};

// for metallic reflection (true-multiply)
vec3 trueMultColor(in vec3 rayColor, in vec3 material) {
  float rfactor = clamp(luminance(max(rayColor,0.f.xxx)), 0.f, 1.f);
  float mfactor = clamp(luminance(max(material,0.f.xxx)), 0.f, 1.f);
  //return rfactor * materialColor + mfactor * rayColor;
  return sqrt((rfactor * max(material,0.f.xxx)) * (mfactor * max(rayColor,0.f.xxx)));
};

// for metallic reflection
vec3 metallicMult(in vec3 rayColor, in vec3 materialColor, in float factor) {
  return mix(rayColor, trueMultColor(rayColor, materialColor), factor);
};

vec3 inRayNormal(in vec3 dir, in vec3 normal) {
  return normalize(faceforward(normal, dir, normal));
};

vec3 outRayNormal(in vec3 dir, in vec3 normal) {
  return normalize(faceforward(normal, dir, normal));
};

//
vec4 cvtRgb16Acc(in uvec4 color) {
  const vec4 minor = (color & 0xFFFF) / 65536.f;
  const vec4 major = ((color & 0xFFFF0000) >> 16);
  return major + minor;
};

//
uvec4 cvtRgb16Float(in vec4 sampled) {
  return uvec4(sampled * 65536.f);
};

#endif
