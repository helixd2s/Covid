#pragma once

// 
#include "./core.hpp"
#include "./instance.hpp"

// 
namespace lxvc {

  // 
  class DeviceObj : std::enable_shared_from_this<DeviceObj> {
  public:
    using tType = std::shared_ptr<DeviceObj>;
    using MSS = stm::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    friend InstanceObj;

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    //
    std::shared_ptr<InstanceObj> instanceObj = {};

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {}; 
    std::vector<MSS> PDInfoMaps = {};

    // 
    vk::Device device = {};
    vk::DispatchLoaderDynamic dispatch = {};
    MSS infoMap = {};

    //
    std::vector<std::string> extensionList = {};
    std::vector<std::string> layerList = {};
    std::vector<char const*> extensionNames = {};
    std::vector<char const*> layerNames = {};

    //
    std::vector<uint32_t> queueFamilyIndices = {};
    std::vector<vk::DeviceQueueCreateInfo> queueInfoCache = {};
    std::vector<std::vector<float>> queuePriorities = {};
    std::vector<MSS> queueInfoMaps = {};

    // 
    uint32_t physicalDeviceGroupIndex = 0u;
    uint32_t physicalDeviceIndex = 0u;

    //
    //std::vector<vk::Queue> queues = {};

    // 
    DeviceObj(std::shared_ptr<InstanceObj> instanceObj = {}, stm::uni_arg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      this->construct(instanceObj, cInfo);
    };

    //
    virtual tType setPhysicalDeviceIndex(uint32_t const& physicalDeviceIndex = 0u) { this->physicalDeviceIndex = physicalDeviceIndex; return SFT(); };

    //
    virtual std::tuple<uint32_t, uint32_t> findMemoryTypeAndHeapIndex(uint32_t const& requiredMemoryTypeBits, MemoryUsage const& usage = MemoryUsage::eGpuOnly) {
      decltype(auto) physicalDevice = this->physicalDevices[this->physicalDeviceIndex];
      decltype(auto) PDInfoMap = this->PDInfoMaps[this->physicalDeviceIndex];
      decltype(auto) memoryProperties2 = PDInfoMap.set(vk::StructureType::ePhysicalDeviceMemoryProperties2, vk::PhysicalDeviceMemoryProperties2{
        
      });
      decltype(auto) memoryProperties = memoryProperties2->memoryProperties; // get ref
      decltype(auto) memoryTypes = memoryProperties.memoryTypes; // get ref
      physicalDevice.getMemoryProperties2(memoryProperties2);

      // 
      uint32_t bitIndex = 0u;
      std::vector<uint32_t> requiredMemoryTypeIndices = {};
      for (uint32_t bitMask = 1u; (bitMask < 0xFFFFFFFF && bitMask > 0); bitMask <<= 1u) {
          if (requiredMemoryTypeBits & bitMask) { requiredMemoryTypeIndices.push_back(bitIndex); };
          bitIndex++;
      };

      //uint32_t memoryTypeIndex = 0u;
      //for (decltype(auto) memoryType : memoryTypes) {
      std::tuple<uint32_t, uint32_t> memoryTypeAndHeapIndex = {0u, 0u};
      for (decltype(auto) memoryTypeIndex : requiredMemoryTypeIndices) {
        decltype(auto) memoryType = memoryTypes[memoryTypeIndex];
        decltype(auto) memoryHeapIndex = memoryType.heapIndex;
        decltype(auto) requiredBits = vk::MemoryPropertyFlags{};// | vk::MemoryPropertyFlagBits::eDeviceLocal;

        // 
        switch (usage) {
          case (MemoryUsage::eGpuOnly): 
          requiredBits |= vk::MemoryPropertyFlagBits::eDeviceLocal;
          break;

          case (MemoryUsage::eCpuToGpu): 
          //requiredBits |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostCoherent;
          requiredBits |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostCoherent;
          break;

          case (MemoryUsage::eGpuToCpu): 
          requiredBits |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostCoherent;
          break;

          case (MemoryUsage::eCpuOnly): 
          requiredBits |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostCoherent;
          break;

          default: 
          requiredBits |= vk::MemoryPropertyFlagBits::eDeviceLocal;
        };

        // 
        if ((memoryType.propertyFlags & requiredBits) == requiredBits) {
          memoryTypeAndHeapIndex = {memoryTypeIndex, memoryHeapIndex};
          break;
        };
      };

      return memoryTypeAndHeapIndex;
    };

    //
    virtual std::vector<vk::PhysicalDevice>& filterPhysicalDevices(uint32_t const& groupIndex) {
      //this->physicalDevices = {};
      decltype(auto) deviceGroups = this->instanceObj->enumeratePhysicalDeviceGroups();
      decltype(auto) deviceGroup = deviceGroups[groupIndex];
      vk::PhysicalDevice* PDP = deviceGroup.physicalDevices;
      decltype(auto) physicalDevices = (this->physicalDevices = std::vector<vk::PhysicalDevice>(PDP, PDP + deviceGroup.physicalDeviceCount));
      PDInfoMaps.resize(physicalDevices.size(), MSS{});
      return physicalDevices;
    };

    //
    virtual std::vector<vk::DeviceQueueCreateInfo>& cacheQueueInfos() {
      uintptr_t queueInfoIndex = 0ull;
      decltype(auto) queueInfos = queueInfoCache;
      for (decltype(auto) queueInfoMap : this->queueInfoMaps) {
        queueInfos.push_back(queueInfoMap.get<vk::DeviceQueueCreateInfo>(vk::StructureType::eDeviceQueueCreateInfo));
        queueInfoIndex++;
      };
      return queueInfos;
    };

    //
    virtual std::vector<std::string>& filterExtensions(vk::PhysicalDevice const& physicalDevice, std::vector<std::string> const& names) {
      decltype(auto) props = physicalDevice.enumerateDeviceExtensionProperties();
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
    virtual std::vector<std::string>& filterLayers(vk::PhysicalDevice const& physicalDevice, std::vector<std::string> const& names) {
      decltype(auto) props = physicalDevice.enumerateDeviceLayerProperties();
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
    virtual tType filterQueueFamilyIndices(std::vector<uint32_t> const& queueFamilyIndices = {}) {
      this->queueInfoMaps = {};
      this->queuePriorities = {};

      // TODO: customize queue priorities
      for (decltype(auto) queueFamilyIndex : queueFamilyIndices) {
        decltype(auto) last = this->queueInfoMaps.size();
        this->queueInfoMaps.push_back(MSS{});
        this->queuePriorities.push_back(std::vector<float>{1.f});

        // 
        decltype(auto) queueInfoMap = this->queueInfoMaps[last];
        decltype(auto) queueInfo = queueInfoMap.set(vk::StructureType::eDeviceQueueCreateInfo, vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = queueFamilyIndex,
        });
      };

      // set final vector
      for (decltype(auto) priorities : this->queuePriorities) {
        decltype(auto) queueInfoMap = this->queueInfoMaps[std::distance(this->queuePriorities.begin(), std::find(this->queuePriorities.begin(), this->queuePriorities.end(), priorities))];
        decltype(auto) queueInfo = queueInfoMap.get<vk::DeviceQueueCreateInfo>(vk::StructureType::eDeviceQueueCreateInfo);
        queueInfo->setQueuePriorities(priorities);
      };

      return SFT();
    };

    // 
    virtual tType construct(std::shared_ptr<InstanceObj> instanceObj = {}, stm::uni_arg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      this->instanceObj = instanceObj;
      this->infoMap = {};
      this->physicalDevices = {};
      this->extensionList = cInfo->extensionList;
      this->layerList = cInfo->layerList;
      this->extensionNames = {};
      this->layerNames = {};

      //
      decltype(auto) deviceGroupInfo = infoMap.set(vk::StructureType::eDeviceGroupDeviceCreateInfo, vk::DeviceGroupDeviceCreateInfo{

      });

      // TODO: get rid from spagetti code or nesting
      decltype(auto) deviceInfo = infoMap.set(vk::StructureType::eDeviceCreateInfo, vk::DeviceCreateInfo{
          .pNext = infoMap.set(vk::StructureType::ePhysicalDeviceFeatures2, vk::PhysicalDeviceFeatures2{
          .pNext = infoMap.set(vk::StructureType::ePhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan11Features{
          .pNext = infoMap.set(vk::StructureType::ePhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan12Features{
          .pNext = infoMap.set(vk::StructureType::ePhysicalDeviceVulkan13Features, vk::PhysicalDeviceVulkan13Features{
          .pNext = deviceGroupInfo
          })
          })
          })
          })
      });

      //
      decltype(auto) physicalDevices = this->filterPhysicalDevices(this->physicalDeviceGroupIndex = cInfo->physicalDeviceGroupIndex);
      decltype(auto) physicalDevice = physicalDevices[this->physicalDeviceIndex = cInfo->physicalDeviceIndex];

      //
      if (!!physicalDevice) {
        physicalDevice.getFeatures2(infoMap.get<vk::PhysicalDeviceFeatures2>(vk::StructureType::ePhysicalDeviceFeatures2));
        deviceGroupInfo->setPhysicalDevices(physicalDevices);

        // 
        deviceInfo->setQueueCreateInfos(this->filterQueueFamilyIndices(this->queueFamilyIndices = cInfo->queueFamilyIndices)->cacheQueueInfos());
        deviceInfo->setPEnabledExtensionNames(stm::toCString(this->extensionNames, this->filterExtensions(physicalDevice, this->extensionList)));
        deviceInfo->setPEnabledLayerNames(stm::toCString(this->layerNames, this->filterLayers(physicalDevice, this->layerList)));

        // 
        this->device = physicalDevice.createDevice(deviceInfo);
      } else {
        std::cerr << "Physical Device Not Detected" << std::endl;
      };

      // 
      return SFT();
    };
  };


};
