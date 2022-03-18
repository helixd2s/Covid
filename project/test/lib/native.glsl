// 
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
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

//
struct PushConstantData {
  uint64_t dataAddress;
  uint32_t drawIndex;
  uint32_t reserved;
};

//
struct InstanceAddressInfo {
  uint64_t data;
  uint64_t accelStruct;
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
struct GeometryExtension {
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

  //
  uint32_t materialId;
  uint32_t primitiveCount;
  uint32_t flags;
  uint32_t reserved;
};

//
layout(push_constant) uniform PConstBlock {
  InstanceAddressBlock instancedData;
  PushConstantData instanceDrawInfo;
  SwapchainStateInfo swapchain;
};
