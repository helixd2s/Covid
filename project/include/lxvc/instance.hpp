#pragma once

// 
#include "./Core.hpp"
#include "./Context.hpp"

// 
namespace lxvc {

  // 
  class InstanceObj : std::enable_shared_from_this<InstanceObj> {
  protected:
    using tType = std::shared_ptr<InstanceObj>;
    using cType = const char const*;
    friend DeviceObj;

  public:

    // 
    vk::Instance instance = {};
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<InstanceCreateInfo> cInfo = {};
    std::shared_ptr<MSS> infoMap = {};

    // 
    InstanceObj(std::shared_ptr<ContextObj> contextObj = {}, cpp21::optional_ref<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) : contextObj(contextObj), cInfo(cInfo) {
      this->construct(contextObj, cInfo);
    };

  protected: 
    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    //
    std::shared_ptr<ContextObj> contextObj = {};

    //
    std::vector<char const*> extensionNames = {};
    std::vector<char const*> layerNames = {};

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {};
    std::vector<vk::PhysicalDeviceGroupProperties> physicalDeviceGroups = {};

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
    virtual std::vector<cType>& filterExtensions(std::vector<std::string> const& names) {
      decltype(auto) props = vk::enumerateInstanceExtensionProperties();
      decltype(auto) selected = opt_ref(this->extensionNames);

      // 
      uintptr_t nameIndex = 0ull;
      for (decltype(auto) name : names) {
        uintptr_t propIndex = 0ull;
        for (decltype(auto) prop : props) {
          std::string_view propName = { prop.extensionName };
          if (name.compare(propName) == 0) {
            selected->push_back(name.c_str()); break;
          };
          propIndex++;
        };
        nameIndex++;
      };

      // 
      return *selected;
      //return (extensionList = selected);
    };

    //
    virtual std::vector<cType>& filterLayers(std::vector<std::string> const& names) {
      decltype(auto) props = vk::enumerateInstanceLayerProperties();
      decltype(auto) selected = opt_ref(this->layerNames);

      // 
      uintptr_t nameIndex = 0ull;
      for (decltype(auto) name : names) {
        uintptr_t propIndex = 0ull;
        for (decltype(auto) prop : props) {
          std::string_view propName = { prop.layerName };
          if (name.compare(propName) == 0) {
            selected->push_back(name.c_str()); break;
          };
          propIndex++;
        };
        nameIndex++;
      };

      // 
      return *selected;
      //return (layerList = selected);
    };

    // 
    virtual tType construct(std::shared_ptr<ContextObj> contextObj, cpp21::optional_ref<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) {
      this->contextObj = contextObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();
      this->extensionNames = {};
      this->layerNames = {};

      //
      decltype(auto) instanceInfo = infoMap->set(vk::StructureType::eInstanceCreateInfo, vk::InstanceCreateInfo{ 
        .pApplicationInfo = infoMap->set(vk::StructureType::eApplicationInfo, vk::ApplicationInfo{
          .pApplicationName = this->cInfo->appName.c_str(),
          .applicationVersion = this->cInfo->appVersion,

          // TODO: updating this data before commit
          .pEngineName = "LXVC",
          .engineVersion = VK_MAKE_VERSION(1,0,0),

          // 
          .apiVersion = VK_VERSION_1_3
        })
      });
      instanceInfo->setPEnabledExtensionNames(this->filterExtensions(this->cInfo->extensionList));
      instanceInfo->setPEnabledLayerNames(this->filterLayers(this->cInfo->layerList));

      //
      this->instance = vk::createInstance(instanceInfo);

      // 
      return SFT();
    };
  };

};
