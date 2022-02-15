#pragma once

// 
#include "./core.hpp"

// 
namespace lxvc {

  //
  class DeviceObj;

  // 
  struct InstanceCreateInfo {
    const std::string& appName = "LXVC_APP";
    uint32_t appVersion = VK_MAKE_VERSION(1,0,0);
    std::vector<std::string> instanceExtensionList = {};
  };

  // 
  class InstanceObj : std::enable_shared_from_this<InstanceObj> {
  public:
    using SI = std::shared_ptr<Instance>;
    using ST = shared_from_this;
    friend DeviceObj;

    // 
    vk::Instance instance = {};
    vk::InstanceDispatch dispatch = {};
    stm::map_of_shared<vk::StructureType, vk::BaseInStructure> infoMap = {};

    //
    std::vector<std::string> extensionList = {};
    std::string<std::string> layerList = {};

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {};

    // 
    InstanceObj(stm::uni_arg<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) {
      this->construct(cInfo);
    };

    //
    virtual std::vector<vk::PhysicalDevice>& enumeratePhysicalDevices() {
      return (this->physicalDevices = (this->physicalDevices ? this->physicalDevices : instance.enumeratePhysicalDevices()));
    };

    //
    virtual std::vector<vk::PhysicalDevice>& enumeratePhysicalDevices() const {
      return (this->physicalDevices ? this->physicalDevices : instance.enumeratePhysicalDevices());
    };

    //
    virtual std::vector<std::string>& filterExtensions(std::vector<std::string> const& names) {
      std::vector<vk::ExtensionProperties> props = vk::EnumerateInstanceExtensionProperties();
      std::vector<std::string>& selected = extensionList;

      // 
      uintptr_t nameIndex = 0ull;
      for (auto& name : names) {
        uintptr_t propIndex = 0ull;
        for (auto& prop : props) {
          std::string_view propName = { prop.extensionName };
          if (std::compare(propName, name) == 0) {
            selected.push_back(name); break;
          };
          propIndex+;
        };
        nameIndex++;
      };

      // 
      return selected;
      //return (extensionList = selected);
    };

    //
    virtual std::vector<std::string>& filterLayers(std::vector<std::string> const& names) {
      std::vector<vk::ExtensionProperties> props = vk::EnumerateInstanceLayerProperties();
      std::vector<std::string>& selected = layerList;

      // 
      uintptr_t nameIndex = 0ull;
      for (auto& name : names) {
        uintptr_t propIndex = 0ull;
        for (auto& prop : props) {
          std::string_view propName = { prop.layerName };
          if (std::compare(propName, name) == 0) {
            selected.push_back(name); break;
          };
          propIndex+;
        };
        nameIndex++;
      };

      // 
      return selected;
      //return (layerList = selected);
    };

    // 
    virtual SI construct(stm::uni_arg<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) {
      this->infoMap = {};
      this->extensionList = {};
      this->layerList = {};

      //
      auto appInfo = infoMap->set(vk::StructureType::eApplicationInfo, vk::ApplicationInfo{
        .pApplicationName = cInfo->appName.c_str(),
        .applicationVersion = cInfo->appVersion,

        // TODO: updating this data before commit
        .pEngineName = "LXVC",
        .engineVersion = VK_MAKE_VERSION(1,0,0),

        // 
        .apiVersion = VK_VERSION_1_3
      });

      //
      auto instanceInfo = infoMap->set(vk::StructureType::eInstanceCreateInfo, vk::InstanceCreateInfo{
        .pApplicationInfo = appInfo->get()
      });

      // 
      instanceInfo->setEnabledExtensionNames(this->filterExtensions());
      instanceInfo->setLayerExtensionNames(this->filterLayers());

      //
      this->instance = vk::createInstance(*instanceInfo);

      // 
      return ST();
    };
  };

};
