#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"

// 
namespace ANAMED {

  //
  struct QueueFamilies {
    cpp21::vector_of_shared<MSS> infoMaps = {};
    std::vector<uint32_t> indices = {};
    std::vector<vk::DeviceQueueCreateInfo> infos = {};
    std::vector<vk::CommandPool> commandPools = {};
    std::vector<std::vector<vk::Queue>> queues = {};
  };

  // 
  class DeviceObj : public BaseObj {
  public:
    using BaseObj::BaseObj;
    using tType = WrapShared<DeviceObj>;

  protected:
    // 
    //vk::Device device = {};
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<DeviceCreateInfo> cInfo = DeviceCreateInfo{};
    
    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {};
    cpp21::vector_of_shared<MSS> PDInfoMaps = {};

    //
    std::vector<char const*> extensionNames = {};
    std::vector<char const*> layerNames = {};

    //
    QueueFamilies queueFamilies = {};

    //
    SMAP addressSpace = {};

    //
    //std::vector<std::shared_ptr<std::future<vk::Result>>> futures = {};
    std::vector<FenceType> fences = {};

  public:

    //
    virtual std::tuple<uint32_t, uint32_t> findMemoryTypeAndHeapIndex(cpp21::carg<MemoryRequirements> req = MemoryRequirements{}, cpp21::carg<uintptr_t> physicalDeviceIndex = {}) {
      auto& physicalDevice = this->getPhysicalDevice(physicalDeviceIndex);
      decltype(auto) PDInfoMap = this->getPhysicalDeviceInfoMap(physicalDeviceIndex);
      auto memoryProperties2 = PDInfoMap->set(vk::StructureType::ePhysicalDeviceMemoryProperties2, vk::PhysicalDeviceMemoryProperties2{

      });
      auto& memoryProperties = memoryProperties2->memoryProperties; // get ref
      auto& memoryTypes = memoryProperties.memoryTypes; // get ref
      physicalDevice.getMemoryProperties2(memoryProperties2.get());
      

      // 
      uint32_t bitIndex = 0u;
      std::vector<uint32_t> requiredMemoryTypeIndices = {};
      for (uint32_t bitMask = 1u; (bitMask < 0xFFFFFFFF && bitMask > 0); bitMask <<= 1u) {
        if (req->requirements.memoryTypeBits & bitMask) { requiredMemoryTypeIndices.push_back(bitIndex); };
        //requiredMemoryTypeIndices.push_back(bitIndex); // sparse memory requires eDeviceLocal?!
        bitIndex++;
      };

      //
      auto requiredBits = vk::MemoryPropertyFlags{};// | vk::MemoryPropertyFlagBits::eDeviceLocal;
      auto declineBits = vk::MemoryPropertyFlags{};

      // 
      switch (req->memoryUsage) {
      case (MemoryUsage::eGpuOnly):
        requiredBits |= vk::MemoryPropertyFlagBits::eDeviceLocal;
        declineBits |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        break;

      case (MemoryUsage::eCpuToGpu):
        requiredBits |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostCoherent;
        declineBits |= vk::MemoryPropertyFlagBits::eDeviceLocal;
        break;

      case (MemoryUsage::eGpuToCpu):
        requiredBits |= vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        break;

      case (MemoryUsage::eCpuOnly):
        requiredBits |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostCoherent;
        break;

      default:;
      };

      //uint32_t memoryTypeIndex = 0u;
      //for (decltype(auto) memoryType : memoryTypes) {
      std::tuple<uint32_t, uint32_t> memoryTypeAndHeapIndex = { 0u, 0u };

      for (auto& memoryTypeIndex : requiredMemoryTypeIndices) {
        auto& memoryType = memoryTypes[memoryTypeIndex];
        auto& memoryHeapIndex = memoryType.heapIndex;

        //
        //std::cout << "MemoryTypeIndex: " << uint32_t(memoryTypeIndex) << std::endl;
        //std::cout << "MemoryPropertyFlags: " << uint32_t(memoryType.propertyFlags) << std::endl;
        //std::cout << "RequiredBits: " << uint32_t(requiredBits) << std::endl;

        // 
        if ((uint32_t(memoryType.propertyFlags & requiredBits) == uint32_t(requiredBits)) && uint32_t(memoryType.propertyFlags & declineBits) == 0u || (!memoryType.propertyFlags) || (!requiredBits)) {
          //std::cout << "ResolvedMemTypeAndHeap: " << uint32_t(memoryTypeIndex) << ", " << uint32_t(memoryHeapIndex) << std::endl;
          //std::cout << "" << std::endl;
          memoryTypeAndHeapIndex = { memoryTypeIndex, memoryHeapIndex };
          break;
        }
        else {
          //std::cout << "Not capable..." << std::endl;
          //std::cout << "" << std::endl;
        }
        
      };

      return memoryTypeAndHeapIndex;
    };

  protected: 

    //
    virtual std::vector<char const*>& filterLayers(cpp21::carg<vk::PhysicalDevice> physicalDevice, cpp21::carg<std::vector<std::string>> names) {
      decltype(auto) props = physicalDevice->enumerateDeviceLayerProperties();
      auto& selected = this->layerNames;

      // 
      uintptr_t nameIndex = 0ull;
      for (auto const& name : (*names)) {
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
    virtual std::vector<char const*>& filterExtensions(cpp21::carg<vk::PhysicalDevice> physicalDevice, cpp21::carg<std::vector<std::string>> names) {
      decltype(auto) props = physicalDevice->enumerateDeviceExtensionProperties(std::string(""));
      auto& selected = (this->extensionNames);

      // 
      uintptr_t nameIndex = 0ull;
      for (auto const& name : (*names)) {
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
    virtual std::vector<vk::PhysicalDevice>& filterPhysicalDevices(cpp21::carg<uint32_t> groupIndex = 0u) {
      //this->physicalDevices = {};
      decltype(auto) instanceObj = ANAMED::context->get<InstanceObj>(this->base);
      decltype(auto) deviceGroups = instanceObj->enumeratePhysicalDeviceGroups();
      auto& physicalDevices = this->physicalDevices;
      if (deviceGroups.size() > 0) {
        decltype(auto) deviceGroup = deviceGroups[*groupIndex];
        vk::PhysicalDevice* PDP = deviceGroup.physicalDevices;
        physicalDevices = std::vector<vk::PhysicalDevice>(PDP, PDP + deviceGroup.physicalDeviceCount);
      } else {
        physicalDevices = instanceObj->enumeratePhysicalDevices();
      };
      for (decltype(auto) PD : physicalDevices) {
        PDInfoMaps->push_back(std::make_shared<MSS>(MSS()));
      };
      return this->physicalDevices;
    };

    // 
    virtual std::vector<vk::DeviceQueueCreateInfo>& filterQueueFamilies(cpp21::carg<std::vector<QueueFamilyCreateInfo>> qfInfosIn = {}) {

      // TODO: customize queue priorities
      auto& qfInfosVk = (this->queueFamilies.infos = {});
      auto& qfIndices = (this->queueFamilies.indices = {});
      auto& qfInfoMaps = (this->queueFamilies.infoMaps = std::vector<std::shared_ptr<MSS>>{});
      auto& qfCommandPools = (this->queueFamilies.commandPools = {});
      auto& qfQueuesStack = (this->queueFamilies.queues = {});
      for (auto& qfInfoIn : (*qfInfosIn)) {
        qfInfoMaps.push_back(std::make_shared<MSS>(MSS()));
        qfQueuesStack.push_back(std::vector<vk::Queue>{});
        auto& qfInfoMap = qfInfoMaps->back();
        //auto qfPriorityVk = qfInfoMap->set(vk::StructureType::eDeviceQueueGlobalPriorityCreateInfo, vk::DeviceQueueGlobalPriorityCreateInfo{
        //  .globalPriority = vk::GlobalPriority::eRealtime
        //});
        auto qfInfoVk = qfInfoMap->set(vk::StructureType::eDeviceQueueCreateInfo, vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = qfInfoIn.queueFamilyIndex,
        });
        //qfPriorityVk.pNext = qfInfoVk.get();
        qfIndices.push_back(qfInfoIn.queueFamilyIndex);
        qfInfosVk.push_back(qfInfoVk->setQueuePriorities(*qfInfoIn.queuePriorities));
      };

      // 
      return qfInfosVk;
    };

    //
    virtual std::vector<vk::CommandPool>& createCommandPools(cpp21::carg<std::vector<QueueFamilyCreateInfo>> qfInfosIn = {}) {
      //uintptr_t index = 0u;
      auto& device = this->handle.as<vk::Device>(); // finally found issue
      if (!!device && this->queueFamilies.commandPools.size() <= 0u) {
        auto& qfInfosVk = (this->queueFamilies.infos);
        auto& qfIndices = (this->queueFamilies.indices);
        auto& qfInfoMaps = (this->queueFamilies.infoMaps);
        auto& qfCommandPools = (this->queueFamilies.commandPools);
        auto& qfQueuesStack = (this->queueFamilies.queues);
        for (auto& qfInfoIn : (*qfInfosIn)) {
          uintptr_t indexOfQF = std::distance(qfIndices.begin(), std::find(qfIndices.begin(), qfIndices.end(), qfInfoIn.queueFamilyIndex));
          auto& qfIndex = qfIndices[indexOfQF];
          auto qfInfoMap = qfInfoMaps[indexOfQF];
          auto& qfQueues = qfQueuesStack[indexOfQF];
          auto qfInfoVk = qfInfoMap->get<vk::DeviceQueueCreateInfo>(vk::StructureType::eDeviceQueueCreateInfo);
          auto qfCmdPoolInfo = qfInfoMap->set(vk::StructureType::eCommandPoolCreateInfo, vk::CommandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = qfIndex,
          });
          qfCommandPools.push_back(device.createCommandPool(qfCmdPoolInfo.ref()));
          for (decltype(auto) i = 0u; i < qfInfoVk->queueCount; i++) {
            qfQueues.push_back(device.getQueue(qfIndex, i));
          };
          //index++;
        };
      };
      return this->queueFamilies.commandPools;
    };

  public:

    //
     void tickProcessing() override {
      std::decay_t<decltype(fences)>::iterator fenceIt = fences.begin();
      while(fenceIt != fences.end()) {
        decltype(auto) fence = *fenceIt;
        bool ready = fence->checkStatus();
        //bool ready = !(fence->future) || !fence->future->valid() || cpp21::is_ready(*fence->future);
        if (ready) {
          fenceIt = fences.erase(fenceIt);
        } else {
          fenceIt++;
        }
      };
      if (this->callstack) {
        this->callstack->process();
      };
    };

    //
    virtual std::vector<FenceType>& getFences() { return this->fences; };
    virtual std::vector<FenceType> const& getFences() const { return this->fences; };

    //
    virtual FenceType executeCommandOnce(cpp21::carg<CommandOnceSubmission> submissionRef_ = {}) {
      decltype(auto) submissionRef = std::make_shared<CommandOnceSubmission>(submissionRef_);
      auto& submission = submissionRef->submission;
      auto& device = this->handle.as<vk::Device>();
      auto& qfIndices = (this->queueFamilies.indices);
      auto& qfCommandPools = (this->queueFamilies.commandPools);
      uintptr_t indexOfQF = std::distance(qfIndices.begin(), std::find(qfIndices.begin(), qfIndices.end(), submission.info->queueFamilyIndex));
      auto& queue = this->getQueue(submission.info);
      auto& commandPool = qfCommandPools[indexOfQF];
      auto commandBuffers = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = (uint32_t)submissionRef->commandInits.size()
      });

      //
      auto cmdInfos = std::vector<vk::CommandBufferSubmitInfo >{};
      auto submitInfo = vk::SubmitInfo2{};
      auto cIndex = 0u; for (auto& fn : submissionRef->commandInits) {
        auto& cmdBuf = commandBuffers[cIndex++];
        cmdBuf.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit, .pInheritanceInfo = cpp21::pointer(submission.inheritanceInfo) });
        auto result = fn(cmdBuf);
        cmdInfos.push_back(vk::CommandBufferSubmitInfo{
          .commandBuffer = cmdBuf,
          .deviceMask = 0x1
        });
        cmdBuf.end();
      };

      // 
      auto fence = std::make_shared<vk::Fence>(device.createFence(vk::FenceCreateInfo{ .flags = {} }));
      auto submits = std::vector<vk::SubmitInfo2>{
        submitInfo.setCommandBufferInfos(cmdInfos).setWaitSemaphoreInfos(*submission.waitSemaphores).setSignalSemaphoreInfos(*submission.signalSemaphores)
      };
      queue.submit2(submits, *fence);

      //
      auto getStatus = [device, fence]() {
        if (fence && *fence) { return device.getFenceStatus(*fence) != vk::Result::eNotReady; };
        return true;
      };

      // 
      auto deAllocation = [device, fence, commandPool, commandBuffers]() {
        if (fence && *fence) {
          device.destroyFence(cpp21::exchange(*fence, vk::Fence{}));
          device.freeCommandBuffers(commandPool, commandBuffers);
        };
      };

      //
      auto onDone = [device, fence, callstack = std::weak_ptr<CallStack>(this->callstack), submissionRef, deAllocation]() {
        auto cl = callstack.lock();
        for (auto& fn : submissionRef->submission.onDone) {
          cl->add(std::bind(fn, device.getFenceStatus(*fence)));
        };
        cl->add(deAllocation);
      };

      //
      auto status = std::make_shared<FenceStatus>(getStatus, onDone);
      fences.push_back(status);

      // clean and call events
      this->tickProcessing();

      // 
      return status;
    };

    //
    virtual SMAP& getAddressSpace() { return this->addressSpace; };
    virtual SMAP const& getAddressSpace() const { return this->addressSpace; };

    //
    virtual vk::PhysicalDevice& getPhysicalDevice(cpp21::carg<uintptr_t> physicalDeviceIndex = {}) { return this->physicalDevices[std::min(physicalDeviceIndex ? physicalDeviceIndex.value() : this->cInfo->physicalDeviceIndex, this->physicalDevices.size() - 1)]; };
    virtual vk::PhysicalDevice const& getPhysicalDevice(cpp21::carg<uintptr_t> physicalDeviceIndex = {}) const { return this->physicalDevices[std::min(physicalDeviceIndex ? physicalDeviceIndex.value() : this->cInfo->physicalDeviceIndex, this->physicalDevices.size() - 1)]; };

    //
    virtual std::shared_ptr<MSS> getPhysicalDeviceInfoMap(cpp21::carg<uintptr_t> physicalDeviceIndex = {}) { return this->PDInfoMaps[std::min(physicalDeviceIndex ? physicalDeviceIndex.value() : this->cInfo->physicalDeviceIndex, this->PDInfoMaps.size() - 1)]; };
    virtual std::shared_ptr<MSS> getPhysicalDeviceInfoMap(cpp21::carg<uintptr_t> physicalDeviceIndex = {}) const { return this->PDInfoMaps[std::min(physicalDeviceIndex ? physicalDeviceIndex.value() : this->cInfo->physicalDeviceIndex, this->PDInfoMaps.size() - 1)]; };

    //
    virtual vk::DispatchLoaderDynamic& getDispatch() { return this->dispatch; };
    virtual vk::DispatchLoaderDynamic const& getDispatch() const { return this->dispatch; };

    //
    virtual QueueFamilies& getQueueFamilies() { return queueFamilies; };
    virtual QueueFamilies const& getQueueFamilies() const { return queueFamilies; };

    //
    virtual std::shared_ptr<MemoryAllocatorObj> createDefaultMemoryAllocator();

  protected:

    // 
    virtual void construct(std::shared_ptr<InstanceObj> instanceObj = {}, cpp21::carg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      //this->instanceObj = instanceObj;
      this->physicalDevices = {};
      this->extensionNames = {};
      this->layerNames = {};
      this->infoMap = std::make_shared<MSS>(MSS());
      this->callstack = std::make_shared<CallStack>();
      if (cInfo) { this->cInfo = cInfo; };

      //memcpy(&this->cInfo, &cInfo, sizeof(DeviceCreateInfo));

      // TODO: get rid from spagetti code or nesting
      auto& physicalDevices = this->filterPhysicalDevices(this->cInfo->physicalDeviceGroupIndex);
      auto& physicalDevice = this->getPhysicalDevice();
      auto PDInfoMap = this->getPhysicalDeviceInfoMap();

      // 
      auto features2 = PDInfoMap->set(vk::StructureType::ePhysicalDeviceFeatures2, vk::PhysicalDeviceFeatures2{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan11Features{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan12Features{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan13Features, vk::PhysicalDeviceVulkan13Features{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceMultiDrawFeaturesEXT, vk::PhysicalDeviceMultiDrawFeaturesEXT{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceAccelerationStructureFeaturesKHR, vk::PhysicalDeviceAccelerationStructureFeaturesKHR{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceRayQueryFeaturesKHR, vk::PhysicalDeviceRayQueryFeaturesKHR{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR, vk::PhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR, vk::PhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceShaderClockFeaturesKHR, vk::PhysicalDeviceShaderClockFeaturesKHR{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceFragmentShadingRateFeaturesKHR, vk::PhysicalDeviceFragmentShadingRateFeaturesKHR{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceGraphicsPipelineLibraryFeaturesEXT, vk::PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceShaderAtomicFloatFeaturesEXT, vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceShaderAtomicFloat2FeaturesEXT, vk::PhysicalDeviceShaderAtomicFloat2FeaturesEXT{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceFragmentShaderBarycentricFeaturesKHR, vk::PhysicalDeviceFragmentShaderBarycentricFeaturesKHR{ // BECOME LAGGY!
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceMemoryPriorityFeaturesEXT, vk::PhysicalDeviceMemoryPriorityFeaturesEXT{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceRayTracingMaintenance1FeaturesKHR, vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceFragmentShaderInterlockFeaturesEXT, vk::PhysicalDeviceFragmentShaderInterlockFeaturesEXT{
        .pNext = nullptr
        })
        })
        })
        })
        })
        })
        })
        })
        })
        })
        })
        })
        })
        })
        })
        })
        })
      });

      // 
      auto deviceGroupInfo = this->infoMap->set(vk::StructureType::eDeviceGroupDeviceCreateInfo, vk::DeviceGroupDeviceCreateInfo{
        .pNext = features2.get()
      });

      // 
      auto properties2 = PDInfoMap->set(vk::StructureType::ePhysicalDeviceProperties2, vk::PhysicalDeviceProperties2{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan11Properties, vk::PhysicalDeviceVulkan11Properties{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan12Properties, vk::PhysicalDeviceVulkan12Properties{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan13Properties, vk::PhysicalDeviceVulkan13Properties{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceConservativeRasterizationPropertiesEXT, vk::PhysicalDeviceConservativeRasterizationPropertiesEXT{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceAccelerationStructurePropertiesKHR, vk::PhysicalDeviceAccelerationStructurePropertiesKHR{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceMultiDrawPropertiesEXT, vk::PhysicalDeviceMultiDrawPropertiesEXT{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceFragmentShaderBarycentricPropertiesKHR, vk::PhysicalDeviceFragmentShaderBarycentricPropertiesKHR{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDevicePushDescriptorPropertiesKHR, vk::PhysicalDevicePushDescriptorPropertiesKHR{
        .pNext = nullptr
        })
        })
        })
        })
        })
        })
        })
        })
      });

      //
      auto& features = features2->features;
      auto& properties = properties2->properties;

      // device group support was broken...
      decltype(auto) deviceInfo = infoMap->set(vk::StructureType::eDeviceCreateInfo, vk::DeviceCreateInfo{ .pNext = features2.get() });
      
      

      //
      {
        physicalDevice.getProperties2(properties2.get());
        physicalDevice.getFeatures2(features2.get());
        deviceGroupInfo->setPhysicalDevices(physicalDevices);

        //
        std::cout << "Used Device: " << std::string_view(properties.deviceName) << std::endl;
        std::cout << "" << std::endl;

        // 
        deviceInfo->setQueueCreateInfos(this->filterQueueFamilies(this->cInfo->queueFamilyInfos.ref()));
        deviceInfo->setPEnabledExtensionNames(this->filterExtensions(physicalDevice, this->cInfo->extensionList.ref()));
        deviceInfo->setPEnabledLayerNames(this->filterLayers(physicalDevice, this->cInfo->layerList.ref()));

        // 
        if (deviceInfo) {
          //try {
            this->handle = physicalDevice.createDevice(deviceInfo.ref(), nullptr, instanceObj->dispatch);
          //}
          //catch (std::exception e) {
            //std::cerr << "Unable to create device..." << std::endl;
            //std::cerr << e.what() << std::endl;
          //}
        };
        if (this->handle) {
          //VULKAN_HPP_DEFAULT_DISPATCHER.init(this->handle.as<vk::Device>());
          this->dispatch = vk::DispatchLoaderDynamic(this->base.as<vk::Instance>(), vkGetInstanceProcAddr, this->handle.as<vk::Device>(), vkGetDeviceProcAddr);
          this->createCommandPools(this->cInfo->queueFamilyInfos.ref());
        };

        // 
        
      };

      
      // 
      //return SFT();
    };

  public:

    // TODO: caching...
    virtual vk::Queue const& getQueue(cpp21::carg<QueueGetInfo> info = {}) const {
      //return this->device.getQueue(info->queueFamilyIndex, info->queueIndex);
      decltype(auto) qfIndices = (this->queueFamilies.indices);
      decltype(auto) qfQueuesStack = (this->queueFamilies.queues);
      uintptr_t indexOfQF = std::distance(qfIndices.begin(), std::find(qfIndices.begin(), qfIndices.end(), info->queueFamilyIndex));
      return qfQueuesStack[indexOfQF][info->queueIndex];
    };

    //
    virtual tType writeCopyBuffersCommand(cpp21::carg<CopyBufferWriteInfo> copyInfoRaw);

    //
    virtual FenceType copyBuffersOnce(cpp21::carg<CopyBuffersExecutionOnce> copyInfo) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = copyInfo->submission };
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      
      // 
      submission.commandInits.push_back([copyInfo, this](cpp21::carg<vk::CommandBuffer> cmdBuf) {
        this->writeCopyBuffersCommand(copyInfo->writeInfo.with(cmdBuf));
        return cmdBuf;
      });

      //
      return this->executeCommandOnce(submission);
    };

    //
    ~DeviceObj() {
      this->tickProcessing();
    };

    // 
    DeviceObj(WrapShared<InstanceObj> instanceObj = {}, cpp21::carg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) : BaseObj(std::move(instanceObj->getHandle())), cInfo(cInfo) {
      //this->construct(instanceObj, cInfo);
    };

    // 
    DeviceObj(cpp21::carg<Handle> handle, cpp21::carg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      //this->construct(ANAMED::context->get<InstanceObj>(this->base), cInfo);
    };

    //
    virtual tType registerSelf() {
      ANAMED::context->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    // 
     std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    inline static tType make(cpp21::carg<Handle> handle, cpp21::carg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      auto shared = std::make_shared<DeviceObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<InstanceObj>(handle), cInfo);
      auto wrap = shared->registerSelf();
      wrap->createDefaultMemoryAllocator();
      return wrap;
    };

  };


};

#endif
