#pragma once

//
#ifndef VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
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
#define VKU_ENABLE_INTERVAL
#include <vk-utils/cpp21.hpp>
#include <vk-utils/chain.hpp>

// 
#include <vector>
#include <string>
#include <memory>
#include <string_view>
#include <utility>

//
#include <glm/glm.hpp>

// but used by precompiled headers
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

// 
namespace lxvc {

  // 
#ifdef VKU_ENABLE_TYPE_SAFE
  namespace ts {
    using namespace type_safe;
  };
#endif

#ifdef TYPE_SAFE_OPTIONAL_REF_HPP_INCLUDED
  template<class T = cpp21::void_t> using optional_ref = ts::optional_ref<T>;
  CPP21_FN_ALIAS(opt_ref, ts::opt_ref);
  CPP21_FN_ALIAS(opt_cref, ts::opt_cref);
#else 
  template<class T = cpp21::void_t> using optional_ref = cpp21::optional_ref<T>;
  CPP21_FN_ALIAS(opt_ref, cpp21::opt_ref);
  CPP21_FN_ALIAS(opt_cref, cpp21::opt_cref);
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
    eUniversal = 5u
  };

  //
  enum class ImageType : uint32_t {
    eStorage = 0u,
    eTexture = 1u,
    eColorAttachment = 2u,
    eDepthAttachment = 3u,
    eStencilAttachment = 4u,
    eDepthStencilAttachment = 5u,
    eSwapchain = 6u
  };

  //
  enum class HandleType : uint32_t {
    eUnknown = 0u,
    eInstance = 1u,
    ePhysicalDevice = 2u,
    eDevice = 3u,
    eQueue = 4u,
    eCommandBuffer = 5u,

    eBuffer = 6u,
    eImage = 7u,
    ePipeline = 8u,
    eUploader = 9u,
    eDescriptors = 10u,
    eQueueFamily = 11u,
    eSurface = 12u,
    eSwapchain = 13u,
    eFramebuffer = 14u,
    eSemaphore = 15u,
    eAccelerationStructure = 16u
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
  class GeometryLevelObj;
  class InstanceLevelObj;
  class ContextObj;
  class InstanceObj;
  class DeviceObj;
  class ResourceObj;
  class QueueFamilyObj;
  class DescriptorsObj;
  class PipelineObj;
  class UploaderObj;
  class FramebufferObj;
  class SwapchainObj;
  class SemaphoreObj;
  

  //
  struct ContextCreateInfo {

  };

  //
  struct MemoryRequirements {
    //uint32_t physicalDeviceIndex = 0u;
    MemoryUsage memoryUsage = MemoryUsage::eGpuOnly;
    uint32_t memoryTypeBits = 0u;
    cpp21::bool32_t dedicated = true;
    size_t size = 0ull;
    size_t alignment = 0ull;
  };

  // 
  struct InstanceCreateInfo {
    std::string appName = "LXVC_APP";
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
      "VK_LAYER_KHRONOS_synchronization2",
      "VK_LAYER_KHRONOS_validation",
      "VK_LAYER_KHRONOS_profiles",
      "VK_LAYER_LUNARG_parameter_validation",
      "VK_LAYER_LUNARG_core_validation",
      "VK_LAYER_LUNARG_object_tracker",
      //"VK_LAYER_LUNARG_api_dump"
    };
  };

  //
  using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;

  //
  struct QueueFamilyCreateInfo {
    uint32_t queueFamilyIndex = 0u;
    cpp21::shared_vector<float> queuePriorities = std::vector<float>{ 1.f };
    //std::shared_ptr<MSS> infoMap = {};//std::make_shared<MSS>();
  };

  // 
  struct DeviceCreateInfo {
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
      "VK_EXT_depth_clip_control", 
      "VK_EXT_vertex_input_dynamic_state",
      "VK_EXT_conservative_rasterization", 
      "VK_EXT_blend_operation_advanced", 
      "VK_EXT_validation_cache",
      "VK_KHR_portability_subset"
    };
    cpp21::shared_vector<std::string> layerList = std::vector<std::string>{
    };
    cpp21::shared_vector<QueueFamilyCreateInfo> queueFamilyInfos = std::vector<QueueFamilyCreateInfo>{ QueueFamilyCreateInfo{} };
    uint32_t physicalDeviceGroupIndex = 0u;
    uint32_t physicalDeviceIndex = 0u;
  };

  //
  struct BufferViewInfo {
    uintptr_t deviceAddress = 0ull;
    uint32_t stride = 4ull;
    BufferViewFormat format = BufferViewFormat::eUint;
  };

  //
  struct GeometryInfo {
    BufferViewInfo texcoord = {};
    BufferViewInfo vertices = {};
    BufferViewInfo normals = {};
    BufferViewInfo indices = {};
    BufferViewInfo transform = {};

    //
    uint32_t materialId = 0u;
    uint32_t primitiveCount = 0u;
    uint32_t opaque = 1u;
    uint32_t reserved = 0u;
  };

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
  inline decltype(auto) cvtIndex(BufferViewFormat const& format) {
    switch (format) {
      case BufferViewFormat::eUint: return vk::IndexType::eUint32; break;
      case BufferViewFormat::eShort: return vk::IndexType::eUint16; break;
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
  struct GeometryLevelCreateInfo {
    std::vector<GeometryInfo> geometryData = {};
    std::vector<uint32_t> maxPrimitiveCounts = {};
    //size_t geometryCount = 1u;
    uintptr_t uploader = 0ull;

    // 
    std::optional<QueueGetInfo> info = QueueGetInfo{};
  };

  //
  using InstanceInfo = vk::AccelerationStructureInstanceKHR ;

  //
  struct InstanceLevelCreateInfo {
    std::vector<InstanceInfo> instanceData = {};
    std::vector<uint32_t> maxPrimitiveCounts = {};
    //size_t instanceCount = 1u;
    uintptr_t uploader = 0ull;

    // 
    std::optional<QueueGetInfo> info = QueueGetInfo{};
  };

  //
  struct AttachmentsInfo {
    vk::Format depthAttachmentFormat = vk::Format::eD32SfloatS8Uint;//eD32Sfloat;
    vk::Format stencilAttachmentFormat = vk::Format::eD32SfloatS8Uint;//eS8Uint;
    std::vector<vk::Format> colorAttachmentFormats = { vk::Format::eR32G32B32A32Sfloat, vk::Format::eR32G32B32A32Sfloat };
    std::vector<vk::PipelineColorBlendAttachmentState> blendStates = {
      vk::PipelineColorBlendAttachmentState{
        .blendEnable = false,
        .srcColorBlendFactor = vk::BlendFactor::eOneMinusDstAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eDstAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eOne,
        .alphaBlendOp = vk::BlendOp::eMax,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
      },
      vk::PipelineColorBlendAttachmentState{
        .blendEnable = false,
        .srcColorBlendFactor = vk::BlendFactor::eOneMinusDstAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eDstAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eOne,
        .alphaBlendOp = vk::BlendOp::eMax,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
      }
    };
  };

  //
  struct DescriptorsCreateInfo {
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    AttachmentsInfo attachments = {};
  };

  //
  struct SemaphoreCreateInfo {

  };

  //
  struct ImageSwapchainInfo {
    vk::SwapchainKHR swapchain = {};
    uint32_t index = 0u;
  };

  //
  struct ImageCreateInfo {
    vk::ImageType imageType = vk::ImageType::e2D;
    vk::Format format = vk::Format::eUndefined;
    vk::Extent3D extent = {2u,2u,2u};
    uint32_t mipLevelCount = 1u;
    uint32_t layerCount = 1u;

    vk::ImageLayout layout = vk::ImageLayout::eGeneral;
    std::optional<ImageSwapchainInfo> swapchain = {};
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    ImageType type = ImageType::eStorage;
  };

  //
  struct BufferCreateInfo {
    size_t size = 0ull;
    BufferType type = BufferType::eStorage;
  };

  //
  struct ResourceCreateInfo {
    vk::PipelineLayout descriptors = {};
    std::optional<vk::Buffer> buffer = {};
    std::optional<vk::Image> image = {};
    std::optional<BufferCreateInfo> bufferInfo = {};
    std::optional<ImageCreateInfo> imageInfo = {};
  };

  //
  struct ImageViewCreateInfo {
    vk::ImageViewType viewType = vk::ImageViewType::e2D;
    vk::ImageSubresourceRange subresourceRange = { vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u };
  };

  //
  struct AllocatedMemory {
    vk::DeviceMemory memory = {};
    uintptr_t offset = 0ull;
    size_t size = 0ull;
  };

  //
  struct DataRegion {
    uintptr_t offset = 0ull;
    size_t size = VK_WHOLE_SIZE;
  };

  //
  struct ImageDataRegion {
    vk::Offset3D offset = { 0u, 0u, 0u };
    vk::Extent3D extent = { 1u, 1u, 1u };
    uint32_t baseMipLevel = 0u;
  };

  //
  struct BufferRegion {
    vk::Buffer buffer = {};
    DataRegion region = {};
    uint32_t queueFamilyIndex = 0u;
  };

  //
  struct ImageRegion {
    vk::Image image = {};
    ImageDataRegion region = {};
    uint32_t queueFamilyIndex = 0u;
  };

  //
  struct ImageLayoutSwitchWriteInfo {
    vk::CommandBuffer cmdBuf = {};
    vk::ImageLayout newImageLayout = vk::ImageLayout::eGeneral;
    uint32_t queueFamilyIndex = 0u;

    // 
    std::optional<vk::ImageLayout> oldImageLayout = {};
    std::optional<vk::ImageSubresourceRange> subresourceRange = {};
    
    //
    decltype(auto) with(cpp21::const_wrap_arg<vk::CommandBuffer> cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
  };

  //
  struct SubmissionInfo {
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    std::optional<vk::CommandBufferInheritanceInfo> inheritanceInfo = {};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
  };

  //
  struct ImageLayoutSwitchInfo {
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    std::optional<ImageLayoutSwitchWriteInfo> switchInfo = {};

    // 
    SubmissionInfo submission = {};
  };

  

  //
  struct UniformDataWriteSet {
    vk::CommandBuffer cmdBuf = {};
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    std::optional<DataRegion> region = DataRegion{};
    cpp21::data_view<char8_t> data = {};

    //
    decltype(auto) with(cpp21::const_wrap_arg<vk::CommandBuffer> cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
  };

  // 
  struct UniformDataSet {
    std::optional<UniformDataWriteSet> writeInfo = {};
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    SubmissionInfo submission = {};
  };


  //
  struct WriteComputeInfo {
    vk::CommandBuffer cmdBuf = {};
    vk::Extent3D dispatch = { 1u, 1u, 1u };
    vk::PipelineLayout layout = {};
    
    //
    decltype(auto) with(cpp21::const_wrap_arg<vk::CommandBuffer> cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
  };

  //
  struct InstanceDrawData {
    glm::mat3x4 transform = glm::mat3x4(1.f);
    uint64_t reference = 0ull;
  };

  //
  struct InstanceAddressInfo {
    uint64_t data = 0ull;
    uint64_t accelStruct = 0ull;
  };

  //
  struct PushConstantData {
    InstanceAddressInfo instanceAddressInfo = {};
    uint32_t drawIndex = 0u;
  };

  //
  struct InstanceDrawInfo {
    //InstanceDrawData drawData = {};
    PushConstantData drawData = {};
    cpp21::shared_vector<vk::MultiDrawInfoEXT> drawInfos = std::vector<vk::MultiDrawInfoEXT>{};
  };
  

  //
  struct WriteGraphicsInfo {
    vk::CommandBuffer cmdBuf = {};
    vk::PipelineLayout layout = {};
    uintptr_t framebuffer = {};

    // needs multiple-levels support
    std::vector<InstanceDrawInfo> instanceInfos = {}; // currently, problematic for dynamic rendering... 

    //
    decltype(auto) with(cpp21::const_wrap_arg<vk::CommandBuffer> cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
  };


  //
  struct ExecutePipelineInfo {
    std::optional<WriteGraphicsInfo> graphics = {};
    std::optional<WriteComputeInfo> compute = {};
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    SubmissionInfo submission = {};
  };


  //
  struct CommandOnceSubmission {
    std::vector<std::function<cpp21::const_wrap_arg<vk::CommandBuffer>(cpp21::const_wrap_arg<vk::CommandBuffer>)>> commandInits = {};
    std::vector<std::function<void(cpp21::const_wrap_arg<vk::Result>)>> onDone = {};
    SubmissionInfo submission = {};
  };

  //
  //struct BufferRegionObj {
    //std::shared_ptr<ResourceObj> buffer = {};
    //DataRegion region = {};
  //};

  //
  struct CopyBufferWriteInfo {
    vk::CommandBuffer cmdBuf = {};

    // 
    std::optional<BufferRegion> src = BufferRegion{};
    std::optional<BufferRegion> dst = BufferRegion{};

    //
    decltype(auto) with(cpp21::const_wrap_arg<vk::CommandBuffer> cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
  };

  //
  struct UploadCommandWriteInfo {
    vk::CommandBuffer cmdBuf = {};
    uintptr_t hostMapOffset = 0ull;

    // 
    std::optional<ImageRegion> dstImage = {};
    std::optional<BufferRegion> dstBuffer = {};

    //
    decltype(auto) with(cpp21::const_wrap_arg<vk::CommandBuffer> cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
  };

  //
  struct DownloadCommandWriteInfo {
    vk::CommandBuffer cmdBuf = {};

    // 
    std::optional<BufferRegion> srcBuffer = {};

    //
    decltype(auto) with(cpp21::const_wrap_arg<vk::CommandBuffer> cmd) const { auto copy = *this; copy.cmdBuf = cmd; return copy; };
  };


  //
  struct UploadExecutionOnce {
    cpp21::data_view<char8_t> host = {};
    uintptr_t hostMapOffset = 0ull;
    UploadCommandWriteInfo writeInfo = {};
    SubmissionInfo submission = {};
  };

  //
  struct DownloadExecutionOnce {
    cpp21::data_view<char8_t> host = {};
    uintptr_t hostMapOffset = 0ull;
    DownloadCommandWriteInfo writeInfo = {};
    SubmissionInfo submission = {};
  };

  //
  struct CopyBuffersExecutionOnce {
    CopyBufferWriteInfo writeInfo = {};
    SubmissionInfo submission = {};
  };



  //
  struct UploaderCreateInfo {
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    size_t cacheSize = 1024ull * 1024ull * 2ull;
  };

  //
  struct GraphicsPipelineCreateInfo {
    std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> stageCodes = {};
  };

  //
  struct ComputePipelineCreateInfo {
    cpp21::shared_vector<uint32_t> code = std::vector<uint32_t>{};
  };

  //
  struct PipelineCreateInfo {
    vk::PipelineLayout layout = {};
    std::optional<ComputePipelineCreateInfo> compute = {};
    std::optional<GraphicsPipelineCreateInfo> graphics = {};
  };

  //
  struct FramebufferCreateInfo {
    vk::PipelineLayout layout = {};
    vk::Extent2D extent = {640u, 480u};
    std::optional<QueueGetInfo> info = {};
  };

  //
  struct SwapchainCreateInfo {
    vk::PipelineLayout layout = {};
    vk::SurfaceKHR surface = {};
    std::optional<QueueGetInfo> info = {};
  };

  //
  using FenceTypeRaw = std::tuple<std::future<vk::Result>, std::shared_ptr<vk::Fence>>;
  using FenceType = std::shared_ptr<FenceTypeRaw>;

  // 
  struct ComputeStageCreateInfo {
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
    decltype(auto) operator =(cpp21::const_wrap_arg<vk::PipelineShaderStageCreateInfo> spi) { (this->spi = spi); return *this; };
    decltype(auto) operator =(cpp21::const_wrap_arg<vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT> sgmp) { (this->sgmp = sgmp); return *this; };
  };

  // 
  struct ShaderModuleCreateInfo {
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
    decltype(auto) operator =(cpp21::const_wrap_arg<vk::ShaderModuleCreateInfo> info) { (this->info = info); return *this; };
    decltype(auto) operator =(cpp21::const_wrap_arg<vk::ShaderModuleValidationCacheCreateInfoEXT> validInfo) { (this->validInfo = validInfo); return *this; };
  };

  // 
  inline static std::vector<uint32_t> eTempCode = {};

  // 
  static inline decltype(auto) makeShaderModuleInfo(cpp21::const_wrap_arg<std::vector<uint32_t>> code) {
    return ShaderModuleCreateInfo{ vk::ShaderModuleCreateInfo{.codeSize = code->size() * 4ull, .pCode = code->data() } };
  };

  // 
  static inline decltype(auto) createShaderModule(cpp21::const_wrap_arg<vk::Device> device, cpp21::const_wrap_arg<ShaderModuleCreateInfo> info = {}) {
    return device->createShaderModule(info->info);
  };

  // 
  static inline decltype(auto) createShaderModule(cpp21::const_wrap_arg<vk::Device> device, cpp21::const_wrap_arg<std::vector<uint32_t>> code = {}) {
    return createShaderModule(device, makeShaderModuleInfo(eTempCode = code));
  };

  // create shader module 
  static inline decltype(auto) makePipelineStageInfo(cpp21::const_wrap_arg<vk::Device> device, cpp21::const_wrap_arg<std::vector<uint32_t>> code = {}, vk::ShaderStageFlagBits stage = vk::ShaderStageFlagBits::eCompute, cpp21::const_wrap_arg<char const*> entry = "main") {
    vk::PipelineShaderStageCreateInfo spi = {
      .stage = stage,
      .pName = entry.value(),
      .pSpecializationInfo = nullptr
    };
    if (code->size() > 0u && (!spi.module)) { spi.module = createShaderModule(device, code); };
    return spi;
  };

  // create compute
  static inline decltype(auto) makeComputePipelineStageInfo(cpp21::const_wrap_arg<vk::Device> device, cpp21::const_wrap_arg<std::vector<uint32_t>> code = {}, cpp21::const_wrap_arg<const char*> entry = "main", cpp21::const_wrap_arg<uint32_t> subgroupSize = 0u) {
    decltype(auto) f = ComputeStageCreateInfo{};
    f.spi = makePipelineStageInfo(device, code, vk::ShaderStageFlagBits::eCompute, entry);
    f.spi.flags = vk::PipelineShaderStageCreateFlags{ vk::PipelineShaderStageCreateFlagBits::eRequireFullSubgroups };
    f.spi.module = createShaderModule(device, eTempCode = code);
    if (subgroupSize && subgroupSize.value() > 0u) {
      f.sgmp = vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT{};
      f.sgmp.pNext = nullptr;
      f.sgmp.requiredSubgroupSize = subgroupSize.value();
      f.spi.pNext = &f.sgmp;
    };
    return f;
  };

  //
  inline extern std::unordered_map<std::type_index, HandleType> handleTypeMap = {};

  //
  inline static decltype(auto) registerTypes() {
    lxvc::handleTypeMap = {};

    //
    lxvc::handleTypeMap[std::type_index(typeid(uintptr_t))] = HandleType::eUnknown;

    // 
    lxvc::handleTypeMap[std::type_index(typeid(vk::Instance))] = HandleType::eInstance;
    lxvc::handleTypeMap[std::type_index(typeid(vk::PhysicalDevice))] = HandleType::ePhysicalDevice;
    lxvc::handleTypeMap[std::type_index(typeid(vk::Device))] = HandleType::eDevice;
    lxvc::handleTypeMap[std::type_index(typeid(vk::Queue))] = HandleType::eQueue;
    lxvc::handleTypeMap[std::type_index(typeid(vk::CommandBuffer))] = HandleType::eCommandBuffer;

    // 
    lxvc::handleTypeMap[std::type_index(typeid(vk::SwapchainKHR))] = HandleType::eSwapchain;
    lxvc::handleTypeMap[std::type_index(typeid(vk::SurfaceKHR))] = HandleType::eSurface;
    lxvc::handleTypeMap[std::type_index(typeid(vk::Buffer))] = HandleType::eBuffer;
    lxvc::handleTypeMap[std::type_index(typeid(vk::Image))] = HandleType::eImage;
    lxvc::handleTypeMap[std::type_index(typeid(vk::Pipeline))] = HandleType::ePipeline;
    lxvc::handleTypeMap[std::type_index(typeid(vk::PipelineLayout))] = HandleType::eDescriptors;
    lxvc::handleTypeMap[std::type_index(typeid(vk::AccelerationStructureKHR))] = HandleType::eAccelerationStructure;
    lxvc::handleTypeMap[std::type_index(typeid(vk::Framebuffer))] = HandleType::eFramebuffer;
    lxvc::handleTypeMap[std::type_index(typeid(vk::Semaphore))] = HandleType::eSemaphore;

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
    Handle(auto const& _handle, cpp21::const_wrap_arg<HandleType> type, cpp21::const_wrap_arg<uint32_t> family = 0u) : value(reinterpret_cast<uintptr_t const&>(_handle)), type(type), family(family) {};
    Handle(auto const& _handle, cpp21::const_wrap_arg<uint32_t> family = 0u) : value(reinterpret_cast<uintptr_t const&>(_handle)), type(getHandleType(_handle)), family(family) {};
    Handle(cpp21::const_wrap_arg<Handle> _handle) : value(_handle->value), type(_handle->type), family(_handle->family) {};

    // 
    template<class T = uintptr_t> inline decltype(auto) as() { return reinterpret_cast<T&>(this->value); };
    template<class T = uintptr_t> inline decltype(auto) as() const { return reinterpret_cast<T const&>(this->value); };

    // 
    inline decltype(auto) with(cpp21::const_wrap_arg<uint32_t> family = 0u) const { return Handle(this->value, this->type, family); };

    // 
    template<class T = uintptr_t> inline operator T& () { return this->as<T>(); };
    template<class T = uintptr_t> inline operator T const& () const { return this->as<T>(); };

    //
    operator bool() const { return !!value; };

    //
    inline decltype(auto) operator =(auto const& handle) {
      this->value = reinterpret_cast<uintptr_t const&>(handle);
      this->type = getHandleType(handle);
      return *this;
    };

    //
    inline decltype(auto) operator =(cpp21::const_wrap_arg<Handle> handle) {
      this->value = handle->value, this->type = handle->type, this->family = handle->family;
      return *this;
    };
  };

  //
  template<class T = BaseObj, class Tw = cpp21::wrap_shared_ptr<T>>
  class WrapShared : public Tw {
  public:
    using Tw::Tw;

    // 
    operator Handle& () { return this->ptr->handle; };
    operator Handle const& () const { return this->ptr->handle; };

    //
    operator cpp21::const_wrap_arg<Handle>() const { return this->ptr->handle; };

    // 
    inline decltype(auto) getHandle() { return this->ptr->handle; };
    inline decltype(auto) getHandle() const { return this->ptr->handle; };

    // 
    inline decltype(auto) getBase() { return this->ptr->handle.base; };
    inline decltype(auto) getBase() const { return this->ptr->handle.base; };

    // 
    inline decltype(auto) handle() { return this->ptr->handle; };
    inline decltype(auto) handle() const { return this->ptr->handle; };

    // 
    inline decltype(auto) base() { return this->ptr->handle.base; };
    inline decltype(auto) base() const { return this->ptr->handle.base; };

    // 
    inline decltype(auto) type() { return this->ptr->handle.type; };
    inline decltype(auto) type() const { return this->ptr->handle.type; };

    // 
    inline decltype(auto) family() { return this->ptr->handle.family; };
    inline decltype(auto) family() const { return this->ptr->handle.family; };

    // 
    template<class T = uintptr_t> inline decltype(auto) as() { return this->ptr->handle.as<T>(); };
    template<class T = uintptr_t> inline decltype(auto) as() const { return this->ptr->handle.as<T>(); };

    // 
    inline decltype(auto) with(cpp21::const_wrap_arg<uint32_t> family = 0u) const { return this->ptr->handle.with(family); };

    // we forbid to change handle directly

    //

  };

  //
  class BaseObj : public std::enable_shared_from_this<BaseObj> {
  protected:
    using SBP = std::shared_ptr<BaseObj>;
  public:
    friend ContextObj;
    friend InstanceObj;
    friend DeviceObj;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    // 
    Handle handle = {}, base = {};
    std::unordered_map<HandleType, cpp21::map_of_shared<uintptr_t, BaseObj>> handleObjectMap = {};
    std::shared_ptr<MSS> infoMap = {};

    // 
    BaseObj() : infoMap(std::make_shared<MSS>(MSS())) {};
    BaseObj(cpp21::const_wrap_arg<Handle> base, cpp21::const_wrap_arg<Handle> handle = {}) : base(base), handle(handle), infoMap(std::make_shared<MSS>()) {

    };

    //
    virtual Handle getHandle() const { return handle; };
    virtual Handle getBase() const { return base; };

    // 
    virtual std::type_info const& type_info() const {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    template<class T = BaseObj>
    inline std::shared_ptr<T> registerObj(cpp21::const_wrap_arg<Handle> handle, std::shared_ptr<T> obj = {}) {
      if (handleObjectMap.find(handle->type) == handleObjectMap.end()) { handleObjectMap[handle->type] = {}; };
      decltype(auto) map = handleObjectMap.at(handle->type);
      map.set(handle->value, (obj ? obj : std::make_shared<T>(this->handle, handle)));
      return shared_from_this();
    };

    //
    template<class T = BaseObj>
    inline std::shared_ptr<T> registerObj(auto const& handle, std::shared_ptr<T> obj = {}) {
      return this->registerObj(cpp21::const_wrap_arg<Handle>(Handle(handle)), obj);
    };

    //
    template<class T = BaseObj>
    inline std::shared_ptr<T> registerObj(std::shared_ptr<T> obj = {}) {
      return this->registerObj(obj->handle, obj);
    };

    //
    template<class T = BaseObj>
    inline decltype(auto) get(cpp21::const_wrap_arg<Handle> handle) {
      if (handleObjectMap.find(handle->type) == handleObjectMap.end()) {
        handleObjectMap[handle->type] = {};
      };

      // 
      decltype(auto) objMap = handleObjectMap.at(handle->type);
      if (objMap->find(handle->value) == objMap->end()) { 
        auto obj = std::make_shared<T>(this->handle, handle); 
        objMap.set(handle->value, obj);
        return WrapShared<T>(obj); 
      };
      return WrapShared<T>(std::dynamic_pointer_cast<T>(objMap.at(handle->value).shared()));
    };

    //
    template<class T = BaseObj>
    inline decltype(auto) get(cpp21::const_wrap_arg<Handle> handle) const {
      decltype(auto) objMap = handleObjectMap.at(handle->type);
      return WrapShared<T>(std::dynamic_pointer_cast<T>(objMap.at(handle->value).shared()));
    };

    //
    template<class T = BaseObj> inline WrapShared<T> get(auto const& handle) { return this->get<T>(cpp21::const_wrap_arg(Handle(handle))); };
    template<class T = BaseObj> inline WrapShared<T> get(auto const& handle) const { return this->get<T>(cpp21::const_wrap_arg(Handle(handle))); };
  };



  //


};
