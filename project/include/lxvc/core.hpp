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

    //template <typename... Args>
    //decltype(auto) g(Args&&... args) -> decltype(f(std::forward<Args>(args)...)) {
      //return f(std::forward<Args>(args)...);
    //};

    

    // 
#ifdef VKU_ENABLE_TYPE_SAFE
    namespace ts {
        using namespace type_safe;
    };
#endif

#ifdef TYPE_SAFE_OPTIONAL_REF_HPP_INCLUDED
    template<class T = cpp21::void_t> using optional_ref = ts::optional_ref<T>;
    template<class T = cpp21::void_t> using opt_ref = ts::opt_ref<T>;
    template<class T = cpp21::void_t> using opt_cref = ts::opt_cref<T>;
#else 
    template<class T = cpp21::void_t> using optional_ref = cpp21::optional_ref<T>;
    inline constexpr decltype(auto) opt_ref = cpp21::opt_ref;
    inline constexpr decltype(auto) opt_cref = cpp21::opt_cref;
#endif

  //
  class ContextObj;
  class InstanceObj;
  class DeviceObj;

  //
  struct ContextCreateInfo {

  };

  //
  struct MemoryRequirements {
    uint32_t physicalDeviceIndex = 0u;
    uint32_t requiredMemoryTypeBits = 0u;
    MemoryUsage usage = MemoryUsage::eGpuOnly;
  };

  // 
  struct InstanceCreateInfo {
    const std::string& appName = "LXVC_APP";
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
  enum class MemoryUsage : uint32_t {
    eUnknown = 0u,
    eGpuOnly = 1u,
    eCpuToGpu = 2u,
    eGpuToCpu = 3u,
    eCpuOnly = 4u
  };
};
