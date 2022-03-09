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
  template<typename T>
  inline void atomic_max(std::atomic<T>& maximum_value, T const& value) noexcept
  {
    T prev_value = maximum_value;
    while (prev_value < value && !maximum_value.compare_exchange_weak(prev_value, value)) {}
  };

  // 
  class DeviceObj : public BaseObj {
  public:
    using BaseObj::BaseObj;
    using tType = WrapShared<DeviceObj>;
    using cType = const char const*;
    //using BaseObj;

  protected:
    // 
    //vk::Device device = {};
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<DeviceCreateInfo> cInfo = DeviceCreateInfo{};

    //
    friend InstanceObj;
    friend ResourceObj;
    friend QueueFamilyObj;
    friend DescriptorsObj;
    friend PipelineObj;
    friend UploaderObj;
    friend FramebufferObj;
    friend SwapchainObj;
    friend GeometryLevelObj;
    friend InstanceLevelObj;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

    //
    //std::unordered_map<uintptr_t, std::function<void()>> destMap = {};
    std::array<std::atomic<std::shared_ptr<std::function<void()>>>, 128> destIds = {};
    std::array<std::atomic<std::shared_ptr<std::function<void()>>>, 128> callIds = {};
    std::shared_ptr<std::atomic_int32_t> destructorCount = {};
    std::shared_ptr<std::atomic_int32_t> callbackCount = {};
    std::shared_ptr<std::atomic_bool> threadLocked = {};
    std::shared_ptr<std::atomic_bool> actionLocked = {};

    //
    //std::shared_ptr<InstanceObj> instanceObj = {};

    //
    std::vector<vk::PhysicalDevice> physicalDevices = {};
    cpp21::vector_of_shared<MSS> PDInfoMaps = {};

    //
    std::vector<cType> extensionNames = {};
    std::vector<cType> layerNames = {};

    //
    QueueFamilies queueFamilies = {};

    //
    cpp21::interval_map<uintptr_t, vk::Buffer> addressSpace = {};

    //
    virtual std::tuple<uint32_t, uint32_t> findMemoryTypeAndHeapIndex(cpp21::const_wrap_arg<MemoryRequirements> req = MemoryRequirements{}, cpp21::const_wrap_arg<uintptr_t> physicalDeviceIndex = {}) {
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
        if (req->memoryTypeBits & bitMask) { requiredMemoryTypeIndices.push_back(bitIndex); };
        bitIndex++;
      };

      //uint32_t memoryTypeIndex = 0u;
      //for (decltype(auto) memoryType : memoryTypes) {
      std::tuple<uint32_t, uint32_t> memoryTypeAndHeapIndex = { 0u, 0u };
      for (auto& memoryTypeIndex : requiredMemoryTypeIndices) {
        auto& memoryType = memoryTypes[memoryTypeIndex];
        auto& memoryHeapIndex = memoryType.heapIndex;
        auto requiredBits = vk::MemoryPropertyFlags{};// | vk::MemoryPropertyFlagBits::eDeviceLocal;

        // 
        switch (req->memoryUsage) {
        case (MemoryUsage::eGpuOnly):
          requiredBits |= vk::MemoryPropertyFlagBits::eDeviceLocal;
          break;

        case (MemoryUsage::eCpuToGpu):
          //requiredBits |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostCoherent;
          requiredBits |= vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
          break;

        case (MemoryUsage::eGpuToCpu):
          requiredBits |= vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
          break;

        case (MemoryUsage::eCpuOnly):
          requiredBits |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached | vk::MemoryPropertyFlagBits::eHostCoherent;
          break;

        default:;
          //requiredBits |= vk::MemoryPropertyFlagBits::eDeviceLocal;
        };

        //
        //std::cout << "MemoryTypeIndex: " << uint32_t(memoryTypeIndex) << std::endl;
        //std::cout << "MemoryPropertyFlags: " << uint32_t(memoryType.propertyFlags) << std::endl;
        //std::cout << "RequiredBits: " << uint32_t(requiredBits) << std::endl;

        // 
        if (((memoryType.propertyFlags & requiredBits) == requiredBits) || (!memoryType.propertyFlags && !requiredBits)) {
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

    //
    virtual std::vector<cType>& filterLayers(cpp21::const_wrap_arg<vk::PhysicalDevice> physicalDevice, cpp21::const_wrap_arg<std::vector<std::string>> names) {
      decltype(auto) props = physicalDevice->enumerateDeviceLayerProperties();
      auto& selected = this->layerNames;

      // 
      uintptr_t nameIndex = 0ull;
      for (auto& name : (*names)) {
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
    virtual std::vector<cType>& filterExtensions(cpp21::const_wrap_arg<vk::PhysicalDevice> physicalDevice, cpp21::const_wrap_arg<std::vector<std::string>> names) {
      decltype(auto) props = physicalDevice->enumerateDeviceExtensionProperties(std::string(""));
      auto& selected = (this->extensionNames);

      // 
      uintptr_t nameIndex = 0ull;
      for (auto& name : (*names)) {
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
    virtual std::vector<vk::PhysicalDevice>& filterPhysicalDevices(cpp21::const_wrap_arg<uint32_t> groupIndex = 0u) {
      //this->physicalDevices = {};
      decltype(auto) instanceObj = lxvc::context->get<InstanceObj>(this->base);
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
    virtual std::vector<vk::DeviceQueueCreateInfo>& filterQueueFamilies(cpp21::const_wrap_arg<std::vector<QueueFamilyCreateInfo>> qfInfosIn = {}) {

      // TODO: customize queue priorities
      auto& qfInfosVk = (this->queueFamilies.infos = {});
      auto& qfIndices = (this->queueFamilies.indices = {});
      auto& qfInfoMaps = (this->queueFamilies.infoMaps = {});
      auto& qfCommandPools = (this->queueFamilies.commandPools = {});
      auto& qfQueuesStack = (this->queueFamilies.queues = {});
      for (auto& qfInfoIn : (*qfInfosIn)) {
        qfInfoMaps.push_back(std::make_shared<MSS>(MSS()));
        qfQueuesStack.push_back(std::vector<vk::Queue>{});
        auto& qfInfoMap = qfInfoMaps->back();
        auto qfInfoVk = qfInfoMap->set(vk::StructureType::eDeviceQueueCreateInfo, vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = qfInfoIn.queueFamilyIndex,
        });
        qfIndices.push_back(qfInfoIn.queueFamilyIndex);
        qfInfosVk.push_back(qfInfoVk->setQueuePriorities(*qfInfoIn.queuePriorities));
      };

      // 
      return qfInfosVk;
    };

    //
    virtual std::vector<vk::CommandPool>& createCommandPools(cpp21::const_wrap_arg<std::vector<QueueFamilyCreateInfo>> qfInfosIn = {}) {
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

    //
    virtual FenceType executeCommandOnce(cpp21::const_wrap_arg<CommandOnceSubmission> submissionRef_ = {}) {
      decltype(auto) submissionRef = submissionRef_.optional();
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

      // clean and call events
      this->tickProcessing();

      // 
      auto promise = std::async(std::launch::async | std::launch::deferred, [=,this]() {
        auto result = device.waitForFences(*fence, true, 1000 * 1000 * 1000);
        do { /* but nothing to do */ } while (this->threadLocked->load()); (*this->actionLocked) = true;
        for (auto& fn : submissionRef->onDone) { if ((*callbackCount) < callIds.size()) { callIds[(*callbackCount)++] = std::make_shared<std::function<void()>>(std::bind(fn, result)); }; };
        if ((*destructorCount) < destIds.size()) {
          destIds[(*destructorCount)++] = std::make_shared<std::function<void()>>([=, this]() {
            if (fence && *fence) {
              device.destroyFence(*fence);
              device.freeCommandBuffers(commandPool, commandBuffers);
              *fence = vk::Fence{};
            };
          });
        };
        (*this->actionLocked) = false;
        return result;
      });

      // 
      return std::make_shared<FenceTypeRaw>(std::move(std::make_tuple(std::forward<std::future<vk::Result>>(promise), std::forward<std::shared_ptr<vk::Fence>>(fence))));
    };

    //
    virtual cpp21::interval_map<uintptr_t, vk::Buffer>& getAddressSpace() { return this->addressSpace; };
    virtual cpp21::interval_map<uintptr_t, vk::Buffer> const& getAddressSpace() const { return this->addressSpace; };

    //
    virtual vk::PhysicalDevice& getPhysicalDevice(cpp21::const_wrap_arg<uintptr_t> physicalDeviceIndex = {}) { return this->physicalDevices[std::min(physicalDeviceIndex ? physicalDeviceIndex.value() : this->cInfo->physicalDeviceIndex, this->physicalDevices.size() - 1)]; };
    virtual vk::PhysicalDevice const& getPhysicalDevice(cpp21::const_wrap_arg<uintptr_t> physicalDeviceIndex = {}) const { return this->physicalDevices[std::min(physicalDeviceIndex ? physicalDeviceIndex.value() : this->cInfo->physicalDeviceIndex, this->physicalDevices.size() - 1)]; };

    //
    virtual std::shared_ptr<MSS> getPhysicalDeviceInfoMap(cpp21::const_wrap_arg<uintptr_t> physicalDeviceIndex = {}) { return this->PDInfoMaps[std::min(physicalDeviceIndex ? physicalDeviceIndex.value() : this->cInfo->physicalDeviceIndex, this->PDInfoMaps.size() - 1)]; };
    virtual std::shared_ptr<MSS> getPhysicalDeviceInfoMap(cpp21::const_wrap_arg<uintptr_t> physicalDeviceIndex = {}) const { return this->PDInfoMaps[std::min(physicalDeviceIndex ? physicalDeviceIndex.value() : this->cInfo->physicalDeviceIndex, this->PDInfoMaps.size() - 1)]; };

    // 
    virtual void construct(std::shared_ptr<InstanceObj> instanceObj = {}, cpp21::const_wrap_arg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      //this->instanceObj = instanceObj;
      this->base = instanceObj->handle;
      this->physicalDevices = {};
      this->extensionNames = {};
      this->layerNames = {};
      this->infoMap = std::make_shared<MSS>(MSS());
      if (cInfo) { this->cInfo = cInfo; };

      // 
      this->destructorCount = std::make_shared<std::atomic_int32_t>(std::move(0ull));
      this->callbackCount = std::make_shared<std::atomic_int32_t>(std::move(0ull));
      this->threadLocked = std::make_shared<std::atomic_bool>(std::move(false));
      this->actionLocked = std::make_shared<std::atomic_bool>(std::move(false));

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
        .pNext = nullptr
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
        .pNext = nullptr
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
      };

      // 
      //return SFT();
    };

  public:

    //
    virtual void tickProcessing() {
      if (!actionLocked->load()) {
        *threadLocked = true;
        while ((*callbackCount) > 0) {
          auto callId = callIds[--(*callbackCount)].exchange({}); if (callId) { (*callId)(); };
        }; atomic_max(*callbackCount, 0);
        *threadLocked = false;
      };

      if (!actionLocked->load()) {
        *threadLocked = true;
        while ((*destructorCount) > 0) {
          auto destId = destIds[--(*destructorCount)].exchange({}); if (destId) { (*destId)(); };
        }; atomic_max(*destructorCount, 0);
        *threadLocked = false;
      };
    };

    // TODO: caching...
    virtual vk::Queue const& getQueue(cpp21::const_wrap_arg<QueueGetInfo> info = {}) const {
      //return this->device.getQueue(info->queueFamilyIndex, info->queueIndex);
      decltype(auto) qfIndices = (this->queueFamilies.indices);
      decltype(auto) qfQueuesStack = (this->queueFamilies.queues);
      uintptr_t indexOfQF = std::distance(qfIndices.begin(), std::find(qfIndices.begin(), qfIndices.end(), info->queueFamilyIndex));
      return qfQueuesStack[indexOfQF][info->queueIndex];
    };

    //
    virtual tType writeCopyBuffersCommand(cpp21::const_wrap_arg<CopyBufferWriteInfo> copyInfoRaw);

    //
    virtual FenceType copyBuffersOnce(cpp21::const_wrap_arg<CopyBuffersExecutionOnce> copyInfo) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = copyInfo->submission };
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      
      // 
      submission.commandInits.push_back([=, this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
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
    DeviceObj(std::shared_ptr<InstanceObj> instanceObj = {}, cpp21::const_wrap_arg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) : cInfo(cInfo) {
      this->base = instanceObj->handle;
      this->construct(instanceObj, cInfo);
    };

    // 
    DeviceObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) : cInfo(cInfo) {
      this->construct(lxvc::context->get<InstanceObj>(this->base = handle), cInfo);
    };

    //
    virtual tType registerSelf() {
      lxvc::context->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };
    
    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      auto shared = std::make_shared<DeviceObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  };


};
