#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"
#include "./MemoryAllocatorVma.hpp"
#include "./Descriptors.hpp"

// 
namespace ZNAMED {

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
    virtual std::optional<AllocatedMemory>& allocateMemory(cpp21::const_wrap_arg<MemoryRequirements> requirements) {
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorVma>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocatorVma);
      return memoryAllocatorObj->allocateMemory(requirements, this->allocated, this->extHandle, this->cInfo->extInfoMap, this->mappedMemory, this->destructors);
    };

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:
    // 
    ResourceVma(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : ResourceObj(deviceObj, cInfo) {
      this->base = deviceObj->getHandle();
      this->construct(deviceObj, cInfo);
    };

    // 
    ResourceVma(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : ResourceObj(handle, cInfo) {
      this->construct(ZNAMED::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual WrapShared<ResourceObj> registerSelf() {
      ZNAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return std::dynamic_pointer_cast<ResourceObj>(shared_from_this());
    };

    //
    inline static WrapShared<ResourceObj> make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
      auto shared = std::make_shared<ResourceVma>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  protected:

    // 
    virtual FenceType createImage(cpp21::const_wrap_arg<ImageCreateInfo> cInfo = {}) {
      // 
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
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
      VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
      if (memoryUsage == MemoryUsage::eGpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };
      if (memoryUsage == MemoryUsage::eCpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; };
      if (memoryUsage == MemoryUsage::eCpuToGpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };
      if (memoryUsage == MemoryUsage::eGpuToCpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };

      // 
      VmaAllocationCreateInfo vmaCreateInfo = {
        .flags = (this->memoryUsage != MemoryUsage::eGpuOnly ? VMA_ALLOCATION_CREATE_MAPPED_BIT : VmaAllocationCreateFlags{}) | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = memUsage,
        
      };

      //
      if ((*this->cInfo->extInfoMap)->find(ExtensionInfoName::eMemoryAllocationVma) == (*this->cInfo->extInfoMap)->end()) {
        this->cInfo->extInfoMap->set(ExtensionInfoName::eMemoryAllocationVma, VmaAllocationExtension{});
      };

      //
      decltype(auto) alloc = this->cInfo->extInfoMap->get<VmaAllocationExtension>(ExtensionInfoName::eMemoryAllocationVma);

      //
      vmaCreateImage(memoryAllocatorObj->getHandle().as<VmaAllocator>(), (VkImageCreateInfo*)imageInfo.get(), &vmaCreateInfo, &this->handle.as<VkImage>(), &alloc->allocation, &alloc->allocationInfo);
    };

    // 
    virtual void createBuffer(cpp21::const_wrap_arg<BufferCreateInfo> cInfo = {}) {
      //
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
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
      VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
      if (memoryUsage == MemoryUsage::eGpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE; };
      if (memoryUsage == MemoryUsage::eCpuOnly) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; };
      if (memoryUsage == MemoryUsage::eCpuToGpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; };
      if (memoryUsage == MemoryUsage::eGpuToCpu) { memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST; };

      // 
      VmaAllocationCreateInfo vmaCreateInfo = {
        .flags = (memoryUsage != MemoryUsage::eGpuOnly ? VMA_ALLOCATION_CREATE_MAPPED_BIT : VmaAllocationCreateFlags{}) | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = memUsage,
      };

      //
      if ((*this->cInfo->extInfoMap)->find(ExtensionInfoName::eMemoryAllocationVma) == (*this->cInfo->extInfoMap)->end()) {
        this->cInfo->extInfoMap->set(ExtensionInfoName::eMemoryAllocationVma, VmaAllocationExtension{});
      };

      //
      decltype(auto) alloc = this->cInfo->extInfoMap->get<VmaAllocationExtension>(ExtensionInfoName::eMemoryAllocationVma);

      //
      vmaCreateBuffer(memoryAllocatorObj->getHandle().as<VmaAllocator>(), (VkBufferCreateInfo*)bufferInfo.get(), &vmaCreateInfo, &this->handle.as<VkBuffer>(), &alloc->allocation, &alloc->allocationInfo);
    };


  };
#endif
  
};
