#pragma once

//
#include <half.hpp>

//
//#define GLM_FORCE_QUAT_DATA_XYZW
#define GLM_FORCE_SWIZZLE

//
#ifdef __cplusplus
#ifndef USE_CMAKE_PCH
#ifdef ALT_ENABLE_GLTF
#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>
#endif

//
#ifdef _WIN32
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#endif
#else
#ifdef __linux__ 
//FD defaultly
#endif
#endif

//
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <robin_hood.h>
#else
#include <cmake_pch.hpp>
#endif

//
#ifndef ANAMED
#define ANAMED alter 
#endif

//
#ifndef VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
#endif

// 
#define VKU_ENABLE_INTERVAL
#include <cpp21.hpp>
#include <vk-utils.hpp>

// 
namespace ANAMED {

    // 
#ifdef VKU_ENABLE_TYPE_SAFE
    namespace ts {
        using namespace type_safe;
    };
#endif

    //
    enum class MemoryUsage : uint32_t {
        eUnknown = 0u,
        eGpuOnly = 1u,
        eCpuToGpu = 2u,
        eGpuToCpu = 3u,
        eCpuOnly = 4u
    };

    //
    enum class BufferType : uint32_t {
        eStorage = 0u,
        eHostMap = 1u,
        eUniform = 2u,
        eVertex = 3u,
        eIndex = 4u,
        eUniversal = 5u,
        //eCounter = 6u
    };

    //
    enum class ImageType : uint32_t {
        eStorage = 0u,
        eTexture = 1u,
        eColorAttachment = 2u,
        eDepthAttachment = 3u,
        eStencilAttachment = 4u,
        eDepthStencilAttachment = 5u,
        eSwapchain = 6u,
        eUniversal = 7u
    };

    // sorted by die order
    enum class HandleType : uint32_t {
        eUnknown = 0u,
        eUploader,
        eAccelerationStructure,
        eBuffer,
        eImage,
        eSampler,
        eDeviceMemory,
        eMemoryAllocator,
        eExtension,
        ePipeline,
        //eFramebuffer, // useless
        //eSwapchain, // useless
        //ePingPong, // useless
        eSurface, // useless?!
        eSemaphore,
        ePipelineLayout,
        eCommandBuffer,
        eQueueFamily,
        eQueue,
        eDevice,
        ePhysicalDevice,
        eInstance
    };

    //
    enum class BufferViewFormat : uint32_t {
        eFloat = 0x0u,
        eFloat2 = 0x1u,
        eFloat3 = 0x2u,
        eFloat4 = 0x3u,
        eHalf = 0x4u,
        eHalf2 = 0x5u,
        eHalf3 = 0x6u,
        eHalf4 = 0x7u,
        eUint = 0x8u,
        eUint2 = 0x9u,
        eUint3 = 0xAu,
        eUint4 = 0xBu,
        eShort = 0xCu,
        eShort2 = 0xDu,
        eShort3 = 0xEu,
        eShort4 = 0xFu,
        eMat3x4 = 0x1Fu,
        eNone = 0x100u
    };

    //
    enum class ExtensionName : uint32_t {
        eUnknown = 0u,
        eMemoryAllocator = 1u,
        eMemoryAllocation = 2u,
        eMemoryAllocatorVma = 3u,
        eMemoryAllocationVma = 4u
    };

    //
    enum class ExtensionInfoName : uint32_t {
        eUnknown = 0u,
        eMemoryAllocator = 1u,
        eMemoryAllocation = 2u,
        eMemoryAllocatorVma = 3u,
        eMemoryAllocationVma = 4u
    };

    //
    enum class FramebufferType : uint32_t {
        eUnknown = 0u,
        eFrame = 1u,
        eCubemap = 2u
    };

    enum class ImageViewPreference : uint32_t {
        eSampled = 0u,
        eStorage = 1u
    };

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct BufferViewFormatBitSet {
        uint32_t countMinusOne : 2;
        uint32_t is16bit : 1;
        uint32_t isUint : 1;

        // 
        BufferViewFormatBitSet(BufferViewFormat const& bvFormat = BufferViewFormat::eUint) {
            memcpy(this, &bvFormat, sizeof(uint32_t));
        };

        // 
        BufferViewFormatBitSet(BufferViewFormatBitSet const& bvFormat) {
            memcpy(this, &bvFormat, sizeof(uint32_t));
        };

        // 
        inline decltype(auto) operator=(BufferViewFormat const& bvFormat) {
            memcpy(this, &bvFormat, sizeof(uint32_t));
        };

        // 
        inline decltype(auto) operator=(BufferViewFormatBitSet const& bvFormat) {
            memcpy(this, &bvFormat, sizeof(uint32_t));
        };

        //
        operator BufferViewFormat& () { return reinterpret_cast<BufferViewFormat&>(*this); };
        operator BufferViewFormat const& () const { return reinterpret_cast<BufferViewFormat const&>(*this); };

        //
        operator BufferViewFormatBitSet& () { return reinterpret_cast<BufferViewFormatBitSet&>(*this); };
        operator BufferViewFormatBitSet const& () const { return reinterpret_cast<BufferViewFormatBitSet const&>(*this); };
    };
#pragma pack(pop)

    //
    enum class AccessFlagBitsSet : VkAccessFlagBits2 {
        eHostMapRead = VkAccessFlagBits2(vk::AccessFlagBits2::eHostRead | vk::AccessFlagBits2::eTransferRead),
        eHostMapWrite = VkAccessFlagBits2(vk::AccessFlagBits2::eHostWrite | vk::AccessFlagBits2::eTransferWrite),
        eHostMapReadWrite = VkAccessFlagBits2(vk::AccessFlagBits2::eHostRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eHostWrite | vk::AccessFlagBits2::eTransferWrite),
        eGeneralRead = VkAccessFlagBits2(vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eShaderRead),
        eGeneralWrite = VkAccessFlagBits2(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eShaderWrite),
        eGeneralReadWrite = VkAccessFlagBits2(vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eShaderWrite),
        eTransferWrite = VkAccessFlagBits2(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite),
        eTransferRead = VkAccessFlagBits2(vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead),
        eTransferReadWrite = VkAccessFlagBits2(vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite),
        eComputeShaderReadWrite = VkAccessFlagBits2(vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite),
        eGraphicsShaderReadWrite = VkAccessFlagBits2(vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite | vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite),
        eShaderReadWrite = VkAccessFlagBits2(vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderSampledRead | vk::AccessFlagBits2::eUniformRead)
    };

    //
    enum class FramebufferState : uint32_t {
        eAttachment = 0u,
        eShaderRead = 1u
    };

    //
    enum class SwapchainState : uint32_t {
        ePresent = 0u,
        eReady = 1u
    };


    //
    class BaseObj;
    class GeometryLevelObj;
    class InstanceLevelObj;
    class ContextObj;
    class InstanceObj;
    class DeviceObj;
    class ResourceObj;
    class QueueFamilyObj;
    class PipelineLayoutObj;
    class DescriptorsObj;
    class PipelineObj;
    class UploaderObj;
    class FramebufferObj;
    class SwapchainObj;
    class SemaphoreObj;
    class MemoryAllocatorObj;
    class SamplerObj;
    class ResourceSparseObj;
    class PingPongObj;
    class DenoiserObj;

    // 
    class GltfLoaderObj;
    class ResourceObj;
    class MemoryAllocatorVma;


    //
    using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    using EXM = cpp21::map_of_shared<ExtensionName, uintptr_t>;
    using EXIF = cpp21::map_of_shared<ExtensionInfoName, cpp21::void_t>;
    using SMAP = cpp21::interval_map<uintptr_t, vk::Buffer>;
    using EXIP = std::unordered_map<ExtensionInfoName, ExtensionName>;

    //
    struct BaseCreateInfo {
        std::shared_ptr<EXIP> extUsed = {};
        std::shared_ptr<EXIF> extInfoMap = {};
    };

    //
    struct ContextCreateInfo : BaseCreateInfo {

    };

    //
    struct DedicatedMemory : BaseCreateInfo {
        vk::Buffer buffer = {};
        vk::Image image = {};
    };

    //
    struct MemoryRequirements : BaseCreateInfo {
        //uint32_t physicalDeviceIndex = 0u;
        MemoryUsage memoryUsage = MemoryUsage::eGpuOnly;
        std::optional<vk::MemoryRequirements> requirements = {};
        std::optional<DedicatedMemory> dedicated = {};
    };

    // 
    struct InstanceCreateInfo : BaseCreateInfo {
        std::string appName = "Alter_APP";
        uint32_t appVersion = VK_MAKE_VERSION(1, 0, 0);
        cpp21::shared_vector<std::string> extensionList = std::vector<std::string>{
          "VK_KHR_surface",
          "VK_EXT_debug_utils",
          "VK_KHR_get_surface_capabilities2",
    #ifdef _WIN32
          "VK_KHR_win32_surface"
    #endif
        };
        cpp21::shared_vector<std::string> layerList = std::vector<std::string>{
            //"VK_LAYER_KHRONOS_synchronization2",
            "VK_LAYER_KHRONOS_validation",
            //"VK_LAYER_KHRONOS_profiles",
            //"VK_LAYER_LUNARG_parameter_validation",
            //"VK_LAYER_LUNARG_core_validation",
            //"VK_LAYER_LUNARG_object_tracker",
            //"VK_LAYER_LUNARG_api_dump"
        };
    };


    //
    struct QueueFamilyCreateInfo : BaseCreateInfo {
        uint32_t queueFamilyIndex = 0u;
        cpp21::shared_vector<float> queuePriorities = std::vector<float>{ 1.f };
        //std::shared_ptr<MSS> infoMap = {};//std::make_shared<MSS>();
    };

    //
    struct MemoryAllocatorCreateInfo : BaseCreateInfo {

    };

    // 
    struct DeviceCreateInfo : BaseCreateInfo {
        cpp21::shared_vector<std::string> extensionList = std::vector<std::string>{
          "VK_KHR_deferred_host_operations",
          "VK_KHR_acceleration_structure",
          "VK_KHR_ray_query",
          "VK_KHR_ray_tracing_pipeline",
          "VK_KHR_swapchain",
          "VK_EXT_multi_draw",
          "VK_KHR_separate_depth_stencil_layouts",
          "VK_EXT_depth_range_unrestricted",
          "VK_EXT_depth_clip_enable",
          //"VK_EXT_depth_clip_control", 
          "VK_EXT_vertex_input_dynamic_state",
          "VK_EXT_conservative_rasterization",
          "VK_EXT_blend_operation_advanced",
          "VK_EXT_validation_cache",
          "VK_KHR_portability_subset",
          "VK_KHR_external_semaphore_win32",
          "VK_KHR_external_semaphore_fd",
          "VK_KHR_external_memory_win32",
          "VK_KHR_external_memory_fd",
          //"VK_KHR_shader_subgroup_uniform_control_flow",
          "VK_KHR_workgroup_memory_explicit_layout",
          "VK_KHR_shared_presentable_image",
          "VK_KHR_shader_clock",
          "VK_KHR_fragment_shading_rate",
          "VK_KHR_pipeline_library",
          "VK_EXT_graphics_pipeline_library",
          "VK_EXT_shader_atomic_float",
          "VK_EXT_shader_atomic_float2",
          "VK_EXT_depth_range_unrestricted",
          "VK_EXT_pageable_device_local_memory",
          "VK_EXT_memory_priority",
          "VK_KHR_fragment_shader_barycentric",
          "VK_KHR_ray_tracing_maintenance1",
          "VK_EXT_fragment_shader_interlock",
          "VK_KHR_push_descriptor"
        };
        cpp21::shared_vector<std::string> layerList = std::vector<std::string>{
        };
        cpp21::shared_vector<QueueFamilyCreateInfo> queueFamilyInfos = std::vector<QueueFamilyCreateInfo>{ QueueFamilyCreateInfo{} };
        uint32_t physicalDeviceGroupIndex = 0u;
        uint32_t physicalDeviceIndex = 0u;
    };

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct CTexture { uint32_t textureId = 0u, samplerId = 0u; };
#pragma pack(pop)

#pragma pack(push, 1)
    __declspec(align(1))
        struct TexOrDef { CTexture texture = {}; glm::vec4 defValue = { 0.f,0.f,0.f,0.f }; };
#pragma pack(pop)

    //
    enum class TextureBind : uint32_t {
        eAlbedo = 0u,
        eNormal = 1u,
        ePBR = 2u,
        eEmissive = 3u,
        eMAX = 4u
    };

    //
    enum class BufferBind : uint32_t {
        // 
        eVertices = 0u,
        eTexcoord = 1u,
        eNormals = 2u,
        eTangent = 3u,
        eBitangent = 4u,

        // 
        //eExtTexcoord = 0u,
        //eExtNormals = 1u,
        //eExtTangent = 2u,
        //eMAX = 3u
    };

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct MaterialInfo {
        TexOrDef texCol[std::to_underlying(TextureBind::eMAX)];
    };
#pragma pack(pop)

    // 
#pragma pack(push, 1)
    __declspec(align(1))
        struct BufferViewRegion {
        uint64_t deviceAddress = 0ull;
        uint32_t stride = 0u;
        uint32_t size = uint32_t(VK_WHOLE_SIZE);
    };
#pragma pack(pop)

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct BufferViewInfo {
        BufferViewRegion region = { .deviceAddress = 0ull, .stride = 0u, .size = 0u };
        BufferViewFormat format = BufferViewFormat::eNone;
        uint32_t flags = 0u;
    };
#pragma pack(pop)

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct GeometryInfo {
        BufferViewInfo bufferViews[4u];
        BufferViewInfo indices;
        BufferViewInfo transform;
        //MaterialInfo materialInfo;

        //
        uint64_t materialRef;
        uint64_t previousRef;
        uint64_t extensionRef;

        //
        uint32_t primitiveCount = 0u;
        vk::GeometryFlagsKHR flags = vk::GeometryFlagBitsKHR::eOpaque;
    };
#pragma pack(pop)



    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct SwapchainStateInfo {
        glm::uvec2 extent = glm::uvec2(0u);
        uint32_t image = uint32_t(0u);
        uint32_t index = uint32_t(-1);
    };
#pragma pack(pop)

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct PingPongStateInfo {
        glm::uvec2 extent = glm::uvec2(0u);
        uint32_t images[2u][6u] = { 0u };
        uint32_t previous = uint32_t(-1);
        uint32_t index = uint32_t(-1);
    };
#pragma pack(pop)

    //
#pragma pack(push, 1)
    struct FramebufferStateInfo {
        glm::uvec2 extent = glm::uvec2(0u);
        uint32_t attachments[2][8]; // framebuffers
    };
#pragma pack(pop)

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct InstanceDrawInfo {
        //uint64_t dataAddress = 0ull;
        uint32_t instanceCount = 0u;
        uint32_t instanceIndex = 0u;

        uint32_t reserved0 = 0u;
        uint32_t drawIndex = 0u;

        decltype(auto) with(uint32_t const& drawIndex) const { auto copy = *this; copy.drawIndex = drawIndex; return copy; };
    };
#pragma pack(pop)

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct InstanceAddressInfo {
        uint64_t data = 0ull;
        uint64_t accelStruct = 0ull;
        uint32_t instanceCount = 0u;
        uint32_t reserved = 0u;
    };
#pragma pack(pop)

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct InstanceAddressBlock {
        InstanceAddressInfo opaqueAddressInfo = {};
        InstanceAddressInfo transparentAddressInfo = {};
    };
#pragma pack(pop)

    //
    using InstanceDevInfo = vk::AccelerationStructureInstanceKHR;

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct InstanceInfo {
        uint64_t geometryReference = 0ull;
        uint32_t geometryCount = 0u;
        vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
        glm::mat3x4 transform = glm::mat3x4(1.f);
        glm::mat3x4 prevTransform = glm::mat3x4(1.f);
    };
#pragma pack(pop)

    //
#pragma pack(push, 1)
    __declspec(align(1))
        struct InstanceDataInfo {
        InstanceDevInfo instanceDevInfo = {};
        InstanceInfo instanceInfo = {};
    };
#pragma pack(pop)

    //
    using DFun = void(BaseObj const*);


    //
    inline decltype(auto) cvtFormat(BufferViewFormat const& format) {
        switch (format) {
        case BufferViewFormat::eFloat: return vk::Format::eR32Sfloat; break;
        case BufferViewFormat::eFloat2: return vk::Format::eR32G32Sfloat; break;
        case BufferViewFormat::eFloat3: return vk::Format::eR32G32B32Sfloat; break;
        case BufferViewFormat::eFloat4: return vk::Format::eR32G32B32A32Sfloat; break;
        case BufferViewFormat::eHalf: return vk::Format::eR16Sfloat; break;
        case BufferViewFormat::eHalf2: return vk::Format::eR16G16Sfloat; break;
        case BufferViewFormat::eHalf3: return vk::Format::eR16G16B16Sfloat; break;
        case BufferViewFormat::eHalf4: return vk::Format::eR16G16B16A16Sfloat; break;
        case BufferViewFormat::eUint: return vk::Format::eR32Uint; break;
        case BufferViewFormat::eUint2: return vk::Format::eR32G32Uint; break;
        case BufferViewFormat::eUint3: return vk::Format::eR32G32B32Uint; break;
        case BufferViewFormat::eUint4: return vk::Format::eR32G32B32A32Uint; break;
        case BufferViewFormat::eShort: return vk::Format::eR16Uint; break;
        case BufferViewFormat::eShort2: return vk::Format::eR16G16Uint; break;
        case BufferViewFormat::eShort3: return vk::Format::eR16G16B16Uint; break;
        case BufferViewFormat::eShort4: return vk::Format::eR16G16B16A16Uint; break;
        default: return vk::Format::eUndefined; break;
        };
        return vk::Format::eUndefined;
    };

    //
    inline decltype(auto) cvtFormatRT(BufferViewFormat const& format) {
        switch (format) {
        case BufferViewFormat::eFloat: return vk::Format::eR32Sfloat; break;
        case BufferViewFormat::eFloat2: return vk::Format::eR32G32Sfloat; break;
        case BufferViewFormat::eFloat3: return vk::Format::eR32G32B32Sfloat; break;
        case BufferViewFormat::eFloat4: return vk::Format::eR32G32B32Sfloat; break;
        case BufferViewFormat::eHalf: return vk::Format::eR16Sfloat; break;
        case BufferViewFormat::eHalf2: return vk::Format::eR16G16Sfloat; break;
        case BufferViewFormat::eHalf3: return vk::Format::eR16G16B16Sfloat; break;
        case BufferViewFormat::eHalf4: return vk::Format::eR16G16B16Sfloat; break;
        case BufferViewFormat::eUint: return vk::Format::eR32Uint; break;
        case BufferViewFormat::eUint2: return vk::Format::eR32G32Uint; break;
        case BufferViewFormat::eUint3: return vk::Format::eR32G32B32Uint; break;
        case BufferViewFormat::eUint4: return vk::Format::eR32G32B32Uint; break;
        case BufferViewFormat::eShort: return vk::Format::eR16Uint; break;
        case BufferViewFormat::eShort2: return vk::Format::eR16G16Uint; break;
        case BufferViewFormat::eShort3: return vk::Format::eR16G16B16Uint; break;
        case BufferViewFormat::eShort4: return vk::Format::eR16G16B16Uint; break;
        default: return vk::Format::eUndefined; break;
        };
        return vk::Format::eUndefined;
    };

    //
    inline decltype(auto) cvtIndex(BufferViewFormat const& format) {
        switch (format) {
        case BufferViewFormat::eUint: return vk::IndexType::eUint32; break;
        case BufferViewFormat::eShort: return vk::IndexType::eUint16; break;
        case BufferViewFormat::eUint3: return vk::IndexType::eUint32; break;
        case BufferViewFormat::eShort3: return vk::IndexType::eUint16; break;
        default: return vk::IndexType::eNoneKHR; break;
        };
        return vk::IndexType::eNoneKHR;
    };

    //
    struct QueueGetInfo {
        uint32_t queueFamilyIndex = 0u;
        uint32_t queueIndex = 0u;
    };

    //
    struct GeometryLevelCreateInfo : BaseCreateInfo {
        cpp21::bucket<GeometryInfo> geometries = std::vector<GeometryInfo>{};
        std::vector<uint32_t> limits = {};
        //union {
          //std::vector<uint32_t> limits = {};
          //std::vector<int32_t> limitsInt;
        //};
        //size_t geometryCount = 1u;
        uintptr_t uploader = 0ull;

        // 
        std::optional<QueueGetInfo> info = QueueGetInfo{};
    };

    //
    struct InstanceLevelCreateInfo : BaseCreateInfo {
        cpp21::bucket<InstanceDataInfo> instances = std::vector<InstanceDataInfo>{};
        uint32_t limit = 0u;
        //std::vector<uint32_t> limits = {};
        //size_t instanceCount = 1u;
        uintptr_t uploader = 0ull;
        bool isTranslucent = false;

        // 
        std::optional<QueueGetInfo> info = QueueGetInfo{};
    };

    //
    constexpr float attachDef0[4] = { 0.f, 0.f, 0.f, 0.f };
    constexpr uint32_t attachDef1[4] = { 0u, 0u, 0u, 0u };

    //
    struct AttachmentLayout {
        vk::ClearValue depthClearValue = vk::ClearValue{ .depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0u} };
        vk::ClearValue stencilClearValue = vk::ClearValue{ .depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0u} };
        std::vector<vk::ClearValue> colorClearValues = {
          vk::ClearValue{.color = vk::ClearColorValue{.uint32 = std::array<uint32_t, 4>{0u,0u,0u,0u}}},
          vk::ClearValue{.color = vk::ClearColorValue{.uint32 = std::array<uint32_t, 4>{0u,0u,0u,0u}}},
          vk::ClearValue{.color = vk::ClearColorValue{.float32 = std::array<float, 4>{0.f,0.f,0.f,0.f}} },
          vk::ClearValue{.color = vk::ClearColorValue{.float32 = std::array<float, 4>{0.f,0.f,1.f,1.f}} },
          vk::ClearValue{.color = vk::ClearColorValue{.float32 = std::array<float, 4>{0.f,0.f,0.f,0.f}} }
        };
        vk::Format depthAttachmentFormat = vk::Format::eD32SfloatS8Uint;//eD32Sfloat;
        vk::Format stencilAttachmentFormat = vk::Format::eD32SfloatS8Uint;//eS8Uint;
        std::vector<vk::Format> colorAttachmentFormats = {
          vk::Format::eR32G32B32A32Uint,
          vk::Format::eR32G32B32A32Uint,
          vk::Format::eR32G32B32A32Sfloat,
          vk::Format::eR32G32B32A32Sfloat,
          vk::Format::eR32G32B32A32Sfloat
        };
        std::vector<vk::PipelineColorBlendAttachmentState> blendStates = {
          vk::PipelineColorBlendAttachmentState{
            .blendEnable = false,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha, // needs pre-mult
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
          },
          vk::PipelineColorBlendAttachmentState{
            .blendEnable = false,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha, // needs pre-mult
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
          },
          vk::PipelineColorBlendAttachmentState{
            .blendEnable = false,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha, // needs pre-mult
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
          },
          vk::PipelineColorBlendAttachmentState{
            .blendEnable = false,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha, // needs pre-mult
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
          },
          vk::PipelineColorBlendAttachmentState{
            .blendEnable = true,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha, // needs pre-mult
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
          }
        };
        FramebufferType type = FramebufferType::eUnknown;
    };

    //
    inline static cpp21::obj<AttachmentLayout> defaultAttachmentLayout = std::make_shared<AttachmentLayout>();

    //
    struct PipelineLayoutCreateInfo : BaseCreateInfo {
        std::optional<QueueGetInfo> info = QueueGetInfo{};
    };

    //
    struct SemaphoreCreateInfo : BaseCreateInfo {
        bool signaled = false;
        bool hasExport = true;
    };

    //
    struct ImageSwapchainInfo : BaseCreateInfo {
        vk::SwapchainKHR swapchain = {};
        uint32_t index = 0u;
    };

    //
    struct ImageCreateInfo : BaseCreateInfo {
        vk::ImageCreateFlags flags = {};
        vk::ImageType imageType = vk::ImageType::e2D;
        vk::Format format = vk::Format::eUndefined;
        vk::Extent3D extent = { 2u,2u,1u };
        uint32_t mipLevelCount = 1u;
        uint32_t layerCount = 1u;

        vk::ImageLayout layout = vk::ImageLayout::eGeneral;
        std::optional<ImageSwapchainInfo> swapchain = {};
        std::optional<QueueGetInfo> info = QueueGetInfo{};
        ImageType type = ImageType::eStorage;
    };

    //
    struct SamplerCreateInfo : BaseCreateInfo {
        vk::PipelineLayout descriptors = {};
        std::optional<vk::SamplerCreateInfo> native = {};
    };

    //
    struct BufferCreateInfo : BaseCreateInfo {
        size_t size = 0ull;
        BufferType type = BufferType::eStorage;
        std::optional<QueueGetInfo> info = QueueGetInfo{};
    };



    //
    struct ResourceCreateInfo : BaseCreateInfo {
        vk::PipelineLayout descriptors = {};
        std::optional<vk::Buffer> buffer = {};
        std::optional<vk::Image> image = {};
        std::optional<BufferCreateInfo> bufferInfo = {};
        std::optional<ImageCreateInfo> imageInfo = {};

        // 
        ResourceCreateInfo use(cpp21::optional_ref<ExtensionName> extName = ExtensionName::eMemoryAllocator) {

            decltype(auto) copy = *this; if (!copy.extUsed) { copy.extUsed = std::make_shared<EXIP>(); }; if (copy.extUsed) { (*copy.extUsed)[ExtensionInfoName::eMemoryAllocator] = extName; }; return copy;
        };
    };

    // TODO: optional componentMapping
    struct ImageViewCreateInfo : BaseCreateInfo {
        vk::ImageViewType viewType = vk::ImageViewType::e2D;
        std::optional<vk::ImageSubresourceRange> subresourceRange = {};
        std::optional<vk::ComponentMapping> componentMapping = {};
        ImageViewPreference preference = ImageViewPreference::eSampled;
    };


    //
    struct GLObject {
        uint32_t handle = 0u;
        uint32_t dependency = 0u;
    };

    //
    struct ExtHandle {
#ifdef _WIN32
        HANDLE handle = 0ull;

        // 
        ExtHandle(HANDLE const& handle = 0ull) : handle(handle) {};
        operator HANDLE& () { return handle; };
        operator HANDLE const& () const { return handle; };
        decltype(auto) operator=(HANDLE const& hnd) { handle = hnd; return *this; };
#else
#ifdef __linux__ 
        uint32_t handle = 0u;
        uint32_t gap = 0u;

        // 
        ExtHandle(uint32_t const& handle = 0ull) : handle(handle) {};
        operator uint32_t& () { return handle; };
        operator uint32_t const& () const { return handle; };
        decltype(auto) operator=(uint32_t const& hnd) { handle = hnd; return *this; };
#endif
#endif
    };

    //
#pragma pack(4)
    __declspec(align(4))
        struct AllocatedMemory {
        vk::DeviceMemory memory = {};
        uintptr_t offset = 0ull;
        size_t size = 0ull;
        void* mapped = nullptr;
        uintptr_t allocation = 0ull;
        ExtHandle extHandle = {};

        //
        cpp21::obj<std::function<DFun>> destructor = {};
    };

    //
#pragma pack(4)
    __declspec(align(4))
        struct DataRegion {
        uintptr_t offset = 0ull;
        size_t stride = 0u;
        size_t size = VK_WHOLE_SIZE;
    };

    //
#pragma pack(4)
    __declspec(align(4))
        struct ImageDataRegion {
        vk::Offset3D offset = { 0u, 0u, 0u };
        vk::Extent3D extent = { 1u, 1u, 1u };
        uint32_t baseMipLevel = 0u;
        uint32_t baseLayer = 0u;
        uint32_t layerCount = 1u;
    };

    //
#pragma pack(4)
    __declspec(align(4))
        struct BufferRegion {
        vk::Buffer buffer = {};
        DataRegion region = {};
        uint32_t queueFamilyIndex = 0u;
    };

    //
#pragma pack(4)
    __declspec(align(4))
        struct ImageRegion {
        vk::Image image = {};
        ImageDataRegion region = {};
        uint32_t queueFamilyIndex = 0u;
    };

    //
    struct ImageLayoutSwitchWriteInfo : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};
        vk::ImageLayout newImageLayout = vk::ImageLayout::eGeneral;
        uint32_t queueFamilyIndex = 0u;

        // 
        std::optional<vk::ImageLayout> oldImageLayout = {};
        std::optional<vk::ImageSubresourceRange> subresourceRange = {};

        //
        decltype(auto) with(vk::CommandBuffer const& cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
    };

    //
    struct ImageClearWriteInfo : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};
        glm::vec4 clearColor = glm::vec4(0.f);
        uint32_t queueFamilyIndex = 0u;

        //
        std::optional<vk::ImageSubresourceRange> subresourceRange = {};

        //
        decltype(auto) with(vk::CommandBuffer const& cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
    };

    //
    struct SubmissionInfo : BaseCreateInfo {
        std::optional<QueueGetInfo> info = QueueGetInfo{};
        std::optional<vk::CommandBufferInheritanceInfo> inheritanceInfo = {};
        std::vector<std::function<void(vk::Result)>> onDone = {};
        cpp21::shared_vector<vk::SemaphoreSubmitInfo> waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
        cpp21::shared_vector<vk::SemaphoreSubmitInfo> signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
    };

    //
    struct ImageLayoutSwitchInfo : BaseCreateInfo {
        std::optional<QueueGetInfo> info = QueueGetInfo{};
        std::optional<ImageLayoutSwitchWriteInfo> switchInfo = {};

        // 
        SubmissionInfo submission = {};
    };

    //
#ifdef _WIN32
    inline extern vk::ExternalMemoryHandleTypeFlags extMemFlags = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
    inline extern vk::ExternalSemaphoreHandleTypeFlags extSemFlags = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
    inline extern vk::ExternalMemoryHandleTypeFlagBits extMemFlagBits = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
    inline extern vk::ExternalSemaphoreHandleTypeFlagBits extSemFlagBits = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueWin32;
#else
#ifdef __linux__ 
    inline extern vk::ExternalMemoryHandleTypeFlags extMemFlags = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd;
    inline extern vk::ExternalSemaphoreHandleTypeFlags extSemFlags = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd;
    inline extern vk::ExternalMemoryHandleTypeFlagBits extMemFlagBits = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueFd;
    inline extern vk::ExternalSemaphoreHandleTypeFlagBits extSemFlagBits = vk::ExternalSemaphoreHandleTypeFlagBits::eOpaqueFd;
#endif
#endif

    //
    struct UniformDataWriteSet : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};
        std::optional<QueueGetInfo> info = QueueGetInfo{};
        std::optional<DataRegion> region = DataRegion{};
        cpp21::data_view<char8_t> data = {};

        //
        decltype(auto) with(vk::CommandBuffer const& cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
    };

    //
    struct CacheDataWriteSet : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};
        std::optional<QueueGetInfo> info = QueueGetInfo{};
        std::optional<DataRegion> region = DataRegion{};
        cpp21::data_view<char8_t> data = {};
        uintptr_t page = 0ull;

        //
        decltype(auto) with(vk::CommandBuffer const& cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
    };

    // 
    struct UniformDataSet : BaseCreateInfo {
        std::optional<UniformDataWriteSet> writeInfo = {};
        std::optional<QueueGetInfo> info = QueueGetInfo{};
        SubmissionInfo submission = {};
    };

    // 
    struct CacheDataSet : BaseCreateInfo {
        std::optional<CacheDataWriteSet> writeInfo = {};
        std::optional<QueueGetInfo> info = QueueGetInfo{};
        SubmissionInfo submission = {};
    };



    //
    struct InstanceDraw {
        std::optional<InstanceDrawInfo> drawConst = {};
        cpp21::shared_vector<vk::MultiDrawInfoEXT> drawInfos = std::vector<vk::MultiDrawInfoEXT>{};
    };



    //
    struct ImageViewIndex {
        vk::ImageView imageView = {};
        uint32_t indice = 0;

        //
        operator uint32_t& () { return indice; };
        operator uint32_t const& () const { return indice; };
        ImageViewIndex& operator= (uint32_t const& indice_) { indice = indice_; return *this; };

        //
        operator vk::ImageView& () { return imageView; };
        operator vk::ImageView const& () const { return imageView; };
        ImageViewIndex& operator= (vk::ImageView const& imageView_) { imageView = imageView_; return *this; };
    };



    //
    struct WriteComputeInfo : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};
        vk::Extent3D dispatch = { 1u, 1u, 1u };
        vk::PipelineLayout layout = {};
        uintptr_t pipelineIndex = 0ull;

        //
        std::optional<InstanceAddressBlock> instanceAddressBlock = {};

        //
        decltype(auto) with(vk::CommandBuffer const& cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
    };

    //
    struct WriteGraphicsInfo : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};
        vk::PipelineLayout layout = {};
        uintptr_t pipelineIndex = 0ull;
        uintptr_t framebuffer = 0ull;

        // needs multiple-levels support
        std::vector<InstanceDraw> instanceDraws = {}; // currently, problematic for dynamic rendering... 

        // 
        std::optional<InstanceAddressBlock> instanceAddressBlock = {};

        //
        decltype(auto) with(vk::CommandBuffer const& cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
    };


    //
    struct ExecutePipelineInfo : BaseCreateInfo {
        std::optional<WriteGraphicsInfo> graphics = {};
        std::optional<WriteComputeInfo> compute = {};
        std::optional<QueueGetInfo> info = QueueGetInfo{};
        SubmissionInfo submission = {};
    };


    //
    struct CommandOnceSubmission : BaseCreateInfo {
        std::vector<std::function<vk::CommandBuffer(vk::CommandBuffer const&)>> commandInits = {};
        SubmissionInfo submission = {};
    };


    //
    struct CopyBufferWriteInfo : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};

        // 
        std::optional<BufferRegion> src = BufferRegion{};
        std::optional<BufferRegion> dst = BufferRegion{};

        //
        decltype(auto) with(vk::CommandBuffer const& cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
    };


    //
    struct DenoiseCommandWriteInfo : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};
        std::vector<vk::Image> images = {};

        //
        decltype(auto) with(vk::CommandBuffer const& cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
    };

    //
    struct UploadCommandWriteInfo : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};
        vk::Buffer bunchBuffer = {};
        uintptr_t hostMapOffset = 0ull;

        // 
        std::optional<ImageRegion> dstImage = {};
        std::optional<BufferRegion> dstBuffer = {};

        //
        decltype(auto) with(vk::CommandBuffer const& cmd, cpp21::optional_ref<vk::Buffer> buffer = {}) const { auto copy = *this; copy.cmdBuf = cmd; if (buffer && *buffer) { copy.bunchBuffer = buffer; }; return copy; };
        decltype(auto) mapOffset(uintptr_t const& offset) const { auto copy = *this; copy.hostMapOffset = offset; return copy; };
    };

    //
    struct DownloadCommandWriteInfo : BaseCreateInfo {
        vk::CommandBuffer cmdBuf = {};
        vk::Buffer bunchBuffer = {};
        uintptr_t hostMapOffset = 0ull;

        // 
        std::optional<BufferRegion> srcBuffer = {};

        //
        decltype(auto) with(vk::CommandBuffer const& cmd, cpp21::optional_ref<vk::Buffer> buffer = {}) const { auto copy = *this; copy.cmdBuf = cmd; if (buffer && *buffer) { copy.bunchBuffer = buffer; }; return copy; };
        decltype(auto) mapOffset(uintptr_t const& offset) const { auto copy = *this; copy.hostMapOffset = offset; return copy; };
    };


    //
    struct UploadExecutionOnce : BaseCreateInfo {
        cpp21::data_view<char8_t> host = {};
        UploadCommandWriteInfo writeInfo = {};
        SubmissionInfo submission = {};
    };

    //
    struct DownloadExecutionOnce : BaseCreateInfo {
        cpp21::data_view<char8_t> host = {};
        DownloadCommandWriteInfo writeInfo = {};
        SubmissionInfo submission = {};
    };

    //
    struct CopyBuffersExecutionOnce : BaseCreateInfo {
        CopyBufferWriteInfo writeInfo = {};
        SubmissionInfo submission = {};
    };



    //
    struct UploaderCreateInfo : BaseCreateInfo {
        std::optional<QueueGetInfo> info = QueueGetInfo{};
        size_t cacheSize = 8192ull * 8192ull * 4ull;

        // 
        UploaderCreateInfo use(cpp21::optional_ref<ExtensionName> extName = ExtensionName::eMemoryAllocator) {

            decltype(auto) copy = *this; if (!copy.extUsed) { copy.extUsed = std::make_shared<EXIP>(); }; if (copy.extUsed) { (*copy.extUsed)[ExtensionInfoName::eMemoryAllocator] = extName; }; return copy;
        };
    };

    //
    struct GraphicsDynamicState {
        bool hasDepthTest = true;
        bool hasDepthWrite = true;
        bool reversalDepth = false;
    };

    //
    struct GraphicsPipelineCreateInfo : BaseCreateInfo {
        std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> stageCodes = {};
        cpp21::obj<AttachmentLayout> attachmentLayout = defaultAttachmentLayout;


        bool hasConservativeRaster = false;
        cpp21::obj<GraphicsDynamicState> dynamicState = {};
    };

    //
    struct ComputePipelineCreateInfo : BaseCreateInfo {
        cpp21::shared_vector<uint32_t> code = std::vector<uint32_t>{};
    };

    //
    struct PipelineCreateInfo : BaseCreateInfo {
        vk::PipelineLayout layout = {};
        std::optional<ComputePipelineCreateInfo> compute = {};
        std::optional<GraphicsPipelineCreateInfo> graphics = {};
    };

    //
    struct DenoiserCreateInfo : BaseCreateInfo {
        //vk::PipelineLayout layout = {};
        //QueueGetInfo info = {};
        std::vector<uint32_t> queueFamilyIndices = {};
        vk::Extent2D extent = { 640u, 480u };
    };

    //
    struct FramebufferCreateInfo : BaseCreateInfo {
        vk::PipelineLayout layout = {};
        vk::Extent2D extent = { 640u, 480u };
        uint32_t minImageCount = 2u;
        std::optional<QueueGetInfo> info = {};
        cpp21::obj<AttachmentLayout> attachmentLayout = defaultAttachmentLayout;
    };

    //
    struct SwapchainCreateInfo : BaseCreateInfo {
        vk::PipelineLayout layout = {};
        vk::SurfaceKHR surface = {};
        std::optional<QueueGetInfo> info = {};
    };

    //
    struct PingPongCreateInfo : BaseCreateInfo {
        vk::PipelineLayout layout = {};
        vk::Extent2D extent = { 1u, 1u };
        uint32_t minImageCount = 1u;
        std::vector<uint32_t> split = {};
        std::vector<vk::Format> formats = {};
        std::optional<QueueGetInfo> info = {};
    };



    //
    class FenceStatus {
    protected:
        std::function<bool()> getStatus = {};
        std::function<void()> onDone = {};

        //
    public:
        bool checkStatus() {
            if (this->getStatus()) { if (this->onDone) { this->onDone(); }; this->onDone = {}; return true; };
            return false;
        };

        // 
        FenceStatus(std::function<bool()> getStatus, std::function<void()> onDone = {}) : getStatus(getStatus), onDone(onDone) {};
    };

    //
    using FenceTypeRaw = std::shared_ptr<FenceStatus>;
    using FenceType = FenceTypeRaw;
    //using FenceType = std::shared_ptr<FenceTypeRaw>;

    // 
    struct ComputeStageCreateInfo : BaseCreateInfo {
        //ComputeStageCreateInfo(vk::PipelineShaderStageCreateInfo spi = {}, vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT sgmp = {}) : spi(spi), sgmp(sgmp) {
        //};

        vk::PipelineShaderStageCreateInfo spi = {};
        vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT sgmp = {};

        // 
        operator vk::PipelineShaderStageCreateInfo& () { return spi; };
        operator vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& () { return sgmp; };
        operator vk::PipelineShaderStageCreateInfo const& () const { return spi; };
        operator vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT const& () const { return sgmp; };

        //
        decltype(auto) operator =(cpp21::optional_ref<vk::PipelineShaderStageCreateInfo> spi) { (this->spi = spi); return *this; };
        decltype(auto) operator =(cpp21::optional_ref<vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT> sgmp) { (this->sgmp = sgmp); return *this; };
    };

    // 
    struct ShaderModuleCreateInfo : BaseCreateInfo {
        //ComputeStageCreateInfo(vk::PipelineShaderStageCreateInfo spi = {}, vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT sgmp = {}) : spi(spi), sgmp(sgmp) {
        //};

        vk::ShaderModuleCreateInfo info = {};
        vk::ShaderModuleValidationCacheCreateInfoEXT validInfo = {};

        // 
        operator vk::ShaderModuleCreateInfo& () { return info; };
        operator vk::ShaderModuleValidationCacheCreateInfoEXT& () { return validInfo; };
        operator vk::ShaderModuleCreateInfo const& () const { return info; };
        operator vk::ShaderModuleValidationCacheCreateInfoEXT const& () const { return validInfo; };

        //
        decltype(auto) operator =(cpp21::optional_ref<vk::ShaderModuleCreateInfo> info) { (this->info = info); return *this; };
        decltype(auto) operator =(cpp21::optional_ref<vk::ShaderModuleValidationCacheCreateInfoEXT> validInfo) { (this->validInfo = validInfo); return *this; };
    };

    // 
    inline static std::vector<uint32_t> eTempCode = {};

    // 
    static inline decltype(auto) makeShaderModuleInfo(cpp21::optional_ref<std::vector<uint32_t>> code) {
        return ShaderModuleCreateInfo{ .info = vk::ShaderModuleCreateInfo{.codeSize = code->size() * 4ull, .pCode = code->data() } };
    };

    // 
    static inline decltype(auto) createShaderModule(cpp21::optional_ref<vk::Device> device, cpp21::optional_ref<ShaderModuleCreateInfo> info = {}) {
        return device->createShaderModule(info->info);
    };

    // 
    static inline decltype(auto) createShaderModule(cpp21::optional_ref<vk::Device> device, cpp21::optional_ref<std::vector<uint32_t>> code = {}) {
        return createShaderModule(device, makeShaderModuleInfo(eTempCode = code));
    };

    // create shader module 
    static inline decltype(auto) makePipelineStageInfo(cpp21::optional_ref<vk::Device> device, cpp21::optional_ref<std::vector<uint32_t>> code = {}, vk::ShaderStageFlagBits stage = vk::ShaderStageFlagBits::eCompute, cpp21::optional_ref<char const*> entry = "main") {
        vk::PipelineShaderStageCreateInfo spi = {
          .stage = stage,
          .pName = entry.value(),
          .pSpecializationInfo = nullptr
        };
        if (code->size() > 0u && (!spi.module)) { spi.module = createShaderModule(device, code); };
        return spi;
    };

    // create compute
    static inline decltype(auto) makeComputePipelineStageInfo(cpp21::optional_ref<vk::Device> device, cpp21::optional_ref<std::vector<uint32_t>> code = {}, cpp21::optional_ref<const char*> entry = "main", uint32_t const& subgroupSize = 0u) {
        decltype(auto) f = ComputeStageCreateInfo{};
        f.spi = makePipelineStageInfo(device, code, vk::ShaderStageFlagBits::eCompute, entry);
        f.spi.flags = vk::PipelineShaderStageCreateFlags{ vk::PipelineShaderStageCreateFlagBits::eRequireFullSubgroups };
        f.spi.module = createShaderModule(device, eTempCode = code);
        if (subgroupSize) {
            f.sgmp = vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT{};
            f.sgmp.pNext = nullptr;
            f.sgmp.requiredSubgroupSize = subgroupSize;
            f.spi.pNext = &f.sgmp;
        };
        return f;
    };

    //
    inline extern std::unordered_map<std::type_index, HandleType> handleTypeMap = {};

    //
    inline static decltype(auto) registerTypes() {
        ANAMED::handleTypeMap = {};

        //
        ANAMED::handleTypeMap[std::type_index(typeid(uintptr_t))] = HandleType::eUnknown;

        // 
        ANAMED::handleTypeMap[std::type_index(typeid(vk::Instance))] = HandleType::eInstance;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::PhysicalDevice))] = HandleType::ePhysicalDevice;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::Device))] = HandleType::eDevice;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::Queue))] = HandleType::eQueue;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::CommandBuffer))] = HandleType::eCommandBuffer;

        // 
        //ANAMED::handleTypeMap[std::type_index(typeid(vk::Framebuffer))] = HandleType::eFramebuffer;
        //ANAMED::handleTypeMap[std::type_index(typeid(vk::SwapchainKHR))] = HandleType::eSwapchain;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::SurfaceKHR))] = HandleType::eSurface;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::Buffer))] = HandleType::eBuffer;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::Image))] = HandleType::eImage;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::Pipeline))] = HandleType::ePipeline;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::PipelineLayout))] = HandleType::ePipelineLayout;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::AccelerationStructureKHR))] = HandleType::eAccelerationStructure;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::Semaphore))] = HandleType::eSemaphore;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::DeviceMemory))] = HandleType::eDeviceMemory;
        ANAMED::handleTypeMap[std::type_index(typeid(vk::Sampler))] = HandleType::eSampler;

        //
        return handleTypeMap;
    };

    //
    inline static decltype(auto) getHandleType(auto const& typed) {
        using T = std::decay_t<decltype(typed)>;
        decltype(auto) ID = std::type_index(typeid(T));
        if (handleTypeMap.find(ID) != handleTypeMap.end()) {
            return std::move(handleTypeMap.at(ID));
        };
        return std::forward<HandleType>(HandleType::eUnknown);
    };

    // 
    class Handle {
    public:
        friend Handle;

        // 
        uintptr_t value = 0ull;
        HandleType type = HandleType::eUnknown;
        uint32_t family = 0u;

    public:
        Handle() {};
        Handle(auto const& _handle, HandleType const& type, uint32_t const& family = 0u) : value(reinterpret_cast<uintptr_t const&>(_handle)), type(type), family(family) {};
        Handle(auto const& _handle, uint32_t const& family = 0u) : value(reinterpret_cast<uintptr_t const&>(_handle)), type(getHandleType(_handle)), family(family) {};
        Handle(Handle const& _handle) : value(_handle.value), type(_handle.type), family(_handle.family) {};

        // 
        template<class T = uintptr_t> inline decltype(auto) as() { return reinterpret_cast<T&>(this->value); };
        template<class T = uintptr_t> inline decltype(auto) as() const { return reinterpret_cast<T const&>(this->value); };

        // 
        inline decltype(auto) with(uint32_t const& family = 0u) const { return Handle(this->value, this->type, family); };

        // 
        //template<class T = uintptr_t> inline operator T& () { return this->as<T>(); };
        //template<class T = uintptr_t> inline operator T const& () const { return this->as<T>(); };

        //
        operator bool() const { return !!value; };

        //
        inline decltype(auto) operator =(auto const& handle) {
            this->value = reinterpret_cast<uintptr_t const&>(handle);
            this->type = getHandleType(handle);
            return *this;
        };

        //
        inline decltype(auto) operator =(Handle const& handle) {
            this->value = handle.value, this->type = handle.type, this->family = handle.family;
            return *this;
        };
    };

    //
    template<class T = BaseObj, class Tw = cpp21::obj<T>>
    class WrapShared : public Tw {
    public:
        using Tw::Tw;

        // 
        inline operator Handle& () { return this->ptr->getHandle(); };
        inline operator Handle const& () const { return this->ptr->getHandle(); };

        //
        inline operator bool() const { return !!this->ptr && this->ptr->isAlive(); };

        // 
        inline decltype(auto) getHandle() { return this->ptr->getHandle(); };
        inline decltype(auto) getHandle() const { return this->ptr->getHandle(); };

        // 
        inline decltype(auto) getBase() { return this->ptr->getBase(); };
        inline decltype(auto) getBase() const { return this->ptr->getBase(); };

        // 
        inline decltype(auto) handle() { return this->ptr->getHandle(); };
        inline decltype(auto) handle() const { return this->ptr->getHandle(); };

        // 
        inline decltype(auto) base() { return this->ptr->getBase(); };
        inline decltype(auto) base() const { return this->ptr->getBase(); };

        // 
        inline decltype(auto) type() { return this->ptr->getHandle().type; };
        inline decltype(auto) type() const { return this->ptr->getHandle().type; };

        // 
        inline decltype(auto) family() { return this->ptr->getHandle().family; };
        inline decltype(auto) family() const { return this->ptr->getHandle().family; };

        // 
        template<class T = uintptr_t> inline decltype(auto) as() { return this->ptr->getHandle().as<T>(); };
        template<class T = uintptr_t> inline decltype(auto) as() const { return this->ptr->getHandle().as<T>(); };

        //
        inline operator std::shared_ptr<T>& () { return this->ptr; };
        inline operator std::shared_ptr<T> const& () const { return this->ptr; };

        // 
        inline decltype(auto) with(uint32_t const& family = 0u) const { return this->ptr->getHandle().with(family); };

        // we forbid to change handle directly

        //

    };

    // 
    template<typename T>
    inline void atomic_max(std::atomic<T>& maximum_value, T const& value) noexcept
    {
        T prev_value = maximum_value;
        while (prev_value < value && !maximum_value.compare_exchange_weak(prev_value, value)) {}
    };

    //
    class CallStack : public std::enable_shared_from_this<CallStack> {
    protected:
        std::array<cpp21::obj<std::function<void()>>, 2048> callIds = {};
        std::atomic_int32_t callbackCount = 0;
        std::atomic_bool threadLocked = false;
        std::atomic_bool actionLocked = false;

    public:
        CallStack() {};
        ~CallStack() { this->process(); };

        // USE ONLY FOR OFF-THREADS...
        void add(cpp21::obj<std::function<void()>> fn = {}) {
            do { /* but nothing to do */ } while (this->threadLocked.load()); this->actionLocked = true;
            if (this->callbackCount < this->callIds.size()) {
                this->callIds[this->callbackCount++] = fn; //std::bind(fn, result)
            };
            this->actionLocked = false;
        };

        // 
        void add(std::function<void()> fn = {}) {
            return this->add(std::make_shared<std::function<void()>>(fn));
        };

        //
        void process() {
            if (!this->actionLocked.load()) {
                this->threadLocked = true;
                atomic_max(this->callbackCount, 0);
                while (this->callbackCount > 0) {
                    auto cid = --this->callbackCount;
                    if (cid >= 0) { // avoid errors...
                        auto callId = this->callIds[cid].exchange(std::shared_ptr<std::function<void()>>{}); if (callId && *callId) { (*callId)(); };
                    };
                };
                atomic_max(this->callbackCount, 0);
                this->threadLocked = false;
            };
        }
    };

    //
    using HMAP_C = cpp21::map_of_shared<uintptr_t, BaseObj>;
    using HMAP_T = std::map<HandleType, std::shared_ptr<HMAP_C>>;
    using HMAP_S = std::shared_ptr<HMAP_T>;
    using HMAP_I = std::optional<std::unordered_map<uintptr_t, std::shared_ptr<BaseObj>>::iterator>;



    //
    class BaseObj : public std::enable_shared_from_this<BaseObj> {
    protected:
        using SBP = std::shared_ptr<BaseObj>;

        // 
        std::shared_ptr<CallStack> callstack = {};

        //
        std::vector<std::shared_ptr<std::function<DFun>>> destructors = {};

        // 
        Handle handle = {}, base = {};
        //ExtHandle extHandle = {};
        GLObject glObject = {};
        HMAP_S handleObjectMap = {};

        // 
        std::shared_ptr<MSS> infoMap = {};
        //std::shared_ptr<EXM> extensions = {};

        //
        friend ContextObj;
        friend InstanceObj;
        friend DeviceObj;
        friend WrapShared<BaseObj>;
        friend cpp21::wrap_shared_ptr<BaseObj>;

        //
        bool alive = true;

    public: //

      //
        virtual std::shared_ptr<CallStack> getCallstack() { return callstack; };
        virtual std::shared_ptr<CallStack> getCallstack() const { return callstack; };

        // temp solution
        virtual bool isAlive() const { return alive; };

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

        //
        inline auto& getHandleMap() { return handleObjectMap; };
        inline auto const& getHandleMap() const { return handleObjectMap; };

        //
        virtual Handle& getHandle() { return this->handle; };
        virtual Handle const& getHandle() const { return this->handle; };
        virtual Handle& getBase() { return this->base; };
        virtual Handle const& getBase() const { return this->base; };

        //
        virtual HMAP_I destroy(Handle const& parent, HMAP_S parentMap = {});

        //
        virtual void tickProcessing() {
            if (this->callstack) {
                this->callstack->process();
            };
        };

        //
        ~BaseObj() {
            //this->tickProcessing();
            this->destroy(this->base);
        };

        // 
        BaseObj(Handle const& base = {}, Handle const& handle = {}) : base(base), handle(handle), infoMap(std::make_shared<MSS>()), callstack(std::make_shared<CallStack>()), handleObjectMap(std::make_shared<HMAP_T>()) {

        };


        //
        //virtual ExtHandle& getExtHandle() { return extHandle; };
        //virtual ExtHandle const& getExtHandle() const { return extHandle; };

        //
        template<class T = BaseObj>
        inline std::shared_ptr<T> emplace(Handle const& handle) {
            std::shared_ptr<T> sh_ptr = {};
            if (handleObjectMap->find(handle.type) != handleObjectMap->end()) {
                decltype(auto) objMap = handleObjectMap->at(handle.type);
                if (objMap && (*objMap)->find(handle.value) == (*objMap)->end()) {
                    sh_ptr = objMap->at(handle.value);
                    (*objMap)->erase(handle.value);
                };
            };
            return sh_ptr;
        };

        // 
        virtual std::type_info const& type_info() const {
            return typeid(std::decay_t<decltype(this)>);
        };

        //
        template<class T = BaseObj>
        inline void registerObj(Handle const& handle, std::shared_ptr<T> obj = {}) {
            if (handleObjectMap->find(handle.type) == handleObjectMap->end()) { (*handleObjectMap)[handle.type] = {}; };
            decltype(auto) map = handleObjectMap->at(handle.type);
            if (!map) { (*handleObjectMap)[handle.type] = (map = std::make_shared<HMAP_C>()); };
            map->set(handle.value, (obj ? obj : std::make_shared<T>(this->handle, handle)));
            //return shared_from_this();
        };

        //
        template<class T = BaseObj>
        inline void registerObj(std::shared_ptr<T> obj = {}) {
            return this->registerObj(obj->handle, obj);
        };




        //
        template<class T = BaseObj>
        inline void registerExt(cpp21::optional_ref<ExtensionName> extName, std::shared_ptr<T> obj = {}) {
            this->registerObj(Handle(uintptr_t(*extName), HandleType::eExtension), obj);
            //return shared_from_this();
        };

        //
        template<class T = BaseObj>
        inline void registerExt(ExtensionName const& extName, std::shared_ptr<T> obj = {}) {
            this->registerObj(Handle(uintptr_t(extName), HandleType::eExtension), obj);
            //return shared_from_this();
        };

        //
        template<class T = BaseObj>
        inline decltype(auto) get(Handle const& handle) {
            if (handleObjectMap->find(handle.type) == handleObjectMap->end()) {
                (*handleObjectMap)[handle.type] = std::make_shared<HMAP_C>();
            };

            //
            decltype(auto) objMap = handleObjectMap->at(handle.type);
            if ((*objMap)->find(handle.value) == (*objMap)->end()) {
                auto obj = std::make_shared<T>(this->handle, handle);
                objMap->set(handle.value, obj);
                return WrapShared<T>(obj);
            };

            //
            if ((*objMap)->find(handle.value) != (*objMap)->end()) {
                decltype(auto) obj = objMap->at(handle.value);
                if (obj && obj->alive && obj->handle.type == handle.type) {
                    return WrapShared<T>(std::dynamic_pointer_cast<T>(obj.shared()));
                };
            };

            //
            return WrapShared<T>();
        };

        //
        template<class T = BaseObj>
        inline decltype(auto) get(Handle const& handle) const {
            decltype(auto) objMap = handleObjectMap->at(handle.type);
            return WrapShared<T>(std::dynamic_pointer_cast<T>(objMap->at(handle.value).shared()));
        };

        //
        template<class T = BaseObj>
        inline decltype(auto) getExt(ExtensionName const& hValue) {
            return this->get<T>(Handle(uintptr_t(hValue), HandleType::eExtension));
        };

        //
        template<class T = BaseObj>
        inline decltype(auto) getExt(ExtensionName const& hValue) const {
            return this->get<T>(Handle(uintptr_t(hValue), HandleType::eExtension));
        };
    };



    //
    template<class T> auto handleResult(vk::ResultValue<T> const& rvalue) {
        rvalue.result;
        return rvalue.value;
    };

};

#endif
