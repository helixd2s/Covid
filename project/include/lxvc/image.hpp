#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {

  // 
  class ImageObj : std::enable_shared_from_this<ImageObj> {
  public:
    using tType = std::shared_ptr<ImageObj>;
    friend DeviceObj;

    // 
    vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
    vk::Image image = {};
    std::optional<AllocatedMemory> allocated = {};
    std::optional<ImageCreateInfo> cInfo = {};
    std::optional<MemoryRequirements> mReqs = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    ImageObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<ImageCreateInfo> cInfo = ImageCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    //
    virtual std::optional<AllocatedMemory>& allocateMemory(cpp21::optional_ref<MemoryRequirements> requirements) {
      decltype(auto) physicalDevice = this->deviceObj->physicalDevices[this->deviceObj->cInfo->physicalDeviceIndex];
      decltype(auto) memTypeHeap = this->deviceObj->findMemoryTypeAndHeapIndex(physicalDevice, *requirements);
      decltype(auto) allocated = this->allocated;

      // 
      allocated = AllocatedMemory{
        .memory = this->deviceObj->device.allocateMemory(infoMap->set(vk::StructureType::eMemoryAllocateInfo, vk::MemoryAllocateInfo{
          .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedAllocateInfo, vk::MemoryDedicatedAllocateInfo{.image = requirements->dedicated ? this->image : vk::Image{} }),
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
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<ImageCreateInfo> cInfo = ImageCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();

      // 
      auto imageUsage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
      auto memoryUsage = MemoryUsage::eGpuOnly;

      // 
      switch (this->cInfo->type) {
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
        .format = this->cInfo->format,
        .extent = this->cInfo->extent,
        .usage = imageUsage
      });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{})
      });

      //
      this->deviceObj->device.getImageMemoryRequirements2(infoMap->set(vk::StructureType::eImageMemoryRequirementsInfo2, vk::ImageMemoryRequirementsInfo2{
        .image = (this->image = this->deviceObj->device.createImage(imageInfo->setQueueFamilyIndices(this->deviceObj->queueFamilies.indices)))
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
      this->deviceObj->device.bindImageMemory2(bindInfos);

      // 
      return this->SFT();
    };

  };
  
};
