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

  protected:

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    // 
     void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) override {
      //
       if (this->cInfo->extInfoMap) {
         if ((*this->cInfo->extInfoMap)->find(ExtensionInfoName::eMemoryAllocatorVma) != (*this->cInfo->extInfoMap)->end()) {
           this->cInfo->extInfoMap->set(ExtensionInfoName::eMemoryAllocatorVma, VmaAllocatorExtension{});
         };
       };

      //
       decltype(auto) alloc = this->cInfo->extInfoMap ? this->cInfo->extInfoMap->get<VmaAllocatorExtension>(ExtensionInfoName::eMemoryAllocatorVma) : VmaAllocatorExtension{};

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
    };

  public:

    //
    ~MemoryAllocatorVma() {
      this->tickProcessing();
    };

    // 
    MemoryAllocatorVma(WrapShared<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : MemoryAllocatorObj(deviceObj, cInfo)  {
      //this->construct(deviceObj, cInfo);
    };

    // 
    MemoryAllocatorVma(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : MemoryAllocatorObj(handle, cInfo) {
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
    inline static WrapShared<MemoryAllocatorObj> make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) {
      auto shared = std::make_shared<MemoryAllocatorVma>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  public:

    //
     std::optional<AllocatedMemory>& allocateMemory(cpp21::const_wrap_arg<MemoryRequirements> requirements, std::optional<AllocatedMemory>& allocated, ExtHandle& extHandle, std::shared_ptr<EXIF> extInfoMap, void*& mapped, std::vector<std::function<void(BaseObj const*)>>& destructors) override {
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
      };

      //
      if ((*this->cInfo->extInfoMap)->find(ExtensionInfoName::eMemoryAllocationVma) == (*this->cInfo->extInfoMap)->end()) {
        this->cInfo->extInfoMap->set(ExtensionInfoName::eMemoryAllocationVma, VmaAllocationExtension{});
      };

      //
      decltype(auto) vmaAllocExt = this->cInfo->extInfoMap->get<VmaAllocationExtension>(ExtensionInfoName::eMemoryAllocationVma);

      // 
      if (requirements->dedicated) {
        if (requirements->dedicated->buffer) { vmaAllocateMemoryForBuffer(this->handle.as<VmaAllocator>(), requirements->dedicated->buffer, &vmaCreateInfo, &vmaAllocExt->allocation, &vmaAllocExt->allocationInfo); };
        if (requirements->dedicated->image) { vmaAllocateMemoryForImage(this->handle.as<VmaAllocator>(), requirements->dedicated->image, &vmaCreateInfo, &vmaAllocExt->allocation, &vmaAllocExt->allocationInfo); };
      } else {
        vmaAllocateMemory(this->handle.as<VmaAllocator>(), (VkMemoryRequirements*)&requirements->requirements, &vmaCreateInfo, &vmaAllocExt->allocation, &vmaAllocExt->allocationInfo);
      };

      //
      if (requirements->needsDestructor) {
        destructors.push_back([device, allocator=this->handle.as<VmaAllocator>(), allocation=vmaAllocExt->allocation](BaseObj const*) {
          device.waitIdle();
          vmaFreeMemory(allocator, allocation);
        });
      };

      // 
      mapped = vmaAllocExt->allocationInfo.pMappedData;

      // 
      return (allocated = AllocatedMemory{ vmaAllocExt->allocationInfo.deviceMemory, vmaAllocExt->allocationInfo.offset, vmaAllocExt->allocationInfo.size });
    };

  };
#endif

};
#endif
