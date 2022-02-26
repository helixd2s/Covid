#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {

  // 
  class ResourceObj : std::enable_shared_from_this<ResourceObj> {
  protected: 
    using tType = std::shared_ptr<ResourceObj>;
    friend DeviceObj;
    friend DescriptorsObj;

    // 
    vk::Buffer buffer = {};
    vk::Image image = {};
    vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;

    // 
    std::optional<AllocatedMemory> allocated = {};
    std::optional<ResourceCreateInfo> cInfo = {};
    std::optional<MemoryRequirements> mReqs = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

  public:
    // 
    ResourceObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

  protected:

    //
    virtual std::optional<AllocatedMemory>& allocateMemory(cpp21::optional_ref<MemoryRequirements> requirements) {
      decltype(auto) physicalDevice = this->deviceObj->physicalDevices[this->deviceObj->cInfo->physicalDeviceIndex];
      decltype(auto) memTypeHeap = this->deviceObj->findMemoryTypeAndHeapIndex(physicalDevice, *requirements);
      decltype(auto) allocated = this->allocated;
      decltype(auto) device = this->deviceObj->device;

      // 
      allocated = AllocatedMemory{
        .memory = device.allocateMemory(infoMap->set(vk::StructureType::eMemoryAllocateInfo, vk::MemoryAllocateInfo{
          .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedAllocateInfo, vk::MemoryDedicatedAllocateInfo{ 
            .image = requirements->dedicated ? this->image : vk::Image{},
            .buffer = requirements->dedicated ? this->buffer : vk::Buffer{}
          }),
          .allocationSize = requirements->size,
          .memoryTypeIndex = std::get<0>(memTypeHeap)
        })),
        .offset = 0ull,
        .size = requirements->size
      };

      // 
      return allocated;
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();
      if (this->cInfo->imageInfo) { this->createImage(this->cInfo->imageInfo.value()); };
      if (this->cInfo->bufferInfo) { this->createBuffer(this->cInfo->bufferInfo.value()); };
    };

    // 
    virtual tType createImage(cpp21::optional_ref<ImageCreateInfo> cInfo = {}) {
      // 
      decltype(auto) device = this->deviceObj->device;
      decltype(auto) imageUsage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
      decltype(auto) memoryUsage = MemoryUsage::eGpuOnly;

      // 
      switch (cInfo->type) {
      case ImageType::eStorage:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
        break;

      case ImageType::eTexture:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eSampled;
        break;

      case ImageType::eColorAttachment:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
        break;

      case ImageType::eDepthAttachment:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
        break;

      case ImageType::eStencilAttachment:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
        break;

      default:;
      };

      // 
      decltype(auto) imageInfo = infoMap->set(vk::StructureType::eImageCreateInfo, vk::ImageCreateInfo{
        .format = cInfo->format,
        .extent = cInfo->extent,
        .usage = imageUsage
      });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{})
      });

      //
      device.getImageMemoryRequirements2(infoMap->set(vk::StructureType::eImageMemoryRequirementsInfo2, vk::ImageMemoryRequirementsInfo2{
        .image = (this->image = device.createImage(imageInfo->setQueueFamilyIndices(this->deviceObj->queueFamilies.indices)))
      }).get(), memReqInfo2.get());

      // 
      decltype(auto) memReqInfo = memReqInfo2->memoryRequirements;
      this->allocated = this->allocateMemory(opt_ref((this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .memoryTypeBits = memReqInfo.memoryTypeBits,
        .size = memReqInfo.size
      }).value()));

      //
      std::vector<vk::BindImageMemoryInfo> bindInfos = { *infoMap->set(vk::StructureType::eBindImageMemoryInfo, vk::BindImageMemoryInfo{
        .image = this->image, .memory = this->allocated->memory, .memoryOffset = this->allocated->offset
      }) };
      device.bindImageMemory2(bindInfos);

      // 
      return this->SFT();
    };

    // 
    virtual tType createBuffer(cpp21::optional_ref<BufferCreateInfo> cInfo = {}) {
      decltype(auto) device = this->deviceObj->device;
      decltype(auto) bufferUsage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
      decltype(auto) memoryUsage = MemoryUsage::eGpuOnly;

      // 
      switch (cInfo->type) {
      case BufferType::eDevice:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |=
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT |
          vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT |
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
          vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
          vk::BufferUsageFlagBits::eShaderBindingTableKHR |
          vk::BufferUsageFlagBits::eStorageTexelBuffer;
        break;

      case BufferType::eHostMap:
        memoryUsage = MemoryUsage::eCpuToGpu;
        bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eUniformTexelBuffer;
        break;

      case BufferType::eUniform:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eUniformTexelBuffer;
        break;

      default:;
      };

      // 
      decltype(auto) bufferInfo = infoMap->set(vk::StructureType::eBufferCreateInfo, vk::BufferCreateInfo{
        .size = cInfo->size,
        .usage = bufferUsage
      });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{})
      });

      //
      device.getBufferMemoryRequirements2(infoMap->set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{
        .buffer = (this->buffer = device.createBuffer(bufferInfo->setQueueFamilyIndices(this->deviceObj->queueFamilies.indices)))
      }).get(), memReqInfo2.get());

      // 
      decltype(auto) memReqInfo = memReqInfo2->memoryRequirements;
      this->allocated = this->allocateMemory(opt_ref((this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .memoryTypeBits = memReqInfo.memoryTypeBits,
        .size = memReqInfo.size
      }).value()));

      //
      std::vector<vk::BindBufferMemoryInfo> bindInfos = { *infoMap->set(vk::StructureType::eBindBufferMemoryInfo, vk::BindBufferMemoryInfo{
        .buffer = this->buffer, .memory = this->allocated->memory, .memoryOffset = this->allocated->offset
      }) };
      device.bindBufferMemory2(bindInfos);

      // 
      return this->SFT();
    };

  };
  
};
