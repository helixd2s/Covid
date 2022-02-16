#pragma once

// 
#include "./core.hpp"

// 
namespace lxvc {

  //
  class ContextObj : std::enable_shared_from_this<ContextObj> {
  public:
    using tType = std::shared_ptr<ContextObj>;
    //using SFT = shared_from_this;

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    ContextObj(stm::uni_arg<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      this->construct(cInfo);
    };

    // 
    virtual tType construct(stm::uni_arg<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      return SFT();
    };
  };

  // 
  class InstanceObj : std::enable_shared_from_this<InstanceObj> {
  public:
    using tType = std::shared_ptr<InstanceObj>;
    friend DeviceObj;

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    //
    std::shared_ptr<ContextObj> contextObj = {};

    // 
    vk::Instance instance = {};
    vk::DispatchLoaderDynamic dispatch = {};
    stm::map_of_shared<vk::StructureType, vk::BaseInStructure> infoMap = {};

    //
    std::vector<std::string> extensionList = {};
    std::vector<std::string> layerList = {};
    std::vector<char const*> extensionNames = {};
    std::vector<char const*> layerNames = {};

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {};
    std::vector<vk::PhysicalDeviceGroupProperties> physicalDeviceGroups = {};

    // 
    InstanceObj(std::shared_ptr<ContextObj> contextObj = {}, stm::uni_arg<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) {
      this->construct(contextObj, cInfo);
    };

    //
    virtual std::vector<vk::PhysicalDeviceGroupProperties>& enumeratePhysicalDeviceGroups() {
      return (this->physicalDeviceGroups = (this->physicalDeviceGroups.size() > 0 ? this->physicalDeviceGroups : instance.enumeratePhysicalDeviceGroups()));
    };

    //
    virtual std::vector<vk::PhysicalDeviceGroupProperties> const& enumeratePhysicalDeviceGroups() const {
      return (this->physicalDeviceGroups.size() > 0 ? this->physicalDeviceGroups : instance.enumeratePhysicalDeviceGroups());
    };

    //
    virtual std::vector<vk::PhysicalDevice>& enumeratePhysicalDevices() {
      return (this->physicalDevices = (this->physicalDevices.size() > 0 ? this->physicalDevices : instance.enumeratePhysicalDevices()));
    };

    //
    virtual std::vector<vk::PhysicalDevice> const& enumeratePhysicalDevices() const {
      return (this->physicalDevices.size() > 0 ? this->physicalDevices : instance.enumeratePhysicalDevices());
    };

    //
    virtual std::vector<std::string>& filterExtensions(std::vector<std::string> const& names) {
      decltype(auto) props = vk::enumerateInstanceExtensionProperties();
      decltype(auto) selected = extensionList;

      // 
      uintptr_t nameIndex = 0ull;
      for (decltype(auto) name : names) {
        uintptr_t propIndex = 0ull;
        for (decltype(auto) prop : props) {
          std::string_view propName = { prop.extensionName };
          if (name.compare(propName) == 0) {
            selected.push_back(name); break;
          };
          propIndex++;
        };
        nameIndex++;
      };

      // 
      return selected;
      //return (extensionList = selected);
    };

    //
    virtual std::vector<std::string>& filterLayers(std::vector<std::string> const& names) {
      decltype(auto) props = vk::enumerateInstanceLayerProperties();
      decltype(auto) selected = layerList;

      // 
      uintptr_t nameIndex = 0ull;
      for (decltype(auto) name : names) {
        uintptr_t propIndex = 0ull;
        for (decltype(auto) prop : props) {
          std::string_view propName = { prop.layerName };
          if (name.compare(propName) == 0) {
            selected.push_back(name); break;
          };
          propIndex++;
        };
        nameIndex++;
      };

      // 
      return selected;
      //return (layerList = selected);
    };

    // 
    virtual tType construct(std::shared_ptr<ContextObj> contextObj = {}, stm::uni_arg<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) {
      this->contextObj = contextObj;
      this->infoMap = {};
      this->extensionList = cInfo->extensionList;
      this->layerList = cInfo->layerList;
      this->extensionNames = {};
      this->layerNames = {};

      //
      decltype(auto) instanceInfo = infoMap.set(vk::StructureType::eInstanceCreateInfo, vk::InstanceCreateInfo{ 
        .pApplicationInfo = infoMap.set(vk::StructureType::eApplicationInfo, vk::ApplicationInfo{
          .pApplicationName = cInfo->appName.c_str(),
          .applicationVersion = cInfo->appVersion,

          // TODO: updating this data before commit
          .pEngineName = "LXVC",
          .engineVersion = VK_MAKE_VERSION(1,0,0),

          // 
          .apiVersion = VK_VERSION_1_3
        })
      });
      instanceInfo->setPEnabledExtensionNames(stm::toCString(this->extensionNames, this->filterExtensions(this->extensionList)));
      instanceInfo->setPEnabledLayerNames(stm::toCString(this->layerNames, this->filterLayers(this->layerList)));

      //
      this->instance = vk::createInstance(instanceInfo);

      // 
      return SFT();
    };
  };

};
