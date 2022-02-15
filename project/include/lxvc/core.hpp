#pragma once

//
#define VULKAN_HPP_NO_CONSTRUCTORS

// 
#include <vector>
#include <string>
#include <memory>
#include <string_view>

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
    std::vector<std::string> extensionNames = {};
    std::vector<std::string> layerNames = {};
  };

  // 
  struct DeviceCreateInfo {
    std::vector<std::string> extensionNames = {};
    std::vector<std::string> layerNames = {};
    std::vector<uint32_t> physicalDeviceIndices = {0u};
    std::vector<uint32_t> queueFamilyIndices = {0u};
  };


};
