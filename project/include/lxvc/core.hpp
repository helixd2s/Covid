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
    using optional_ref = ts::optional_ref;
    using opt_ref = ts::opt_ref;
    using opt_cref = ts::opt_cref;
#else 
    using optional_ref = stm::optional_ref;
    using opt_ref = stm::opt_ref;
    using opt_cref = stm::opt_cref;
#endif

  //
  class ContextObj;
  class InstanceObj;
  class DeviceObj;

  //
  struct ContextCreateInfo {

  };

  // 
  struct InstanceCreateInfo {
    const std::string& appName = "LXVC_APP";
    uint32_t appVersion = VK_MAKE_VERSION(1,0,0);
    std::vector<std::string> extensionList = {};
    std::vector<std::string> layerList = {};
  };

  // 
  struct DeviceCreateInfo {
    std::vector<std::string> extensionList = {};
    std::vector<std::string> layerList = {};
    std::vector<uint32_t> queueFamilyIndices = {0u};
    uint32_t physicalDeviceGroupIndex = 0u;
    uint32_t physicalDeviceIndex = 0u;
  };

  //
  enum class MemoryUsage : uint32_t {
    eUnknown = 0u,
    eGpuOnly = 1u,
    eCpuToGpu = 2u,
    eGpuToCpu = 3u,
    eCpuOnly = 4u
  };
};
