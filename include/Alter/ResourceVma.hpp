#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Device.hpp"
#include "./MemoryAllocatorVma.hpp"
#include "./PipelineLayout.hpp"

// 
namespace ANAMED {

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
  

  // 
  class ResourceVma : public ResourceObj {
  public: 
    using tType = WrapShared<ResourceVma>;
    using ResourceObj::ResourceObj;

    // 
    friend MemoryAllocatorVma;

  protected:

    //
    virtual std::shared_ptr<AllocatedMemory> allocateMemory(cpp21::optional_ref<MemoryRequirements> requirements) {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorVma>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocatorVma);
      return memoryAllocatorObj->allocateMemory(requirements, this->allocated = std::make_shared<AllocatedMemory>());
    };

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:
    // 
    ResourceVma(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : ResourceObj(deviceObj, cInfo) {
    };

    // 
    ResourceVma(Handle const& handle, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : ResourceObj(handle, cInfo) {
    };

    // 
    std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    WrapShared<ResourceObj> registerSelf() override {
      ANAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return std::dynamic_pointer_cast<ResourceObj>(shared_from_this());
    };

    //
    inline static tType make(Handle const& handle, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
      auto shared = std::make_shared<ResourceVma>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
      return std::dynamic_pointer_cast<ResourceVma>(shared->registerSelf().shared());
    };

  protected:

    // 
     FenceType createImage(cpp21::optional_ref<ImageCreateInfo> cInfo = {}) override {
      // 
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorVma>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocatorVma);

      //
      decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryImageCreateInfo, vk::ExternalMemoryImageCreateInfo{
        .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
      });

      // 
      decltype(auto) imageUsage = this->handleImageUsage(cInfo->type);
      decltype(auto) imageInfo = infoMap->set(vk::StructureType::eImageCreateInfo, vk::ImageCreateInfo{
        .pNext = memoryUsage == MemoryUsage::eGpuOnly ? externalInfo.get() : nullptr,
        .flags = cInfo->flags,
        .imageType = cInfo->imageType,
        .format = cInfo->format,
        .extent = cInfo->extent,
        .mipLevels = cInfo->mipLevelCount,
        .arrayLayers = cInfo->layerCount, // TODO: correct array layers
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = imageUsage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
      });

      //
      imageInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices);

      //
      VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
      if (memoryUsage == MemoryUsage::eGpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };
      if (memoryUsage == MemoryUsage::eCpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; };
      if (memoryUsage == MemoryUsage::eCpuToGpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };
      if (memoryUsage == MemoryUsage::eGpuToCpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };

      // 
      VmaAllocationCreateInfo vmaCreateInfo = {
        .flags = (this->memoryUsage != MemoryUsage::eGpuOnly ? VMA_ALLOCATION_CREATE_MAPPED_BIT : VmaAllocationCreateFlags{}) | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = memUsage,
        .pool = memoryUsage == MemoryUsage::eGpuOnly ? memoryAllocatorObj->getExportPool() : VmaPool{}
      };

      //
      decltype(auto) allocator = memoryAllocatorObj->getHandle().as<VmaAllocator>();

      //
      VmaAllocation allocation = {};
      VmaAllocationInfo allocationInfo = {};
      vmaCreateImage(allocator, (VkImageCreateInfo*)imageInfo.get(), &vmaCreateInfo, &this->handle.as<VkImage>(), &allocation, &allocationInfo);
      this->handle.type = HandleType::eImage;

      //
      decltype(auto) memReqInfo2 = vk::MemoryRequirements2{};
      vk::DeviceImageMemoryRequirements memReqIn = vk::DeviceImageMemoryRequirements{ .pCreateInfo = imageInfo.get() };
      device.getImageMemoryRequirements(&memReqIn, &memReqInfo2);

      //
      this->allocated = memoryAllocatorObj->handleVmaAllocation(this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .requirements = memReqInfo2.memoryRequirements,
        .dedicated = DedicatedMemory{.image = cInfo->type != ImageType::eSwapchain ? this->handle.as<vk::Image>() : vk::Image{} },
      }, this->allocated = std::make_shared<AllocatedMemory>(), allocator, allocation, allocationInfo);

      // 
      if (cInfo->info) {
        return this->executeSwitchLayoutOnce(ImageLayoutSwitchInfo{
          .info = cInfo->info,
          .switchInfo = ImageLayoutSwitchWriteInfo{
            .newImageLayout = cInfo->layout,
            .oldImageLayout = imageInfo->initialLayout,
          },
          });
      };

      //
      this->destructors.push_back(std::make_shared<std::function<DFun>>([device, image = this->handle.as<vk::Image>(), type = cInfo->type, allocator, allocation](BaseObj const*) {
        if (type != ImageType::eSwapchain) {
          //device.waitIdle();
          vmaDestroyImage(allocator, image, allocation);
        };
      }));

      //
      return FenceType{};
    };

    // 
     FenceType createBuffer(cpp21::optional_ref<BufferCreateInfo> cInfo = {}) override {
      //
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorVma>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocatorVma);

      //
      decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryBufferCreateInfo, vk::ExternalMemoryBufferCreateInfo{
        .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
      });

      //
      decltype(auto) bufferUsage = this->handleBufferUsage(cInfo->type);
      decltype(auto) bufferInfo = infoMap->set(vk::StructureType::eBufferCreateInfo, vk::BufferCreateInfo{
        .pNext = memoryUsage == MemoryUsage::eGpuOnly ? externalInfo.get() : nullptr,
        .size = cInfo->size,
        .usage = bufferUsage,
        .sharingMode = vk::SharingMode::eExclusive
      });

      //
      bufferInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices);

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
        .pool = memoryUsage == MemoryUsage::eGpuOnly ? memoryAllocatorObj->getExportPool() : VmaPool{}
      };

      //
      decltype(auto) allocator = memoryAllocatorObj->getHandle().as<VmaAllocator>();

      //
      VmaAllocation allocation = {};
      VmaAllocationInfo allocationInfo = {};
      vmaCreateBuffer(allocator, (VkBufferCreateInfo*)bufferInfo.get(), &vmaCreateInfo, &this->handle.as<VkBuffer>(), &allocation, &allocationInfo);
      this->handle.type = HandleType::eBuffer;

      //
      decltype(auto) memReqInfo2 = vk::MemoryRequirements2{};
      vk::DeviceBufferMemoryRequirements memReqIn = vk::DeviceBufferMemoryRequirements{ .pCreateInfo = bufferInfo.get() };
      device.getBufferMemoryRequirements(&memReqIn, &memReqInfo2);

      //
      this->allocated = memoryAllocatorObj->handleVmaAllocation(this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .requirements = memReqInfo2.memoryRequirements,
        .dedicated = DedicatedMemory{ .buffer = this->handle.as<vk::Buffer>() },
      }, this->allocated = std::make_shared<AllocatedMemory>(), allocator, allocation, allocationInfo);

      //
      if (bufferUsage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
        this->deviceAddress = device.getBufferAddress(vk::BufferDeviceAddressInfo{
          .buffer = this->handle.as<vk::Buffer>()
        });
        deviceObj->getAddressSpace().insert({ this->deviceAddress, this->deviceAddress + cInfo->size }, this->handle.as<vk::Buffer>());
      };

      //
      this->destructors.push_back(std::make_shared<std::function<DFun>>([device, buffer = this->handle.as<vk::Buffer>(), type = cInfo->type, allocator, allocation](BaseObj const*) {
        //device.waitIdle();
        vmaDestroyBuffer(allocator, buffer, allocation);
      }));

      //
      decltype(auto) submission = CommandOnceSubmission{
        .submission = SubmissionInfo{.info = cInfo->info ? cInfo->info.value() : QueueGetInfo{0u, 0u}}
      };

      // 
      return this->executeFillBuffer(submission);
    };


  };
#endif
  
};
#endif
