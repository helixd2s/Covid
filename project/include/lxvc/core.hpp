#pragma once

//
#ifndef VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
#endif

// 
#include <vector>
#include <string>
#include <memory>
#include <string_view>
#include <utility>

//
#include <glm/glm.hpp>

// 
//#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
//#define VULKAN_HPP_DEFAULT_DISPATCHER vk::DispatchLoaderDynamic

// but used by precompiled headers
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

// 
#include <vk-utils/cpp21.hpp>
#include <vk-utils/chain.hpp>

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
    eDevice = 0u,
    eHostMap = 1u,
    eUniform = 2u
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
    eAccelerationStructure = 15u
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
  class AccelerationStructureObj;
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
    cpp21::shared_vector<std::string> extensionList = std::vector<std::string>{ "VK_KHR_surface", "VK_EXT_debug_utils", "VK_KHR_get_surface_capabilities2"
#ifdef _WIN32
      , "VK_KHR_win32_surface"
#endif
    };
    cpp21::shared_vector<std::string> layerList = std::vector<std::string>{ "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2"};
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
    cpp21::shared_vector<std::string> extensionList = std::vector<std::string>{ "VK_KHR_deferred_host_operations", "VK_KHR_acceleration_structure", "VK_KHR_ray_query", "VK_KHR_ray_tracing_pipeline", "VK_KHR_swapchain", "VK_EXT_multi_draw", "VK_KHR_separate_depth_stencil_layouts", "VK_KHR_dynamic_rendering"};
    cpp21::shared_vector<std::string> layerList = std::vector<std::string>{ "VK_LAYER_KHRONOS_validation" , "VK_LAYER_KHRONOS_synchronization2" };
    cpp21::shared_vector<QueueFamilyCreateInfo> queueFamilyInfos = std::vector<QueueFamilyCreateInfo>{ QueueFamilyCreateInfo{} };
    uint32_t physicalDeviceGroupIndex = 0u;
    uint32_t physicalDeviceIndex = 0u;
  };

  //
  struct AccelerationStructureCreateInfo {

  };

  //
  struct QueueGetInfo {
    uint32_t queueFamilyIndex = 0u;
    uint32_t queueIndex = 0u;
  };

  //
  struct DescriptorsCreateInfo {

  };

  //
  struct ImageSwapchainInfo {
    vk::SwapchainKHR swapchain = {};
    uint32_t index = 0u;
  };

  //
  struct ImageCreateInfo {
    std::optional<ImageSwapchainInfo> swapchain = {};
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    ImageType type = ImageType::eStorage;
    vk::ImageType imageType = vk::ImageType::e2D;
    vk::Extent3D extent = {};
    vk::Format format = vk::Format::eUndefined;
    vk::ImageLayout layout = vk::ImageLayout::eGeneral;
  };

  //
  struct BufferCreateInfo {
    BufferType type = BufferType::eDevice;
    size_t size = 0ull;

  };

  //
  struct ResourceCreateInfo {
    std::optional<vk::Buffer> buffer = {};
    std::optional<vk::Image> image = {};
    std::optional<BufferCreateInfo> bufferInfo = {};
    std::optional<ImageCreateInfo> imageInfo = {};
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
  struct ImageLayoutSwitchInfo {
    vk::ImageLayout const& newImageLayout = vk::ImageLayout::eGeneral;
    std::optional<vk::ImageLayout> oldImageLayout = {};
    std::optional<vk::ImageSubresourceRange> subresourceRange = {};
    std::optional<QueueGetInfo> info = {};
  };

  // 
  struct UniformDataSet {
    std::span<char8_t> data = {};
    std::optional<DataRegion> region = DataRegion{};
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
  };

  //
  struct ExecuteComputeInfo {
    vk::Extent3D dispatch = { 1u, 1u, 1u };
    vk::PipelineLayout layout = {};
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
  };

  //
  struct ExecuteGraphicsInfo {
    uintptr_t framebuffer = 0ull;
    std::vector<vk::MultiDrawInfoEXT> multiDrawInfo = {};
    vk::PipelineLayout layout = {};
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
  };

  //
  struct CommandOnceSubmission {
    std::optional<QueueGetInfo> info = QueueGetInfo{};
    std::vector<std::function<vk::CommandBuffer const& (vk::CommandBuffer const&)>> commandInits = {};
    std::vector<std::function<void(vk::Result const&)>> onDone = {};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
    std::optional<vk::CommandBufferInheritanceInfo> inheritanceInfo = {};
  };

  //
  //struct BufferRegionObj {
    //std::shared_ptr<ResourceObj> buffer = {};
    //DataRegion region = {};
  //};

  //
  struct CopyBufferInfo {
    //std::optional<QueueGetInfo> info = QueueGetInfo{};
    std::optional<BufferRegion> src = BufferRegion{};
    std::optional<BufferRegion> dst = BufferRegion{};
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
    vk::Extent2D extent = {640u, 480u};
    vk::PipelineLayout layout = {};
  };

  //
  struct SwapchainCreateInfo {
    vk::SurfaceKHR surface = {};
    vk::PipelineLayout layout = {};
  };

  //
  using FenceType = std::tuple<std::future<vk::Result>, vk::Fence>;

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
    decltype(auto) operator =(vk::PipelineShaderStageCreateInfo const& spi) { (this->spi = spi); return *this; };
    decltype(auto) operator =(vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT const& sgmp) { (this->sgmp = sgmp); return *this; };
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
    decltype(auto) operator =(vk::ShaderModuleCreateInfo const& info) { (this->info = info); return *this; };
    decltype(auto) operator =(vk::ShaderModuleValidationCacheCreateInfoEXT const& validInfo) { (this->validInfo = validInfo); return *this; };
  };

  // 
  inline static std::vector<uint32_t> eTempCode = {};

  // 
  static inline decltype(auto) makeShaderModuleInfo(std::vector<uint32_t> const& code) {
    return ShaderModuleCreateInfo{ vk::ShaderModuleCreateInfo{.codeSize = code.size() * 4ull, .pCode = code.data() } };
  };

  // 
  static inline decltype(auto) createShaderModule(vk::Device const& device, ShaderModuleCreateInfo const& info = {}) {
    return device.createShaderModule(info);
  };

  // 
  static inline decltype(auto) createShaderModule(vk::Device const& device, std::vector<uint32_t> const& code = {}) {
    return createShaderModule(device, makeShaderModuleInfo(eTempCode = code));
  };

  // create shader module 
  static inline decltype(auto) makePipelineStageInfo(vk::Device const& device, std::vector<uint32_t> const& code = {}, vk::ShaderStageFlagBits stage = vk::ShaderStageFlagBits::eCompute, std::optional<char const*> entry = "main") {
    vk::PipelineShaderStageCreateInfo spi = {
      .stage = stage,
      .pName = entry.value(),
      .pSpecializationInfo = nullptr
    };
    if (code.size() > 0u && (!spi.module)) { spi.module = createShaderModule(device, code); };
    return std::move(spi);
  };

  // create compute
  static inline decltype(auto) makeComputePipelineStageInfo(vk::Device const& device, std::vector<uint32_t> const& code = {}, std::optional<const char*> entry = "main", std::optional<uint32_t> subgroupSize = 0u) {
    decltype(auto) f = ComputeStageCreateInfo{};
    f.spi = makePipelineStageInfo(device, code, vk::ShaderStageFlagBits::eCompute, entry);
    f.spi.flags = vk::PipelineShaderStageCreateFlags{ vk::PipelineShaderStageCreateFlagBits::eRequireFullSubgroups };
    f.spi.module = createShaderModule(device, eTempCode = code);
    if (subgroupSize && subgroupSize.value() > 0u) {
      f.sgmp = vk::PipelineShaderStageRequiredSubgroupSizeCreateInfoEXT{};
      f.sgmp.requiredSubgroupSize = subgroupSize.value();
      f.spi.pNext = &f.sgmp;
    };
    return std::move(f);
  };

  //
  inline extern std::unordered_map<std::type_index, HandleType> handleTypeMap = {};

  //
  inline static decltype(auto) registerTypes() {
    lxvc::handleTypeMap = {};

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
    template<class T = uintptr_t> inline operator T& () { return this->as<T>(); };
    template<class T = uintptr_t> inline operator T const& () const { return this->as<T>(); };

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
  class BaseObj : public std::enable_shared_from_this<BaseObj> {
  protected:
    using SBP = std::shared_ptr<BaseObj>;
  public:
    friend ContextObj;
    friend InstanceObj;
    friend DeviceObj;

    // 
    inline decltype(auto) SFT() { return std::dynamic_pointer_cast<std::decay_t<decltype(*this)>>(shared_from_this()); };
    inline decltype(auto) SFT() const { return std::dynamic_pointer_cast<const std::decay_t<decltype(*this)>>(shared_from_this()); };

    // 
    Handle handle = {}, base = {};
    std::unordered_map<HandleType, cpp21::map_of_shared<uintptr_t, BaseObj>> handleObjectMap = {};
    std::shared_ptr<MSS> infoMap = {};

    // 
    BaseObj() : infoMap(std::make_shared<MSS>()) {};
    BaseObj(Handle const& base, Handle const& handle = {}) : base(base), handle(handle), infoMap(std::make_shared<MSS>()) {

    };

    // 
    virtual std::type_info const& type_info() const {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    template<class T = BaseObj>
    inline decltype(auto) registerObj(Handle const& handle, std::shared_ptr<T> const& obj = {}) {
      if (handleObjectMap.find(handle.type) == handleObjectMap.end()) { handleObjectMap[handle.type] = {}; };
      decltype(auto) map = handleObjectMap.at(handle.type);
      map.set(handle.value, (obj ? obj : std::make_shared<T>(this->handle, handle)));
      return shared_from_this();
    };

    //
    template<class T = BaseObj>
    inline decltype(auto) registerObj(auto const& handle, std::shared_ptr<T> const& obj = {}) {
      return this->registerObj(Handle(handle), obj);
    };

    //
    template<class T = BaseObj>
    inline decltype(auto) registerObj(std::shared_ptr<T> const& obj = {}) {
      return this->registerObj(obj->handle, obj);
    };

    //
    template<class T = BaseObj>
    inline decltype(auto) get(Handle const& handle) {
      if (handleObjectMap.find(handle.type) == handleObjectMap.end()) {
        handleObjectMap[handle.type] = {};
      };

      // 
      decltype(auto) objMap = handleObjectMap.at(handle.type);
      if (objMap->find(handle.value) == objMap->end()) { objMap.set(handle.value, std::make_shared<T>(this->handle, handle)); };
      return std::dynamic_pointer_cast<T>(objMap.at(handle.value).shared());
    };

    //
    template<class T = BaseObj>
    inline decltype(auto) get(Handle const& handle) const {
      decltype(auto) objMap = handleObjectMap.at(handle.type);
      return std::dynamic_pointer_cast<T>(objMap.at(handle.value).shared());
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
    inline decltype(auto) type() { return this->ptr->handle.type; };
    inline decltype(auto) type() const { return this->ptr->handle.type; };

    // 
    inline decltype(auto) family() { return this->ptr->handle.family; };
    inline decltype(auto) family() const { return this->ptr->handle.family; };

    // 
    template<class T = uintptr_t> inline decltype(auto) as() { return this->ptr->handle.as<T>(); };
    template<class T = uintptr_t> inline decltype(auto) as() const { return this->ptr->handle.as<T>(); };

    // 
    inline decltype(auto) with(uint32_t const& family = 0u) const { return this->ptr->handle.with(family); };

    // we forbid to change handle directly

    //

  };

  //


};
