#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"

// 
namespace lxvc {

  // 
  class DeviceObj : std::enable_shared_from_this<DeviceObj> {
  public:
    // 
    vk::Device device = {};
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<DeviceCreateInfo> cInfo = {};
    MSS infoMap = {};

  protected:

    using tType = std::shared_ptr<DeviceObj>;
    using cType = const char const*;
    friend InstanceObj;
    friend BufferObj;
    friend ImageObj;
    friend QueueFamilyObj;

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    //
    std::shared_ptr<InstanceObj> instanceObj = {};

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {};
    std::vector<MSS> PDInfoMaps = {};

    //
    std::vector<cType> extensionNames = {};
    std::vector<cType> layerNames = {};

    // 
    std::vector<vk::DeviceQueueCreateInfo> queueFamilyInfos = {};
    std::vector<uint32_t> queueFamilyIndices = {};
    std::vector<MSS> queueFamilyInfoMaps = {};

    //
    virtual std::tuple<uint32_t, uint32_t> findMemoryTypeAndHeapIndex(vk::PhysicalDevice const& physicalDevice, cpp21::optional_ref<MemoryRequirements> req = MemoryRequirements{}) {
      //decltype(auto) physicalDevice = this->physicalDevices[req->physicalDeviceIndex];
      decltype(auto) physicalDeviceIndex = std::distance(this->physicalDevices.begin(), std::find(this->physicalDevices.begin(), this->physicalDevices.end(), physicalDevice));
      decltype(auto) PDInfoMap = this->PDInfoMaps[physicalDeviceIndex];
      decltype(auto) memoryProperties2 = PDInfoMap.set(vk::StructureType::ePhysicalDeviceMemoryProperties2, vk::PhysicalDeviceMemoryProperties2{

        });
      decltype(auto) memoryProperties = memoryProperties2->memoryProperties; // get ref
      decltype(auto) memoryTypes = memoryProperties.memoryTypes; // get ref
      physicalDevice.getMemoryProperties2(memoryProperties2.get());

      // 
      uint32_t bitIndex = 0u;
      std::vector<uint32_t> requiredMemoryTypeIndices = {};
      for (uint32_t bitMask = 1u; (bitMask < 0xFFFFFFFF && bitMask > 0); bitMask <<= 1u) {
        if (req->memoryTypeBits & bitMask) { requiredMemoryTypeIndices.push_back(bitIndex); };
        bitIndex++;
      };

      //uint32_t memoryTypeIndex = 0u;
      //for (decltype(auto) memoryType : memoryTypes) {
      std::tuple<uint32_t, uint32_t> memoryTypeAndHeapIndex = { 0u, 0u };
      for (decltype(auto) memoryTypeIndex : requiredMemoryTypeIndices) {
        decltype(auto) memoryType = memoryTypes[memoryTypeIndex];
        decltype(auto) memoryHeapIndex = memoryType.heapIndex;
        decltype(auto) requiredBits = vk::MemoryPropertyFlags{};// | vk::MemoryPropertyFlagBits::eDeviceLocal;

        // 
        switch (req->memoryUsage) {
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
          memoryTypeAndHeapIndex = { memoryTypeIndex, memoryHeapIndex };
          break;
        };
      };

      return memoryTypeAndHeapIndex;
    };

    //
    virtual std::vector<cType>& filterLayers(vk::PhysicalDevice const& physicalDevice, std::vector<std::string> const& names) {
      decltype(auto) props = physicalDevice.enumerateDeviceLayerProperties();
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
    virtual std::vector<cType>& filterExtensions(vk::PhysicalDevice const& physicalDevice, std::vector<std::string> const& names) {
      decltype(auto) props = physicalDevice.enumerateDeviceExtensionProperties();
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
    virtual std::vector<vk::DeviceQueueCreateInfo>& filterQueueFamilies(std::vector<QueueFamilyCreateInfo> const& qfInfosIn = {}) {

      // TODO: customize queue priorities
      decltype(auto) qfInfosVk = opt_ref(this->queueFamilyInfos = {});
      decltype(auto) qfIndices = opt_ref(this->queueFamilyIndices = {});
      decltype(auto) qfInfoMaps = opt_ref(this->queueFamilyInfoMaps = {});
      for (decltype(auto) qfInfoIn : qfInfosIn) {
        qfInfoMaps.push_back(MSS());
        decltype(auto) qfInfoMap = qfInfoMaps.back();
        decltype(auto) qfInfoVk = qfInfoMap.set(vk::StructureType::eDeviceQueueCreateInfo, vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = qfInfoIn.queueFamilyIndex,
        });
        qfIndices->push_back(qfInfoIn.queueFamilyIndex);
        qfInfosVk->push_back(qfInfoVk->setQueuePriorities(*qfInfoIn.queuePriorities));
      };

      // 
      return qfInfosVk;
    };


  public:

    // 
    DeviceObj(std::shared_ptr<InstanceObj> instanceObj = {}, cpp21::optional_ref<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) : instanceObj(instanceObj), cInfo(cInfo) {
      this->construct(instanceObj, cInfo);
    };

    // 
    virtual tType construct(std::shared_ptr<InstanceObj> instanceObj = {}, cpp21::optional_ref<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      this->instanceObj = instanceObj;
      this->physicalDevices = {};
      this->extensionNames = {};
      this->layerNames = {};
      this->infoMap = {};
      memcpy(&this->cInfo, &cInfo, sizeof(DeviceCreateInfo));

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
      decltype(auto) physicalDevices = this->filterPhysicalDevices(this->cInfo->physicalDeviceGroupIndex);
      decltype(auto) physicalDevice = physicalDevices[this->cInfo->physicalDeviceIndex];

      //
      if (!!physicalDevice) {
        physicalDevice.getFeatures2(infoMap.get<vk::PhysicalDeviceFeatures2>(vk::StructureType::ePhysicalDeviceFeatures2).get());
        deviceGroupInfo->setPhysicalDevices(physicalDevices);

        // 
        deviceInfo->setQueueCreateInfos(this->filterQueueFamilies(this->cInfo->queueFamilyInfos));
        deviceInfo->setPEnabledExtensionNames(this->filterExtensions(physicalDevice, this->cInfo->extensionList));
        deviceInfo->setPEnabledLayerNames(this->filterLayers(physicalDevice, this->cInfo->layerList));

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
