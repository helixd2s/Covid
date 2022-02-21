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
  class AccelerationObj;
  class ContextObj;
  class InstanceObj;
  class DeviceObj;
  class BufferObj;
  class ImageObj;
  class QueueObj;

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
  struct DeviceCreateInfo {
    cpp21::shared_vector<std::string> extensionList = {};
    cpp21::shared_vector<std::string> layerList = {};
    cpp21::shared_vector<uint32_t> queueFamilyIndices = {};
    uint32_t physicalDeviceGroupIndex = 0u;
    uint32_t physicalDeviceIndex = 0u;
  };

  //
  struct AccelerationCreateInfo {
    
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
  struct QueueCreateInfo {
    uint32_t queueFamilyIndex = 0u;
    uint32_t queueIndex = 0u;
  };

  //
  struct AllocatedMemory {
    vk::DeviceMemory memory = {};
    uintptr_t offset = 0ull;
    size_t size = 0ull;
  };

  //
  struct BufferRegion {
    vk::Buffer buffer = {};
    uintptr_t offset = 0ull;
    size_t size = VK_WHOLE_SIZE;
  };

  
};
