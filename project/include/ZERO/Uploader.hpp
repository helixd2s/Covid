#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

// 
namespace ZNAMED {
  
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
    //std::shared_ptr<DeviceObj> deviceObj = {};
    //std::shared_ptr<ResourceObj> uploadBuffer = {};
    //std::shared_ptr<ResourceObj> downloadBuffer = {};
    vk::Buffer uploadBuffer = {};
    vk::Buffer downloadBuffer = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:

    // 
    UploaderObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->getHandle();
      this->construct(deviceObj, cInfo);
    };

    // 
    UploaderObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) : cInfo(cInfo) {
      this->construct(ZNAMED::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual tType registerSelf() {
      ZNAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      auto shared = std::make_shared<UploaderObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    // you can copy from host to device Buffer and Image together!
    // TODO: per-type role based barriers...
    virtual tType writeUploadToResourceCmd(cpp21::const_wrap_arg<UploadCommandWriteInfo> copyRegionInfo) {
      //decltype(auto) submission = CommandOnceSubmission{ .info = this->cInfo->info };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) size = copyRegionInfo->dstBuffer ? copyRegionInfo->dstBuffer->region.size : VK_WHOLE_SIZE;
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

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
          .buffer = uploadBuffer,
          .offset = copyRegionInfo->hostMapOffset,
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
          .buffer = uploadBuffer,
          .offset = copyRegionInfo->hostMapOffset,
          .size = size
        }
      };

      // 
      if (copyRegionInfo->dstImage) {
        auto& imageRegion = copyRegionInfo->dstImage.value();

        decltype(auto) imageObj = deviceObj->get<ResourceObj>(imageRegion.image);
        decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);

        BtIRegions.push_back(vk::BufferImageCopy2{
          .bufferOffset = copyRegionInfo->hostMapOffset, .imageSubresource = subresourceLayers, .imageOffset = imageRegion.region.offset, .imageExtent = imageRegion.region.extent
        });
        BtI = vk::CopyBufferToImageInfo2{ .srcBuffer = uploadBuffer, .dstImage = imageRegion.image, .dstImageLayout = vk::ImageLayout::eTransferDstOptimal };

        // 
        subresourceRange = vk::ImageSubresourceRange{
          .aspectMask =
             imageObj->cInfo->imageInfo->type == ImageType::eDepthStencilAttachment ? vk::ImageAspectFlagBits::eDepth :
            (imageObj->cInfo->imageInfo->type == ImageType::eDepthAttachment ? vk::ImageAspectFlagBits::eDepth :
            (imageObj->cInfo->imageInfo->type == ImageType::eStencilAttachment ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor)),
          .baseMipLevel = imageRegion.region.baseMipLevel,
          .levelCount = 1u,//imageInfo->mipLevels - imageRegion->region->baseMipLevel,
          .baseArrayLayer = 0u,
          .layerCount = imageInfo->arrayLayers
        };

        // 
        subresourceLayers = vk::ImageSubresourceLayers{
          .aspectMask = subresourceRange.aspectMask,
          .mipLevel = subresourceRange.baseMipLevel,
          .baseArrayLayer = subresourceRange.baseArrayLayer,
          .layerCount = subresourceRange.layerCount
        };

        //
        imageBarriersBegin.push_back(vk::ImageMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
            .oldLayout = vk::ImageLayout::eTransferDstOptimal,
            .newLayout = imageObj->cInfo->imageInfo->layout,
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = imageRegion.queueFamilyIndex,
            .image = imageRegion.image,
            .subresourceRange = subresourceRange
          });

        //
        imageBarriersEnd.push_back(vk::ImageMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
            .oldLayout = imageObj->cInfo->imageInfo->layout,
            .newLayout = vk::ImageLayout::eTransferDstOptimal,
            .srcQueueFamilyIndex = imageRegion.queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .image = imageRegion.image,
            .subresourceRange = subresourceRange
          });
      };

      // 
      if (copyRegionInfo->dstBuffer) {
        auto& bufferRegion = copyRegionInfo->dstBuffer.value();

        BtBRegions.push_back(vk::BufferCopy2{ .srcOffset = copyRegionInfo->hostMapOffset, .dstOffset = bufferRegion.region.offset, .size = size });
        BtB = vk::CopyBufferInfo2{ .srcBuffer = uploadBuffer, .dstBuffer = bufferRegion.buffer };

        bufferBarriersBegin.push_back(vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
          .srcQueueFamilyIndex = bufferRegion.queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = bufferRegion.buffer,
          .offset = bufferRegion.region.offset,
          .size = size
        });

        bufferBarriersEnd.push_back(vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = bufferRegion.queueFamilyIndex,
          .buffer = bufferRegion.buffer,
          .offset = bufferRegion.region.offset,
          .size = size
        });
      };

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
      //decltype(auto) submission = CommandOnceSubmission{ .info = SubmissionInfo {.info = this->cInfo->info } };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) regions = std::vector<vk::BufferCopy2>{  };
      decltype(auto) copyInfo = vk::CopyBufferInfo2{};
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) size = info->srcBuffer ? info->srcBuffer->region.size : VK_WHOLE_SIZE;

      //
      decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = downloadBuffer,
          .offset = 0ull,
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
          .buffer = downloadBuffer,
          .offset = 0ull,
          .size = size
        }
      };

      if (info->srcBuffer) {
        copyInfo = vk::CopyBufferInfo2{ .srcBuffer = info->srcBuffer->buffer, .dstBuffer = downloadBuffer };
        regions.push_back(vk::BufferCopy2{ .srcOffset = info->srcBuffer->region.offset, .dstOffset = 0ull, .size = size });

        bufferBarriersBegin.push_back(vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
          .srcQueueFamilyIndex = info->srcBuffer->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = info->srcBuffer->buffer,
          .offset = info->srcBuffer->region.offset,
          .size = size
        });

        bufferBarriersEnd.push_back(vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
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
    virtual FenceType executeUploadToResourceOnce(cpp21::const_wrap_arg<UploadExecutionOnce> exec) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = this->cInfo->info } };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) size = exec->host.size();
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);

      // 
      memcpy(cpp21::shift(ZNAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(uploadBuffer)->mappedMemory, exec->hostMapOffset), exec->host.data(), size);

      // 
      submission.commandInits.push_back([=,this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        this->writeUploadToResourceCmd(exec->writeInfo.with(cmdBuf));
        return cmdBuf;
      });

      //
      return ZNAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType executeDownloadToResourceOnce(cpp21::const_wrap_arg<DownloadExecutionOnce> exec) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = this->cInfo->info } };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) size = std::min(exec->host.size(), exec->writeInfo.srcBuffer->region.size);
      decltype(auto) regions = std::vector<vk::BufferCopy2>{  };
      decltype(auto) copyInfo = vk::CopyBufferInfo2{};
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      // 
      submission.commandInits.push_back([=,this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        this->writeDownloadToResourceCmd(exec->writeInfo.with(cmdBuf));
        return cmdBuf;
      });

      //
      submission.onDone.push_back([=](cpp21::const_wrap_arg<vk::Result> result) {
        auto _host = exec->host;
        memcpy(_host.data(), ZNAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(downloadBuffer)->mappedMemory, size);
      });

      //
      return ZNAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };


  protected:

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      this->base = deviceObj->getHandle();
      //this->deviceObj = deviceObj;

      this->uploadBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = this->cInfo->cacheSize,
          .type = BufferType::eHostMap,
        }
      }).as<vk::Buffer>();

      this->downloadBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = this->cInfo->cacheSize,
          .type = BufferType::eHostMap,
        }
      }).as<vk::Buffer>();

      this->handle = uintptr_t(this);
    };
  };

};
