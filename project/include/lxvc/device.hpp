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

  template<typename T>
  inline void atomic_max(std::atomic<T>& maximum_value, T const& value) noexcept
  {
    T prev_value = maximum_value;
    while (prev_value < value &&
      !maximum_value.compare_exchange_weak(prev_value, value))
    {
    }
  }

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
    std::optional<DeviceCreateInfo> cInfo = {};

    //
    friend InstanceObj;
    friend ResourceObj;
    friend QueueFamilyObj;
    friend DescriptorsObj;
    friend PipelineObj;
    friend UploaderObj;
    friend FramebufferObj;
    friend SwapchainObj;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

    //
    //std::unordered_map<uintptr_t, std::function<void()>> destMap = {};
    std::array<std::atomic<std::shared_ptr<std::function<void()>>>, 128> destIds = {};
    std::array<std::atomic<std::shared_ptr<std::function<void()>>>, 128> callIds = {};
    std::shared_ptr<std::atomic_int32_t> destructorCount;
    std::shared_ptr<std::atomic_int32_t> callbackCount;
    std::shared_ptr<std::atomic_bool> threadLocked;
    std::shared_ptr<std::atomic_bool> actionLocked;

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
    virtual std::tuple<uint32_t, uint32_t> findMemoryTypeAndHeapIndex(vk::PhysicalDevice const& physicalDevice, std::optional<MemoryRequirements> req = MemoryRequirements{}) {
      //decltype(auto) physicalDevice = this->physicalDevices[req->physicalDeviceIndex];
      uintptr_t physicalDeviceIndex = std::distance(this->physicalDevices.begin(), std::find(this->physicalDevices.begin(), this->physicalDevices.end(), physicalDevice));
      decltype(auto) PDInfoMap = this->PDInfoMaps[physicalDeviceIndex];
      decltype(auto) memoryProperties2 = PDInfoMap->set(vk::StructureType::ePhysicalDeviceMemoryProperties2, vk::PhysicalDeviceMemoryProperties2{

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
    virtual std::vector<cType>& filterLayers(vk::PhysicalDevice const& physicalDevice, std::vector<std::string> const& names) {
      decltype(auto) props = physicalDevice.enumerateDeviceLayerProperties();
      decltype(auto) selected = opt_ref(this->layerNames);

      // 
      uintptr_t nameIndex = 0ull;
      for (decltype(auto) name : names) {
        uintptr_t propIndex = 0ull;
        for (decltype(auto) prop : props) {
          std::string_view propName = { prop.layerName };
          if (propName.find(name) != std::string::npos) {
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
          if (propName.find(name) != std::string::npos) {
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
      decltype(auto) instanceObj = lxvc::context->get<InstanceObj>(this->base);
      decltype(auto) deviceGroups = instanceObj->enumeratePhysicalDeviceGroups();
      auto& physicalDevices = this->physicalDevices;
      if (deviceGroups.size() > 0) {
        decltype(auto) deviceGroup = deviceGroups[groupIndex];
        vk::PhysicalDevice* PDP = deviceGroup.physicalDevices;
        physicalDevices = std::vector<vk::PhysicalDevice>(PDP, PDP + deviceGroup.physicalDeviceCount);
      } else {
        physicalDevices = instanceObj->enumeratePhysicalDevices();
      };
      for (decltype(auto) PD : physicalDevices) {
        PDInfoMaps->push_back(std::make_shared<MSS>());
      };
      return this->physicalDevices;
    };

    // 
    virtual std::vector<vk::DeviceQueueCreateInfo>& filterQueueFamilies(std::vector<QueueFamilyCreateInfo> const& qfInfosIn = {}) {

      // TODO: customize queue priorities
      decltype(auto) qfInfosVk = cpp21::opt_ref(this->queueFamilies.infos = {});
      decltype(auto) qfIndices = cpp21::opt_ref(this->queueFamilies.indices = {});
      decltype(auto) qfInfoMaps = cpp21::opt_ref(this->queueFamilies.infoMaps = {});
      decltype(auto) qfCommandPools = cpp21::opt_ref(this->queueFamilies.commandPools = {});
      decltype(auto) qfQueuesStack = cpp21::opt_ref(this->queueFamilies.queues = {});
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
      decltype(auto) device = this->handle.as<vk::Device>(); // finally found issue
      if (!!device && this->queueFamilies.commandPools.size() <= 0u) {
        decltype(auto) qfInfosVk = cpp21::opt_ref(this->queueFamilies.infos);
        decltype(auto) qfIndices = cpp21::opt_ref(this->queueFamilies.indices);
        decltype(auto) qfInfoMaps = cpp21::opt_ref(this->queueFamilies.infoMaps);
        decltype(auto) qfCommandPools = cpp21::opt_ref(this->queueFamilies.commandPools);
        decltype(auto) qfQueuesStack = cpp21::opt_ref(this->queueFamilies.queues);
        for (decltype(auto) qfInfoIn : qfInfosIn) {
          uintptr_t indexOfQF = std::distance(qfIndices->begin(), std::find(qfIndices->begin(), qfIndices->end(), qfInfoIn.queueFamilyIndex));
          decltype(auto) qfIndex = cpp21::opt_ref(qfIndices[indexOfQF]);
          decltype(auto) qfInfoMap = qfInfoMaps[indexOfQF];
          decltype(auto) qfQueues = cpp21::opt_ref(qfQueuesStack[indexOfQF]);
          decltype(auto) qfInfoVk = qfInfoMap->get<vk::DeviceQueueCreateInfo>(vk::StructureType::eDeviceQueueCreateInfo);
          decltype(auto) qfCmdPoolInfo = qfInfoMap->set(vk::StructureType::eCommandPoolCreateInfo, vk::CommandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = qfIndex,
          });
          qfCommandPools->push_back(device.createCommandPool(qfCmdPoolInfo.ref()));
          for (decltype(auto) i = 0u; i < qfInfoVk->queueCount; i++) {
            qfQueues->push_back(device.getQueue(qfIndex, i));
          };
          //index++;
        };
      };
      return this->queueFamilies.commandPools;
    };

    //
    virtual FenceType executeCommandOnce(cpp21::optional_ref<CommandOnceSubmission> submissionRef = {}) {
      decltype(auto) device = this->handle.as<vk::Device>();
      std::optional<CommandOnceSubmission> submission = submissionRef;
      decltype(auto) qfIndices = cpp21::opt_ref(this->queueFamilies.indices);
      decltype(auto) qfCommandPools = cpp21::opt_ref(this->queueFamilies.commandPools);
      uintptr_t indexOfQF = std::distance(qfIndices->begin(), std::find(qfIndices->begin(), qfIndices->end(), submissionRef->info->queueFamilyIndex));
      decltype(auto) queue = this->getQueue(submissionRef->info);
      decltype(auto) commandPool = qfCommandPools[indexOfQF];
      decltype(auto) commandBuffers = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = (uint32_t)submission->commandInits.size()
      });

      //
      decltype(auto) cmdInfos = std::vector<vk::CommandBufferSubmitInfo >{};
      decltype(auto) submitInfo = vk::SubmitInfo2{};
      decltype(auto) cIndex = 0u; for (decltype(auto) fn : submission->commandInits) {
        decltype(auto) cmdBuf = commandBuffers[cIndex++];
        cmdBuf.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit, .pInheritanceInfo = cpp21::pointer(submissionRef->inheritanceInfo) });
        decltype(auto) result = fn(cmdBuf);
        cmdInfos.push_back(vk::CommandBufferSubmitInfo{
          .commandBuffer = result ? result : cmdBuf,
          .deviceMask = 0x1
        });
        cmdBuf.end();
      };

      // 
      //decltype(auto) 
      decltype(auto) fence = device.createFence(vk::FenceCreateInfo{ .flags = {} });
      decltype(auto) submits = std::vector<vk::SubmitInfo2>{
        submitInfo.setCommandBufferInfos(cmdInfos).setWaitSemaphoreInfos(*submission->waitSemaphores).setSignalSemaphoreInfos(*submission->signalSemaphores)
      };
      queue.submit2(submits, fence);

      // 
      decltype(auto) promise = std::async(std::launch::async | std::launch::deferred, [=,this]() {
        decltype(auto) result = device.waitForFences(fence, true, 1000 * 1000 * 1000);
        do { /* but nothing to do */ } while (this->threadLocked); (*this->actionLocked) = true;
        for (decltype(auto) fn : submission->onDone) { if ((*callbackCount) < callIds.size()) { callIds[(*callbackCount)++] = std::make_shared<std::function<void()>>(std::bind(fn, result)); }; };
        if ((*destructorCount) < destIds.size()) {
          destIds[(*destructorCount)++] = std::make_shared<std::function<void()>>([=, this]() {
            device.destroyFence(fence);
            device.freeCommandBuffers(commandPool, commandBuffers);
          });
        };
        (*this->actionLocked) = false;
        return result;
      });

      // 
      return std::make_shared<FenceTypeRaw>(std::move(std::make_tuple(std::forward<std::future<vk::Result>>(promise), std::forward<vk::Fence>(fence))));
    };

    // 
    virtual void construct(std::shared_ptr<InstanceObj> instanceObj = {}, std::optional<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      //this->instanceObj = instanceObj;
      this->base = instanceObj->handle;
      this->physicalDevices = {};
      this->extensionNames = {};
      this->layerNames = {};
      this->infoMap = std::make_shared<MSS>();
      this->cInfo = cInfo;

      // 
      this->destructorCount = std::make_shared<std::atomic_int32_t>(std::move(0ull));
      this->callbackCount = std::make_shared<std::atomic_int32_t>(std::move(0ull));
      this->threadLocked = std::make_shared<std::atomic_bool>(std::move(false));
      this->actionLocked = std::make_shared<std::atomic_bool>(std::move(false));

      //memcpy(&this->cInfo, &cInfo, sizeof(DeviceCreateInfo));

      // TODO: get rid from spagetti code or nesting
      decltype(auto) physicalDevices = this->filterPhysicalDevices(this->cInfo->physicalDeviceGroupIndex);
      decltype(auto) PDInfoMap = this->PDInfoMaps[this->cInfo->physicalDeviceIndex];
      decltype(auto) physicalDevice = physicalDevices[this->cInfo->physicalDeviceIndex];
      decltype(auto) deviceGroupInfo = this->infoMap->set(vk::StructureType::eDeviceGroupDeviceCreateInfo, vk::DeviceGroupDeviceCreateInfo{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceFeatures2, vk::PhysicalDeviceFeatures2{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan11Features{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan12Features{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceVulkan13Features, vk::PhysicalDeviceVulkan13Features{
        .pNext = PDInfoMap->set(vk::StructureType::ePhysicalDeviceMultiDrawFeaturesEXT, vk::PhysicalDeviceMultiDrawFeaturesEXT{
        .pNext = nullptr
        })
        })
        })
        })
        })
      });

      // 
      decltype(auto) deviceInfo = infoMap->set(vk::StructureType::eDeviceCreateInfo, vk::DeviceCreateInfo{ .pNext = deviceGroupInfo });
      
      //
      if (!!physicalDevice) {
        physicalDevice.getFeatures2(PDInfoMap->get<vk::PhysicalDeviceFeatures2>(vk::StructureType::ePhysicalDeviceFeatures2).get());
        deviceGroupInfo->setPhysicalDevices(physicalDevices);

        // 
        deviceInfo->setQueueCreateInfos(this->filterQueueFamilies(this->cInfo->queueFamilyInfos));
        deviceInfo->setPEnabledExtensionNames(this->filterExtensions(physicalDevice, this->cInfo->extensionList));
        deviceInfo->setPEnabledLayerNames(this->filterLayers(physicalDevice, this->cInfo->layerList));

        // 
        this->handle = physicalDevice.createDevice(deviceInfo);
        //VULKAN_HPP_DEFAULT_DISPATCHER.init(this->handle.as<vk::Device>());
        this->dispatch = vk::DispatchLoaderDynamic(this->base.as<vk::Instance>(), vkGetInstanceProcAddr, this->handle.as<vk::Device>(), vkGetDeviceProcAddr);
        this->createCommandPools(this->cInfo->queueFamilyInfos);
        
      }
      else {
        std::cerr << "Physical Device Not Detected" << std::endl;
      };

      // 
      //return SFT();
    };

  public:

    //
    virtual void tickProcessing() {
      if (!(*actionLocked)) {
        *threadLocked = true;
        do {
          auto destId = destIds[--(*destructorCount)].exchange({}); if (destId) { (*destId)(); };
        } while ((*destructorCount) > 0); atomic_max(*destructorCount, 0);
        do {
          auto callId = callIds[--(*callbackCount)].exchange({}); if (callId) { (*callId)(); };
        } while ((*callbackCount) > 0); atomic_max(*callbackCount, 0);
        *threadLocked = false;
      };
    };

    // TODO: caching...
    virtual vk::Queue const& getQueue(cpp21::optional_ref<QueueGetInfo> info = {}) const {
      //return this->device.getQueue(info->queueFamilyIndex, info->queueIndex);
      decltype(auto) qfIndices = cpp21::opt_ref(this->queueFamilies.indices);
      decltype(auto) qfQueuesStack = cpp21::opt_ref(this->queueFamilies.queues);
      uintptr_t indexOfQF = std::distance(qfIndices->begin(), std::find(qfIndices->begin(), qfIndices->end(), info->queueFamilyIndex));
      return qfQueuesStack[indexOfQF][info->queueIndex];
    };

    //
    virtual FenceType executeCopyBuffersOnce(cpp21::optional_ref<CopyBufferInfo> copyInfoRaw);

    //
    ~DeviceObj() {
      this->tickProcessing();
    };

    // 
    DeviceObj(std::shared_ptr<InstanceObj> instanceObj = {}, std::optional<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) : cInfo(cInfo) {
      this->base = instanceObj->handle;
      this->construct(instanceObj, cInfo);
    };

    // 
    DeviceObj(Handle const& handle, std::optional<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(Handle const& handle, std::optional<DeviceCreateInfo> cInfo = DeviceCreateInfo{}) {
      auto shared = std::make_shared<DeviceObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  };


};
