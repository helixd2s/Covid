#pragma once

// 
#include "./core.hpp"

// 
namespace lxvc {
  
  // 
  struct DeviceCreateInfo {
    std::shared_ptr<InstanceObj> instanceObj = {};
    std::vector<std::string> deviceExtensionList = {};
    std::vector<uint32_t> physicalDeviceIndices = {0u};
    std::vector<uint32_t> queueFamilyIndices = {0u};
  };

  // 
  class DeviceObj : std::enable_shared_from_this<DeviceObj> {
  public:
    using tType = std::shared_ptr<Device>;
    using SFT = shared_from_this;
    using MSS = stm::map_of_shared<vk::StructureType, vk::BaseInStructure>
    friend InstanceObj;

    //
    std::shared_ptr<InstanceObj> instanceObj = {};

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {}; 

    // 
    vk::Device instance = {};
    vk::DeviceDispatch dispatch = {};
    MSS infoMap = {};

    //
    std::vector<std::string> extensionList = {};
    std::string<std::string> layerList = {};

    //
    std::vector<MSS> queueInfoMaps = {};
    std::vector<vk::DeviceQueueCreateInfo> queueInfoCache = {};

    //
    std::vector<uint32_t> queueFamilyIndices = {};
    std::vector<vk::Queue> queues = {};

    // 
    DeviceObj(stm::uni_arg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      this->construct(cInfo);
    };

    //
    std::vector<vk::PhysicalDevice>& filterPhysicalDevices(std::vector<uint32_t> const& indices = {0u}) {
      decltype(auto) instancePhysicalDevices = this->instanceObj->enumeratePhysicalDevices();
      for (auto& indice : indices) {
        physicalDevices.push_back(instancePhysicalDevices[indices]);
      };
      return physicalDevices;
    };

    //
    std::vector<vk::DeviceQueueCreateInfo>& filterQueueInfo() {
      uintptr_t queueInfoIndex = 0ull;
      std::vector<vk::DeviceQueueCreateInfo>& queueInfos = queueInfoCache;
      for (auto& queueInfoMap : this->queueInfoMaps) {
        queueInfos.push_back(queueInfoMap->get<vk::DeviceQueueCreateInfo>(vk::eDeviceQueueCreateInfo));
        queueInfoIndex++;
      };
      return queueInfos;
    };

    //
    virtual std::vector<std::string>& filterExtensions(std::vector<std::string> const& names) {
      std::vector<vk::ExtensionProperties> props = vk::EnumerateDeviceExtensionProperties();
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
      std::vector<vk::ExtensionProperties> props = vk::EnumerateDeviceLayerProperties();
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
    virtual tType filterQueueFamilyIndices(std::vector<uint32_t> const& queueFamilyIndices = {}) {
      queueInfoMaps = {};
      for (auto& queueFamilyIndex : queueFamilyIndices) {
        auto last = queueInfoMaps.size();
        queueInfoMaps.push_back(SID{});
        auto& queueInfoMap = queueInfoMaps[last];

        // 
        std::vector<float> priorities = {1.f};
        queueInfoMap->set(vk::StructureType::eDeviceQueueCreateInfo, vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = queueFamilyIndex,
          .queueCount = (uint32_t)priorities.size(),
          .pQueuePriorities = priorities.data()
        });
      };
      return SFT();
    };

    // 
    virtual tType construct(stm::uni_arg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      this->instanceObj = cInfo->instanceObj;
      this->infoMap = {};
      this->extensionList = {};
      this->layerList = {};
      this->physicalDevices = {};
      this->filterQueueFamilyIndices(this->queueFamilyIndices = cInfo->queueFamilyIndices);

      // TODO: get rid from spagetti code or nesting
      auto deviceInfo = infoMap->set(vk::DeviceCreateInfo{
          .pNext = infoMap->set(vk::StructureType::ePhysicalDeviceFeatures2, vk::PhysicalDeviceFeatures2{
          .pNext = infoMap->set(vk::StructureType::ePhysicalDeviceVulkan11Features, vk::ePhysicalDeviceVulkan11Features{
          .pNext = infoMap->set(vk::StructureType::ePhysicalDeviceVulkan12Features, vk::ePhysicalDeviceVulkan12Features{
          .pNext = infoMap->set(vk::StructureType::ePhysicalDeviceVulkan13Features, vk::ePhysicalDeviceVulkan13Features{

          })
          })
          })
          })
      });

      //
      auto physicalDevice = this->filterPhysicalDevices(cInfo->physicalDeviceIndices)[0];

      //
      physicalDevice.getFeatures2(infoMap->get<vk::StructureType::ePhysicalDeviceFeatures2>(vk::StructureType::ePhysicalDeviceFeatures2));

      // 
      deviceInfo->setQueueCreateInfos(this->filterQueueInfo());
      deviceInfo->setEnabledExtensionNames(this->filterExtensions());
      deviceInfo->setLayerExtensionNames(this->filterLayers());

      //
      if (!!physicalDevice) {
        this->device = physicalDevice.createDevice(*deviceInfo);
      } else {
        std::cerr << "Physical Device Not Detected" << std::endl;
      };

      // 
      return SFT();
    };
  };


};
