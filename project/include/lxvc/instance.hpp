#pragma once

// 
#include "./Core.hpp"
#include "./Context.hpp"

// 
namespace lxvc {

  // 
  class InstanceObj : public BaseObj {
  public:
    using tType = std::shared_ptr<InstanceObj>;
    using cType = const char const*;
    using BaseObj::BaseObj;

  protected:
    friend DeviceObj;

    // 
    //vk::Instance instance = {};
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<InstanceCreateInfo> cInfo = {};

  public: 
    // 
    InstanceObj(std::shared_ptr<ContextObj> contextObj = {}, cpp21::optional_ref<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) : cInfo(cInfo) {
      this->base = contextObj->handle;
      this->construct(contextObj, cInfo);
    };

    // 
    InstanceObj(Handle const& handle, std::optional<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) : cInfo(cInfo) {
      this->construct(lxvc::context, cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

  protected: 
    // 
    inline decltype(auto) SFT() { return std::dynamic_pointer_cast<std::decay_t<decltype(*this)>>(shared_from_this()); };
    inline decltype(auto) SFT() const { return std::dynamic_pointer_cast<const std::decay_t<decltype(*this)>>(shared_from_this()); };

    //
    //std::shared_ptr<ContextObj> contextObj = {};

    //
    std::vector<char const*> extensionNames = {};
    std::vector<char const*> layerNames = {};

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {};
    std::vector<vk::PhysicalDeviceGroupProperties> physicalDeviceGroups = {};

    //
    virtual std::vector<vk::PhysicalDeviceGroupProperties>& enumeratePhysicalDeviceGroups() {
      return (this->physicalDeviceGroups = (this->physicalDeviceGroups.size() > 0 ? this->physicalDeviceGroups : this->handle.as<vk::Instance>().enumeratePhysicalDeviceGroups()));
    };

    //
    virtual std::vector<vk::PhysicalDeviceGroupProperties> const& enumeratePhysicalDeviceGroups() const {
      return (this->physicalDeviceGroups.size() > 0 ? this->physicalDeviceGroups : this->handle.as<vk::Instance>().enumeratePhysicalDeviceGroups());
    };

    //
    virtual std::vector<vk::PhysicalDevice>& enumeratePhysicalDevices() {
      return (this->physicalDevices = (this->physicalDevices.size() > 0 ? this->physicalDevices : this->handle.as<vk::Instance>().enumeratePhysicalDevices()));
    };

    //
    virtual std::vector<vk::PhysicalDevice> const& enumeratePhysicalDevices() const {
      return (this->physicalDevices.size() > 0 ? this->physicalDevices : this->handle.as<vk::Instance>().enumeratePhysicalDevices());
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
      this->base = contextObj->handle;
      //this->deviceObj = deviceObj;
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
      lxvc::context->registerObj(this->handle = vk::createInstance(instanceInfo), shared_from_this());

      // 
      return SFT();
    };
  };

};
