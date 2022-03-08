#pragma once

// 
#include "./Core.hpp"
#include "./Context.hpp"

// 
namespace lxvc {

  // 
  inline VkBool32 callbackFn(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

  // 
  class InstanceObj : public BaseObj {
  public:
    using tType = WrapShared<InstanceObj>;
    using cType = const char const*;
    using BaseObj::BaseObj;

  protected:
    friend DeviceObj;

    // 
    //vk::Instance instance = {};
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<InstanceCreateInfo> cInfo = InstanceCreateInfo{};

    //
    //using dfnT = std::remove_pointer_t<std::decay_t<PFN_vkDebugUtilsMessengerCallbackEXT>>;
    vk::DebugUtilsMessengerEXT debugMessenger = {};
    //std::function<dfnT> debugCallback = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

  public: 
    // 
    InstanceObj(std::shared_ptr<ContextObj> contextObj = {}, cpp21::const_wrap_arg<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) : cInfo(cInfo) {
      this->base = contextObj->handle;
      this->construct(contextObj, cInfo);
    };

    // 
    InstanceObj(Handle const& handle, cpp21::const_wrap_arg<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) : cInfo(cInfo) {
      this->base = handle;
      this->construct(lxvc::context, cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual tType registerSelf() {
      lxvc::context->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(Handle const& handle, cpp21::const_wrap_arg<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) {
      auto shared = std::make_shared<InstanceObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  protected:
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
      if (this->physicalDeviceGroups.size() > 0) {
        //try {
          this->physicalDeviceGroups = this->handle.as<vk::Instance>().enumeratePhysicalDeviceGroups();
        //}
        //catch (std::exception e) {
          //std::cerr << "Failed to get physical device group, probably not supported..." << std::endl;
          //std::cerr << e.what() << std::endl;
        //};
      }
      return this->physicalDeviceGroups;
    };

    //
    virtual std::vector<vk::PhysicalDeviceGroupProperties> const& enumeratePhysicalDeviceGroups() const {
      //try {
        return (this->physicalDeviceGroups.size() > 0 ? this->physicalDeviceGroups : this->handle.as<vk::Instance>().enumeratePhysicalDeviceGroups());
      //}
      //catch (std::exception e) {
        //std::cerr << "Failed to get physical device group, probably not supported..." << std::endl;
        //std::cerr << e.what() << std::endl;
        //return this->physicalDeviceGroups;
      //};
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
      //std::vector<vk::ExtensionProperties> props(1024u); uint32_t size = 0ull;
      //vk::enumerateInstanceExtensionProperties("", &size, props.data()); props.resize(size);
      decltype(auto) props = vk::enumerateInstanceExtensionProperties(std::string(""));
      auto& selected = (this->extensionNames);

      // 
      uintptr_t nameIndex = 0ull;
      for (auto& name : names) {
        uintptr_t propIndex = 0ull;
        for (auto& prop : props) {
          std::string_view propName = { prop.extensionName };
          if (propName.find(name) != std::string::npos) {
            selected.push_back(name.c_str()); break;
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
    virtual std::vector<cType>& filterLayers(std::vector<std::string> const& names) {
      //std::vector<vk::LayerProperties> props(1024u); uint32_t size = 0ull;
      //vk::enumerateInstanceLayerProperties(&size, props.data()); props.resize(size);
      decltype(auto) props = vk::enumerateInstanceLayerProperties();
      auto& selected = (this->layerNames);

      // 
      uintptr_t nameIndex = 0ull;
      for (auto& name : names) {
        uintptr_t propIndex = 0ull;
        for (auto& prop : props) {
          std::string_view propName = { prop.layerName };
          if (propName.find(name) != std::string::npos) {
            selected.push_back(name.c_str()); break;
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
    virtual void construct(std::shared_ptr<ContextObj> contextObj, cpp21::const_wrap_arg<InstanceCreateInfo> cInfo = InstanceCreateInfo{}) {
      this->base = contextObj->handle;
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());
      this->extensionNames = {};
      this->layerNames = {};

      //
      decltype(auto) instanceInfo = infoMap->set(vk::StructureType::eInstanceCreateInfo, vk::InstanceCreateInfo{ 
        .pNext = nullptr,
        .pApplicationInfo = infoMap->set(vk::StructureType::eApplicationInfo, vk::ApplicationInfo{
          .pNext = nullptr,
          .pApplicationName = this->cInfo->appName.c_str(),
          .applicationVersion = this->cInfo->appVersion,

          // TODO: updating this data before commit
          .pEngineName = "LXVC",
          .engineVersion = VK_MAKE_VERSION(1,0,0),

          // 
          .apiVersion = VK_API_VERSION_1_3
        }).get()
      });
      instanceInfo->setPEnabledExtensionNames(this->filterExtensions(this->cInfo->extensionList));
      instanceInfo->setPEnabledLayerNames(this->filterLayers(this->cInfo->layerList));

      //
      this->handle = vk::createInstance(instanceInfo.ref());
      this->dispatch = vk::DispatchLoaderDynamic(this->handle.as<vk::Instance>(), vkGetInstanceProcAddr);
      //VULKAN_HPP_DEFAULT_DISPATCHER.init(this->handle.as<vk::Instance>());
      
      // 
      this->debugMessenger = this->handle.as<vk::Instance>().createDebugUtilsMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT{
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        .pfnUserCallback = &callbackFn,
        .pUserData = this
      }, nullptr, this->dispatch);

      // 
      //return SFT();
    };
  };

  // 
  inline VkBool32 callbackFn(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    decltype(auto) instanceObj = (InstanceObj*)(pUserData);
    switch (messageSeverity) {
      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        std::cerr << "Vulkan Debug Error: " << pCallbackData->pMessage << std::endl;
        return false;
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        std::cout << "Vulkan Debug Info: " << pCallbackData->pMessage << std::endl;
        break;

      case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        std::cerr << "Vulkan Debug Warning: " << pCallbackData->pMessage << std::endl;
        break;

      //case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        //std::cerr << "Vulkan Debug Verbose: " << pCallbackData->pMessage << std::endl;
        //break;
    };
    return true;
  };


};
