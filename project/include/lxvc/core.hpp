#pragma once

// 
#include <vector>

// 
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vk-utils/cpp21.hpp>

// 
namespace lxvc {

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
    std::vector<std::string> instanceExtensionList = {};
  };

  // 
  struct DeviceCreateInfo {
    std::vector<std::string> deviceExtensionList = {};
    std::vector<uint32_t> physicalDeviceIndices = {0u};
    std::vector<uint32_t> queueFamilyIndices = {0u};
  };


};
