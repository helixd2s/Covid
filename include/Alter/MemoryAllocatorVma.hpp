#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"

//
#ifdef ALT_ENABLE_VMA
#include <vk_mem_alloc.h>
#endif

// 
namespace ANAMED {

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
  //
  struct VmaAllocatorExtension {
    VmaAllocator allocator = nullptr;
    VmaAllocatorInfo allocatorInfo = {};
  };

  //
  struct VmaAllocationExtension {
    VmaAllocation allocation = nullptr;
    VmaAllocationInfo allocationInfo = {};
  };

  // 
  class MemoryAllocatorVma : public MemoryAllocatorObj {
  public:
    using MemoryAllocatorObj::MemoryAllocatorObj;
    using tType = WrapShared<MemoryAllocatorVma>;

  protected:
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{};
    VmaPool exportPool = {};

  protected:

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    // 
    void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) override {
      if (!this->cInfo->extInfoMap) {
        this->cInfo->extInfoMap = std::make_shared<EXIF>();
      };

      //
      if (this->cInfo->extInfoMap) {
        if ((*this->cInfo->extInfoMap)->find(ExtensionInfoName::eMemoryAllocatorVma) == (*this->cInfo->extInfoMap)->end()) {
          this->cInfo->extInfoMap->set(ExtensionInfoName::eMemoryAllocatorVma, VmaAllocatorExtension{});
        };
      };

      //
      decltype(auto) alloc = this->cInfo->extInfoMap ? this->cInfo->extInfoMap->get<VmaAllocatorExtension>(ExtensionInfoName::eMemoryAllocatorVma) : cpp21::obj<VmaAllocatorExtension>(VmaAllocatorExtension{});

      // 
      VmaVulkanFunctions vulkanFunctions = {};
      vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
      vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

      // 
      VmaAllocatorCreateInfo vmaCreateInfo = {};
      vmaCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
      vmaCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
      vmaCreateInfo.physicalDevice = deviceObj->getPhysicalDevice();
      vmaCreateInfo.device = deviceObj->getHandle().as<VkDevice>();
      vmaCreateInfo.instance = deviceObj->getBase().as<VkInstance>();

      // 
      vmaCreateAllocator(&vmaCreateInfo, &this->handle.as<VmaAllocator>());
      //this->handle.type = HandleType::eMemoryAllocator; // Unable to Map without specific type
      this->handle.type = HandleType::eExtension;
      //this->handle = Handle(uintptr_t(this), HandleType::eMemoryAllocator);

      //
      decltype(auto) exportMemory = infoMap->set(vk::StructureType::eExportMemoryAllocateInfo, vk::ExportMemoryAllocateInfo{
        .handleTypes = extMemFlags
      });

      //
      VmaPoolCreateInfo vmaPoolCreateInfo = {};
      vmaPoolCreateInfo.minBlockCount = 0;
      vmaPoolCreateInfo.maxBlockCount = 0;
      vmaPoolCreateInfo.minAllocationAlignment = 0;
      vmaPoolCreateInfo.blockSize = 0;
      vmaPoolCreateInfo.pMemoryAllocateNext = exportMemory.get();

      //
      vmaCreatePool(this->handle.as<VmaAllocator>(), &vmaPoolCreateInfo, &exportPool);
    };

  public:

    //
    VmaPool& getExportPool() { return exportPool; };
    VmaPool const& getExportPool() const { return exportPool; };

    //
    ~MemoryAllocatorVma() {
      this->tickProcessing();
    };

    // 
    MemoryAllocatorVma(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : MemoryAllocatorObj(deviceObj, cInfo)  {
      //this->construct(deviceObj, cInfo);
    };

    // 
    MemoryAllocatorVma(Handle const& handle, cpp21::optional_ref<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : MemoryAllocatorObj(handle, cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    //
     WrapShared<MemoryAllocatorObj> registerSelf() override {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      deviceObj->registerExt(ExtensionName::eMemoryAllocatorVma, shared_from_this());
      return std::dynamic_pointer_cast<MemoryAllocatorObj>(shared_from_this());
    };

    // 
     std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    inline static WrapShared<MemoryAllocatorVma> make(Handle const& handle, cpp21::optional_ref<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) {
      auto shared = std::make_shared<MemoryAllocatorVma>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
      return std::dynamic_pointer_cast<MemoryAllocatorVma>(shared->registerSelf().shared());
    };

  public:

    //
    std::shared_ptr<AllocatedMemory> handleVmaAllocation(cpp21::optional_ref<MemoryRequirements> requirements, std::shared_ptr<AllocatedMemory> allocated, VmaAllocator const& allocator, VmaAllocation const& allocation, VmaAllocationInfo const& allocationInfo = {}) {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      auto& device = this->base.as<vk::Device>();
      
      //
      *allocated = AllocatedMemory{ allocationInfo.deviceMemory, allocationInfo.offset, allocationInfo.size };
      allocated->allocation = uintptr_t(allocation);

      //
      if (requirements->memoryUsage == MemoryUsage::eGpuOnly) {
#ifdef _WIN32
        allocated->extHandle = device.getMemoryWin32HandleKHR(vk::MemoryGetWin32HandleInfoKHR{ .memory = allocated->memory, .handleType = extMemFlagBits }, deviceObj->getDispatch());
#else
#ifdef __linux__ 
        allocated->extHandle = device.getMemoryFdKHR(vk::MemoryGetFdInfoKHR{ .memory = allocated->memory, .handleType = extMemFlagBits }, deviceObj->getDispatch());
#endif
#endif
      };

      //
      device.setMemoryPriorityEXT(allocated->memory, 1.f, deviceObj->getDispatch());

      //
      allocated->destructor = std::make_shared<std::function<DFun>>([device, allocator = this->handle.as<VmaAllocator>(), &allocation = allocated->allocation](BaseObj const*) {
        //device.waitIdle();
        if (allocation) {
          vmaFreeMemory(allocator, reinterpret_cast<VmaAllocation&>(allocation)); allocation = {};
        };
      });

      // 
      allocated->mapped = allocationInfo.pMappedData;
      return allocated;
    };

    //
    virtual vk::Buffer createBufferAndAllocateMemory(std::shared_ptr<AllocatedMemory>& allocated, MemoryUsage const& memoryUsage, std::shared_ptr<MSS> infoMap, std::vector<std::shared_ptr<std::function<DFun>>>& destructors) override {
      
      //
      decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryBufferCreateInfo, vk::ExternalMemoryBufferCreateInfo{
        .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
      });

      //
      decltype(auto) bufferInfo = infoMap->get<vk::BufferCreateInfo>(vk::StructureType::eBufferCreateInfo);
      decltype(auto) bufferUsage = bufferInfo->usage;

      //
      VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
      if (memoryUsage == MemoryUsage::eGpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };
      if (memoryUsage == MemoryUsage::eCpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; };
      if (memoryUsage == MemoryUsage::eCpuToGpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; };
      if (memoryUsage == MemoryUsage::eGpuToCpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; };

      // 
      VmaAllocationCreateInfo vmaCreateInfo = {
        .flags = (memoryUsage != MemoryUsage::eGpuOnly ? VMA_ALLOCATION_CREATE_MAPPED_BIT : VmaAllocationCreateFlags{}) | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = memUsage,
        .pool = memoryUsage == MemoryUsage::eGpuOnly ? this->getExportPool() : VmaPool{}
      };

      //
      decltype(auto) allocator = this->getHandle().as<VmaAllocator>();

      //
      vk::Buffer buffer = {};
      VmaAllocation allocation = {};
      VmaAllocationInfo allocationInfo = {};
      vmaCreateBuffer(allocator, (VkBufferCreateInfo*)bufferInfo.get(), &vmaCreateInfo, (VkBuffer*)&buffer, &allocation, &allocationInfo);
      this->handle.type = HandleType::eBuffer;

      //
      auto& device = this->base.as<vk::Device>();
      decltype(auto) memReqInfo2 = vk::MemoryRequirements2{};
      vk::DeviceBufferMemoryRequirements memReqIn = vk::DeviceBufferMemoryRequirements{ .pCreateInfo = bufferInfo.get() };
      device.getBufferMemoryRequirements(&memReqIn, &memReqInfo2);

      //
      allocated = this->handleVmaAllocation(MemoryRequirements{
        .memoryUsage = memoryUsage,
        .requirements = memReqInfo2.memoryRequirements,
        .dedicated = DedicatedMemory{ .buffer = buffer },
      }, allocated, allocator, allocation, allocationInfo);

      //
      return buffer;
    };

    //
    virtual vk::Image createImageAndAllocateMemory(std::shared_ptr<AllocatedMemory>& allocated, MemoryUsage const& memoryUsage, std::shared_ptr<MSS> infoMap, std::vector<std::shared_ptr<std::function<DFun>>>& destructors) override {
      
      //
      decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryImageCreateInfo, vk::ExternalMemoryImageCreateInfo{
        .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
      });

      //
      decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
      decltype(auto) imageUsage = imageInfo->usage;

      //
      VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
      if (memoryUsage == MemoryUsage::eGpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };
      if (memoryUsage == MemoryUsage::eCpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; };
      if (memoryUsage == MemoryUsage::eCpuToGpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };
      if (memoryUsage == MemoryUsage::eGpuToCpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };

      // 
      VmaAllocationCreateInfo vmaCreateInfo = {
        .flags = (memoryUsage != MemoryUsage::eGpuOnly ? VMA_ALLOCATION_CREATE_MAPPED_BIT : VmaAllocationCreateFlags{}) | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = memUsage,
        .pool = memoryUsage == MemoryUsage::eGpuOnly ? this->getExportPool() : VmaPool{}
      };

      //
      decltype(auto) allocator = this->getHandle().as<VmaAllocator>();

      //
      vk::Image image = {};
      VmaAllocation allocation = {};
      VmaAllocationInfo allocationInfo = {};
      vmaCreateImage(allocator, (VkImageCreateInfo*)imageInfo.get(), &vmaCreateInfo, (VkImage*)&image, &allocation, &allocationInfo);
      this->handle.type = HandleType::eImage;

      //
      auto& device = this->base.as<vk::Device>();
      decltype(auto) memReqInfo2 = vk::MemoryRequirements2{};
      vk::DeviceImageMemoryRequirements memReqIn = vk::DeviceImageMemoryRequirements{ .pCreateInfo = imageInfo.get() };
      device.getImageMemoryRequirements(&memReqIn, &memReqInfo2);

      //
      allocated = this->handleVmaAllocation(MemoryRequirements{
        .memoryUsage = memoryUsage,
        .requirements = memReqInfo2.memoryRequirements,
        .dedicated = DedicatedMemory{.image = image },
      }, allocated, allocator, allocation, allocationInfo);

      //
      return image;
    };


    //
    std::shared_ptr<AllocatedMemory> allocateMemory(std::shared_ptr<AllocatedMemory>& allocated, cpp21::optional_ref<MemoryRequirements> requirements, std::shared_ptr<MSS> infoMap) override {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      auto& device = this->base.as<vk::Device>();
      auto& physicalDevice = deviceObj->getPhysicalDevice();
      auto PDInfoMap = deviceObj->getPhysicalDeviceInfoMap();
      auto memTypeHeap = deviceObj->findMemoryTypeAndHeapIndex(*requirements);

      //
      VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
      if (requirements->memoryUsage == MemoryUsage::eGpuOnly) { memUsage = VMA_MEMORY_USAGE_GPU_ONLY; };
      if (requirements->memoryUsage == MemoryUsage::eCpuOnly) { memUsage = VMA_MEMORY_USAGE_CPU_ONLY; };
      if (requirements->memoryUsage == MemoryUsage::eCpuToGpu) { memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU; };
      if (requirements->memoryUsage == MemoryUsage::eGpuToCpu) { memUsage = VMA_MEMORY_USAGE_GPU_TO_CPU; };

      // 
      VmaAllocationCreateInfo vmaCreateInfo = {
        .flags = (requirements->memoryUsage != MemoryUsage::eGpuOnly ? VMA_ALLOCATION_CREATE_MAPPED_BIT : VmaAllocationCreateFlags{}) | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = memUsage,
        .pool = requirements->memoryUsage == MemoryUsage::eGpuOnly ? this->getExportPool() : VmaPool{}
      };

      //
      VmaAllocation allocation = {};
      VmaAllocationInfo allocationInfo = {};

      //
      decltype(auto) allocator = this->getHandle().as<VmaAllocator>();

      // 
      if (requirements->dedicated) {
        if (requirements->dedicated->buffer) { vmaAllocateMemoryForBuffer(allocator, requirements->dedicated->buffer, &vmaCreateInfo, &allocation, &allocationInfo); };
        if (requirements->dedicated->image) { vmaAllocateMemoryForImage(allocator, requirements->dedicated->image, &vmaCreateInfo, &allocation, &allocationInfo); };
      } else {
        vmaAllocateMemory(allocator, (VkMemoryRequirements*)&requirements->requirements, &vmaCreateInfo, &allocation, &allocationInfo);
      };
      
      // 
      return this->handleVmaAllocation(requirements, allocated, allocator, allocation, allocationInfo);
    };

  };
#endif

};
#endif
