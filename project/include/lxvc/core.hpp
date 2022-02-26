#pragma once

//
#define VULKAN_HPP_NO_CONSTRUCTORS

// 
#include <vector>
#include <string>
#include <memory>
#include <string_view>
#include <utility>

//
#include <glm/glm.hpp>

// 
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
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
    eStencilAttachment = 4u
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
    uint32_t appVersion = VK_MAKE_VERSION(1,0,0);
    cpp21::shared_vector<std::string> extensionList = {};
    cpp21::shared_vector<std::string> layerList = {};
  };

  //
  using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;

  //
  struct QueueFamilyCreateInfo {
    uint32_t queueFamilyIndex = 0u;
    cpp21::shared_vector<float> queuePriorities = std::vector<float>{1.f};
    //std::shared_ptr<MSS> infoMap = {};//std::make_shared<MSS>();
  };

  // 
  struct DeviceCreateInfo {
    cpp21::shared_vector<std::string> extensionList = {};
    cpp21::shared_vector<std::string> layerList = {};
    cpp21::shared_vector<QueueFamilyCreateInfo> queueFamilyInfos = std::vector<QueueFamilyCreateInfo>{ QueueFamilyCreateInfo{} };
    uint32_t physicalDeviceGroupIndex = 0u;
    uint32_t physicalDeviceIndex = 0u;
  };

  //
  struct AccelerationStructureCreateInfo {
    
  };

  //
  struct DescriptorsCreateInfo {

  };

  //
  struct ImageCreateInfo {
    ImageType type = ImageType::eStorage;
    vk::Extent3D extent = {};
    vk::Format format = vk::Format::eUndefined;
  };

  //
  struct BufferCreateInfo {
    BufferType type = BufferType::eDevice;
    size_t size = 0ull;
    
  };

  //
  struct ResourceCreateInfo {
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
  struct BufferRegion {
    vk::Buffer buffer = {};
    DataRegion region = {};
  };

  

  //
  struct QueueGetInfo {
    uint32_t queueFamilyIndex = 0u;
    uint32_t queueIndex = 0u;
  };

  //
  struct CommandSubmission {
    std::optional<QueueGetInfo> info = {};
    std::vector<std::function<vk::CommandBuffer const&(vk::CommandBuffer const&)>> commandInits = {};
    std::vector<std::function<void(vk::Result const&)>> onDone = {};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
    cpp21::shared_vector<vk::SemaphoreSubmitInfo> signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
  };

  //
  struct BufferRegionObj {
    std::shared_ptr<ResourceObj> buffer = {};
    DataRegion region = {};
  };

  //
  struct UploaderCreateInfo {
    std::optional<QueueGetInfo> info = {};
    size_t cacheSize = 1024ull * 1024ull * 2ull;
  };

  //
  struct GraphicsPipelineCreateInfo {
    
  };

  //
  struct ComputePipelineCreateInfo {
    cpp21::shared_vector<uint32_t> code = {};
  };

  //
  struct PipelineCreateInfo {
    std::shared_ptr<DescriptorsObj> descriptors = {};
    std::optional<ComputePipelineCreateInfo> compute = {};
    std::optional<GraphicsPipelineCreateInfo> graphics = {};
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
  static inline decltype(auto) createShaderModule(vk::Device const& device, cpp21::optional_ref<ShaderModuleCreateInfo> info = {}) {
      return device.createShaderModule(info.value());
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

};
