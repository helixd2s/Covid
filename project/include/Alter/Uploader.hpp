#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

//
#ifdef ALT_ENABLE_VMA
#include "./MemoryAllocatorVma.hpp"
#endif

// 
namespace ANAMED {
  
  // 
  class UploaderObj : public BaseObj {
  public: 
    //using BaseObj;
    using tType = WrapShared<UploaderObj>;
    using BaseObj::BaseObj;

  protected: 
    friend DeviceObj;
    friend PipelineObj;
    friend ResourceObj;
    friend GeometryLevelObj;
    friend InstanceLevelObj;

    // 
    cpp21::vector_of_shared<MSS> layoutInfoMaps = {};
    std::optional<UploaderCreateInfo> cInfo = UploaderCreateInfo{};
    
    //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
    VmaVirtualBlock mappedBlock;
#endif

    //
    vk::Buffer mappedBuffer = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:

    // 
    UploaderObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    UploaderObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    // 
     std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual tType registerSelf() {
      ANAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      auto shared = std::make_shared<UploaderObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual void* getMappedMemory(uintptr_t const& offset = 0ull) { return cpp21::shift(ANAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(mappedBuffer)->mappedMemory, offset); };

    // you can copy from host to device Buffer and Image together!
    // TODO: per-type role based barriers...
    virtual tType writeUploadToResourceCmd(cpp21::const_wrap_arg<UploadCommandWriteInfo> copyRegionInfo) {
      //decltype(auto) submission = CommandOnceSubmission{ .info = this->cInfo->info };
      decltype(auto) mappedBuffer = this->mappedBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) size = copyRegionInfo->dstBuffer ? copyRegionInfo->dstBuffer->region.size : VK_WHOLE_SIZE;
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) hostMapOffset = /*hostMapOffset_ ? (*hostMapOffset_) :*/ copyRegionInfo->hostMapOffset;

      //
      decltype(auto) subresourceRange = vk::ImageSubresourceRange{};
      decltype(auto) subresourceLayers = vk::ImageSubresourceLayers{};

      // 
      decltype(auto) BtI = vk::CopyBufferToImageInfo2{};
      decltype(auto) BtB = vk::CopyBufferInfo2{};
      decltype(auto) BtBRegions = std::vector<vk::BufferCopy2>{  };
      decltype(auto) BtIRegions = std::vector<vk::BufferImageCopy2>{  };

      //
      decltype(auto) imageBarriersBegin = std::vector<vk::ImageMemoryBarrier2>{};
      decltype(auto) imageBarriersEnd = std::vector<vk::ImageMemoryBarrier2>{};

      //
      decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = mappedBuffer,
          .offset = hostMapOffset,
          .size = size
        }
      };

      //
      decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = mappedBuffer,
          .offset = hostMapOffset,
          .size = size
        }
      };

      // 
      if (copyRegionInfo->dstImage) {
        auto& imageRegion = copyRegionInfo->dstImage.value();

        //
        decltype(auto) imageObj = deviceObj->get<ResourceObj>(imageRegion.image);
        decltype(auto) imageInfo = imageObj->infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);

        // 
        subresourceRange = imageObj->subresourceRange(imageRegion.region.baseLayer, imageRegion.region.layerCount, imageRegion.region.baseMipLevel, 1u);
        subresourceLayers = vk::ImageSubresourceLayers{
          .aspectMask = subresourceRange.aspectMask,
          .mipLevel = subresourceRange.baseMipLevel,
          .baseArrayLayer = subresourceRange.baseArrayLayer,
          .layerCount = subresourceRange.layerCount
        };

        BtIRegions.push_back(vk::BufferImageCopy2{
          .bufferOffset = hostMapOffset, .imageSubresource = subresourceLayers, .imageOffset = imageRegion.region.offset, .imageExtent = imageRegion.region.extent
        });
        BtI = vk::CopyBufferToImageInfo2{ .srcBuffer = mappedBuffer, .dstImage = imageRegion.image, .dstImageLayout = vk::ImageLayout::eTransferDstOptimal };

        //
        decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(imageObj->getImageUsage()));

        //
        imageBarriersBegin.push_back(vk::ImageMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
            .srcAccessMask = vk::AccessFlagBits2(accessMask),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
            .oldLayout = imageObj->cInfo->imageInfo->layout,
            .newLayout = vk::ImageLayout::eTransferDstOptimal,
            .srcQueueFamilyIndex = imageRegion.queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .image = imageRegion.image,
            .subresourceRange = subresourceRange
        });

        //
        imageBarriersEnd.push_back(vk::ImageMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
            .dstAccessMask = vk::AccessFlagBits2(accessMask),
            .oldLayout = vk::ImageLayout::eTransferDstOptimal,
            .newLayout = imageObj->cInfo->imageInfo->layout,
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = imageRegion.queueFamilyIndex,
            .image = imageRegion.image,
            .subresourceRange = subresourceRange
          });
      };

      // 
      if (copyRegionInfo->dstBuffer) {
        auto& bufferRegion = copyRegionInfo->dstBuffer.value();

        //
        decltype(auto) bufferObj = deviceObj->get<ResourceObj>(bufferRegion.buffer);
        decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(bufferObj->getBufferUsage()));

        //
        BtBRegions.push_back(vk::BufferCopy2{ .srcOffset = hostMapOffset, .dstOffset = bufferRegion.region.offset, .size = size });
        BtB = vk::CopyBufferInfo2{ .srcBuffer = mappedBuffer, .dstBuffer = bufferRegion.buffer };

        //
        bufferBarriersBegin.push_back(vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
          .srcAccessMask = accessMask,
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
          .srcQueueFamilyIndex = bufferRegion.queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = bufferRegion.buffer,
          .offset = bufferRegion.region.offset,
          .size = size
        });

        //
        bufferBarriersEnd.push_back(vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
          .dstAccessMask = accessMask,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = bufferRegion.queueFamilyIndex,
          .buffer = bufferRegion.buffer,
          .offset = bufferRegion.region.offset,
          .size = size
        });
      };

      // 
      copyRegionInfo->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin).setImageMemoryBarriers(imageBarriersBegin));
      if (copyRegionInfo->dstImage) copyRegionInfo->cmdBuf.copyBufferToImage2(BtI.setRegions(BtIRegions));
      if (copyRegionInfo->dstBuffer) copyRegionInfo->cmdBuf.copyBuffer2(BtB.setRegions(BtBRegions));
      copyRegionInfo->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd).setImageMemoryBarriers(imageBarriersEnd));

      //
      return SFT();
    };

    //
    // TODO: per-type role based barriers...
    // TODO: image, imageType and imageLayout supports...
    virtual tType writeDownloadToResourceCmd(cpp21::const_wrap_arg<DownloadCommandWriteInfo> info) {
      decltype(auto) mappedBuffer = this->mappedBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) regions = std::vector<vk::BufferCopy2>{  };
      decltype(auto) copyInfo = vk::CopyBufferInfo2{};
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) size = info->srcBuffer ? info->srcBuffer->region.size : VK_WHOLE_SIZE;
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) hostMapOffset = /*hostMapOffset_ ? (*hostMapOffset_) :*/ info->hostMapOffset;

      //
      decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = mappedBuffer,
          .offset = hostMapOffset,
          .size = size
        },
      };

      //
      decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = mappedBuffer,
          .offset = hostMapOffset,
          .size = size
        }
      };

      // 
      if (info->srcBuffer) {
        //
        decltype(auto) bufferObj = deviceObj->get<ResourceObj>(info->srcBuffer->buffer);
        decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(bufferObj->getBufferUsage()));

        //
        copyInfo = vk::CopyBufferInfo2{ .srcBuffer = info->srcBuffer->buffer, .dstBuffer = mappedBuffer };
        regions.push_back(vk::BufferCopy2{ .srcOffset = info->srcBuffer->region.offset, .dstOffset = hostMapOffset, .size = size });

        //
        bufferBarriersBegin.push_back(vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask),
          .srcAccessMask = accessMask,
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
          .srcQueueFamilyIndex = info->srcBuffer->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = info->srcBuffer->buffer,
          .offset = info->srcBuffer->region.offset,
          .size = size
        });

        //
        bufferBarriersEnd.push_back(vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask),
          .dstAccessMask = accessMask,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = info->srcBuffer->queueFamilyIndex,
          .buffer = info->srcBuffer->buffer,
          .offset = info->srcBuffer->region.offset,
          .size = size
        });
      };

      if (info->srcBuffer) {
        info->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
        info->cmdBuf.copyBuffer2(copyInfo.setRegions(regions));
        info->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
      };

      return SFT();
    };

    //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
    virtual VmaVirtualBlock& getMappedBlock() { return mappedBlock; };
    virtual VmaVirtualBlock const& getMappedBlock() const { return mappedBlock; };

    //
    virtual VmaVirtualAllocation allocateMappedTemp(size_t const& size, uintptr_t& offset) {
      // 
      VmaVirtualAllocationCreateInfo allocCreateInfo = {};
      allocCreateInfo.size = size; // 4 KB
      allocCreateInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT;

      //
      VmaVirtualAllocation alloc;
      VkResult upRes = vmaVirtualAllocate(mappedBlock, &allocCreateInfo, &alloc, &offset);

      //
      return alloc;
    };

    //
    virtual std::tuple<VkDeviceSize, VmaVirtualAllocation> allocateMappedTemp(size_t const& size) {
      VkDeviceSize offset = 0ull;
      return std::make_tuple(offset, allocateMappedTemp(size, offset));
    };
#endif

    //
    virtual FenceType executeUploadToResourceOnce(cpp21::const_wrap_arg<UploadExecutionOnce> exec) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = this->cInfo->info } };
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) size = exec->host ? (exec->writeInfo.dstBuffer ? std::min(exec->host->size(), exec->writeInfo.dstBuffer->region.size) : exec->host->size()) : (exec->writeInfo.dstBuffer ? exec->writeInfo.dstBuffer->region.size : VK_WHOLE_SIZE);
      decltype(auto) mappedBuffer = this->mappedBuffer;

      //
      if (exec->writeInfo.dstImage) {
        decltype(auto) pixelCount = size_t(exec->writeInfo.dstImage->region.extent.width) * size_t(exec->writeInfo.dstImage->region.extent.height) * size_t(exec->writeInfo.dstImage->region.extent.depth) * size_t(exec->writeInfo.dstImage->region.layerCount);
        decltype(auto) imageObj = deviceObj->get<ResourceObj>(exec->writeInfo.dstImage->image);
        decltype(auto) cInfo = imageObj->infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
        if (cInfo) {
          size = std::min(size, pixelCount * size_t(vku::vk_format_table.at(VkFormat(cInfo->format)).size));
        };
      };

      // 
      VkDeviceSize offset = exec->writeInfo.hostMapOffset;

      // 
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      decltype(auto) alloc = allocateMappedTemp(size, offset);
#endif

      // 
      if (exec->host) {
        memcpy(this->getMappedMemory(offset), exec->host->data(), size);
      };

      // 
      submission.commandInits.push_back([exec, offset, this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        this->writeUploadToResourceCmd(exec->writeInfo.with(cmdBuf).mapOffset(offset));
        return cmdBuf;
      });

      //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      submission.submission.onDone.push_back([mappedBlock=this->mappedBlock, alloc](cpp21::const_wrap_arg<vk::Result> result) {
        vmaVirtualFree(mappedBlock, alloc);
      });
#endif

      //
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType executeDownloadToResourceOnce(cpp21::const_wrap_arg<DownloadExecutionOnce> exec) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = this->cInfo->info } };
      decltype(auto) mappedBuffer = this->mappedBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) size = exec->host ? (exec->writeInfo.srcBuffer ? std::min(exec->host->size(), exec->writeInfo.srcBuffer->region.size) : exec->host->size()) : (exec->writeInfo.srcBuffer ? exec->writeInfo.srcBuffer->region.size : VK_WHOLE_SIZE);
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      // 
      VkDeviceSize offset = exec->writeInfo.hostMapOffset;

      // 
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      decltype(auto) alloc = allocateMappedTemp(size, offset);
#endif

      // 
      submission.commandInits.push_back([exec, offset, this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        this->writeDownloadToResourceCmd(exec->writeInfo.with(cmdBuf).mapOffset(offset));
        return cmdBuf;
      });

      //
      if (exec->host) {
        submission.submission.onDone.push_back([offset, size, _host = exec->host, mapped = this->getMappedMemory(offset)](cpp21::const_wrap_arg<vk::Result> result) {
          memcpy(_host->data(), cpp21::shift(mapped, offset), size);
        });

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        submission.submission.onDone.push_back([mappedBlock = this->mappedBlock, alloc](cpp21::const_wrap_arg<vk::Result> result) {
          vmaVirtualFree(mappedBlock, alloc);
        });
#endif
      };

      //
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };


  protected:

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      //this->deviceObj = deviceObj;

      this->mappedBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = this->cInfo->cacheSize,
          .type = BufferType::eHostMap,
        }
      }).as<vk::Buffer>();

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      // 
      VmaVirtualBlockCreateInfo blockCreateInfo = {};
      blockCreateInfo.size = this->cInfo->cacheSize;
      //blockCreateInfo.flags = VMA_VIRTUAL_BLOCK_CREATE_LINEAR_ALGORITHM_BIT;

      // 
      VkResult result = vmaCreateVirtualBlock(&blockCreateInfo, &mappedBlock);
#endif

      // 
      this->handle = uintptr_t(this);//Handle(uintptr_t(this), HandleType::eUploader);
      this->handle.type = HandleType::eUploader; // Unable to Map without specific type
    };
  };

};
#endif
