#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"

// 
namespace lxvc {

  //
  struct QueueFamilies {
    cpp21::vector_of_shared<MSS> infoMaps = {};
    std::vector<uint32_t> indices = {};
    std::vector<vk::DeviceQueueCreateInfo> infos = {};
    std::vector<vk::CommandPool> commandPools = {};
    std::vector<std::vector<vk::Queue>> queues = {};
  };

  // 
  class DeviceObj : std::enable_shared_from_this<DeviceObj> {
  protected:
    // 
    vk::Device device = {};
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<DeviceCreateInfo> cInfo = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    friend InstanceObj;
    friend ResourceObj;
    friend QueueFamilyObj;
    friend DescriptorsObj;
    friend PipelineObj;

    //
    using tType = std::shared_ptr<DeviceObj>;
    using cType = const char const*;

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    //
    std::shared_ptr<InstanceObj> instanceObj = {};

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {};
    cpp21::vector_of_shared<MSS> PDInfoMaps = {};

    //
    std::vector<cType> extensionNames = {};
    std::vector<cType> layerNames = {};

    //
    QueueFamilies queueFamilies = {};

    //
    virtual std::tuple<uint32_t, uint32_t> findMemoryTypeAndHeapIndex(vk::PhysicalDevice const& physicalDevice, cpp21::optional_ref<MemoryRequirements> req = MemoryRequirements{}) {
      //decltype(auto) physicalDevice = this->physicalDevices[req->physicalDeviceIndex];
      uintptr_t physicalDeviceIndex = std::distance(this->physicalDevices.begin(), std::find(this->physicalDevices.begin(), this->physicalDevices.end(), physicalDevice));
      decltype(auto) PDInfoMap = this->PDInfoMaps[physicalDeviceIndex];
      decltype(auto) memoryProperties2 = PDInfoMap->set(vk::StructureType::ePhysicalDeviceMemoryProperties2, vk::PhysicalDeviceMemoryProperties2{

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
      for (decltype(auto) PD : physicalDevices) {
        PDInfoMaps->push_back(std::make_shared<MSS>());
      };
      return physicalDevices;
    };

    // 
    virtual std::vector<vk::DeviceQueueCreateInfo>& filterQueueFamilies(std::vector<QueueFamilyCreateInfo> const& qfInfosIn = {}) {

      // TODO: customize queue priorities
      decltype(auto) qfInfosVk = opt_ref(this->queueFamilies.infos = {});
      decltype(auto) qfIndices = opt_ref(this->queueFamilies.indices = {});
      decltype(auto) qfInfoMaps = opt_ref(this->queueFamilies.infoMaps = {});
      decltype(auto) qfCommandPools = opt_ref(this->queueFamilies.commandPools = {});
      decltype(auto) qfQueuesStack = opt_ref(this->queueFamilies.queues = {});
      for (decltype(auto) qfInfoIn : qfInfosIn) {
        qfInfoMaps->push_back(std::make_shared<MSS>());
        qfQueuesStack->push_back(std::vector<vk::Queue>{});
        decltype(auto) qfInfoMap = qfInfoMaps->back();
        decltype(auto) qfInfoVk = qfInfoMap->set(vk::StructureType::eDeviceQueueCreateInfo, vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = qfInfoIn.queueFamilyIndex,
        });
        qfIndices->push_back(qfInfoIn.queueFamilyIndex);
        qfInfosVk->push_back(qfInfoVk->setQueuePriorities(*qfInfoIn.queuePriorities));
      };

      // 
      return qfInfosVk;
    };

    //
    virtual std::vector<vk::CommandPool>& createCommandPools(std::vector<QueueFamilyCreateInfo> const& qfInfosIn = {}) {
      //uintptr_t index = 0u;
      if (this->queueFamilies.commandPools.size() <= 0u) {
        decltype(auto) qfInfosVk = opt_ref(this->queueFamilies.infos);
        decltype(auto) qfIndices = opt_ref(this->queueFamilies.indices);
        decltype(auto) qfInfoMaps = opt_ref(this->queueFamilies.infoMaps);
        decltype(auto) qfCommandPools = opt_ref(this->queueFamilies.commandPools);
        decltype(auto) qfQueuesStack = opt_ref(this->queueFamilies.queues);
        for (decltype(auto) qfInfoIn : qfInfosIn) {
          uintptr_t indexOfQF = std::distance(qfIndices->begin(), std::find(qfIndices->begin(), qfIndices->end(), qfInfoIn.queueFamilyIndex));
          decltype(auto) qfIndex = qfIndices[indexOfQF];
          decltype(auto) qfInfoMap = qfInfoMaps[indexOfQF];
          decltype(auto) qfQueues = qfQueuesStack[indexOfQF];
          decltype(auto) qfInfoVk = qfInfoMap[indexOfQF].get<vk::DeviceQueueCreateInfo>(vk::StructureType::eDeviceQueueCreateInfo);
          qfCommandPools->push_back(this->device.createCommandPool(qfInfoMap->set(vk::StructureType::eCommandPoolCreateInfo, vk::CommandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = qfIndex,
          })));
          for (decltype(auto) i = 0u; i < qfInfoVk->queueCount; i++) {
            qfQueues.push_back(this->device.getQueue(qfIndex, i));
          };
          //index++;
        };
      };
      return this->queueFamilies.commandPools;
    };

    // TODO: caching...
    virtual vk::Queue const& getQueue(cpp21::optional_ref<QueueGetInfo> info = {}) const {
      //return this->device.getQueue(info->queueFamilyIndex, info->queueIndex);
      decltype(auto) qfIndices = opt_ref(this->queueFamilies.indices);
      decltype(auto) qfQueuesStack = opt_ref(this->queueFamilies.queues);
      uintptr_t indexOfQF = std::distance(qfIndices->begin(), std::find(qfIndices->begin(), qfIndices->end(), info->queueFamilyIndex));
      return qfQueuesStack[indexOfQF][info->queueIndex];
    };

    //
    virtual vk::Fence executeCommandOnce(cpp21::optional_ref<CommandSubmission> submissionRef = {}) {
      std::optional<CommandSubmission> submission = submissionRef;
      decltype(auto) qfIndices = opt_ref(this->queueFamilies.indices);
      decltype(auto) qfCommandPools = opt_ref(this->queueFamilies.commandPools);
      uintptr_t indexOfQF = std::distance(qfIndices->begin(), std::find(qfIndices->begin(), qfIndices->end(), submissionRef->info->queueFamilyIndex));
      decltype(auto) queue = this->getQueue(submissionRef->info);
      decltype(auto) commandPool = qfCommandPools[indexOfQF];
      decltype(auto) commandBuffers = this->device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = (uint32_t)submission->commandInits.size()
      });

      //
      decltype(auto) cmdInfos = std::vector<vk::CommandBufferSubmitInfo >{};
      decltype(auto) submitInfo = vk::SubmitInfo2{};
      decltype(auto) cIndex = 0u; for (decltype(auto) fn : submission->commandInits) {
        decltype(auto) cmdBuf = commandBuffers[cIndex++];
        cmdInfos.push_back(vk::CommandBufferSubmitInfo{
          .commandBuffer = fn(cmdBuf),
          .deviceMask = 0x1
        });
      };

      // 
      //decltype(auto) 
      decltype(auto) fence = this->device.createFence(vk::FenceCreateInfo{ .flags = {} });
      decltype(auto) submits = std::vector<vk::SubmitInfo2>{
        submitInfo.setCommandBufferInfos(cmdInfos).setWaitSemaphoreInfos(*submission->waitSemaphores).setSignalSemaphoreInfos(*submission->signalSemaphores)
      };
      queue.submit2(submits, fence);

      // 
      std::async(std::launch::async | std::launch::deferred, [=,this]() {
        this->device.waitForFences(fence, true, 10000000000);
        for (decltype(auto) fn : submission->onDone) { fn(); };
      });

      // 
      return fence;
    };

    // 
    virtual tType construct(std::shared_ptr<InstanceObj> instanceObj = {}, cpp21::optional_ref<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      this->instanceObj = instanceObj;
      this->physicalDevices = {};
      this->extensionNames = {};
      this->layerNames = {};
      this->infoMap = std::make_shared<MSS>();
      this->cInfo = cInfo;
      //memcpy(&this->cInfo, &cInfo, sizeof(DeviceCreateInfo));

      // TODO: get rid from spagetti code or nesting
      decltype(auto) PDInfoMap = this->PDInfoMaps[this->cInfo->physicalDeviceIndex];
      decltype(auto) deviceGroupInfo = this->infoMap->set(vk::StructureType::eDeviceGroupDeviceCreateInfo, vk::DeviceGroupDeviceCreateInfo{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceFeatures2, vk::PhysicalDeviceFeatures2{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan11Features{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan12Features{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan13Features, vk::PhysicalDeviceVulkan13Features{

        })
        })
        })
        })
      });

      // 
      decltype(auto) deviceInfo = infoMap->set(vk::StructureType::eDeviceCreateInfo, vk::DeviceCreateInfo{ .pNext = deviceGroupInfo });
      decltype(auto) physicalDevices = this->filterPhysicalDevices(this->cInfo->physicalDeviceGroupIndex);
      decltype(auto) physicalDevice = physicalDevices[this->cInfo->physicalDeviceIndex];

      //
      if (!!physicalDevice) {
        physicalDevice.getFeatures2(infoMap->get<vk::PhysicalDeviceFeatures2>(vk::StructureType::ePhysicalDeviceFeatures2).get());
        deviceGroupInfo->setPhysicalDevices(physicalDevices);

        // 
        deviceInfo->setQueueCreateInfos(this->filterQueueFamilies(this->cInfo->queueFamilyInfos));
        deviceInfo->setPEnabledExtensionNames(this->filterExtensions(physicalDevice, this->cInfo->extensionList));
        deviceInfo->setPEnabledLayerNames(this->filterLayers(physicalDevice, this->cInfo->layerList));

        // 
        this->device = physicalDevice.createDevice(deviceInfo);
        this->createCommandPools(this->cInfo->queueFamilyInfos);

      }
      else {
        std::cerr << "Physical Device Not Detected" << std::endl;
      };

      // 
      return SFT();
    };

  public:

    // 
    DeviceObj(std::shared_ptr<InstanceObj> instanceObj = {}, cpp21::optional_ref<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) : instanceObj(instanceObj), cInfo(cInfo) {
      this->construct(instanceObj, cInfo);
    };

    
  };


};
