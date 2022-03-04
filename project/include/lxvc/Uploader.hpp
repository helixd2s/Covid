#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

// 
namespace lxvc {
  
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

    // 
    cpp21::vector_of_shared<MSS> layoutInfoMaps = {};
    std::optional<UploaderCreateInfo> cInfo = {};
    

    //
    //std::shared_ptr<DeviceObj> deviceObj = {};
    //std::shared_ptr<ResourceObj> uploadBuffer = {};
    //std::shared_ptr<ResourceObj> downloadBuffer = {};
    vk::Buffer uploadBuffer = {};
    vk::Buffer downloadBuffer = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

  public:

    // 
    UploaderObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    UploaderObj(Handle const& handle, std::optional<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) : cInfo(cInfo) {
      this->construct(lxvc::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual tType registerSelf() {
      lxvc::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(Handle const& handle, std::optional<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      return std::make_shared<UploaderObj>(handle, cInfo)->registerSelf();
    };



    //
    virtual FenceType executeUploadToImageOnce(std::span<char8_t> const& host, std::optional<ImageRegion> imageRegion) {
      decltype(auto) submission = CommandOnceSubmission{ .info = this->cInfo->info };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) size = host.size();
      decltype(auto) copyInfo = vk::CopyBufferToImageInfo2{ .srcBuffer = uploadBuffer, .dstImage = imageRegion->image, .dstImageLayout = vk::ImageLayout::eTransferDstOptimal };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) imageObj = deviceObj->get<ResourceObj>(imageRegion->image);
      decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
      
      //
      decltype(auto) subresourceRange = vk::ImageSubresourceRange{
          .aspectMask =
             imageObj->cInfo->imageInfo->type == ImageType::eDepthAttachment ? vk::ImageAspectFlagBits::eDepth :
            (imageObj->cInfo->imageInfo->type == ImageType::eStencilAttachment ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor),
          .baseMipLevel = imageRegion->region.baseMipLevel,
          .levelCount = 1u,//imageInfo->mipLevels - imageRegion->region->baseMipLevel,
          .baseArrayLayer = 0u,
          .layerCount = imageInfo->arrayLayers
      };

      //
      decltype(auto) subresourceLayers = vk::ImageSubresourceLayers{
        .aspectMask = subresourceRange.aspectMask,
        .mipLevel = subresourceRange.baseMipLevel,
        .baseArrayLayer = subresourceRange.baseArrayLayer,
        .layerCount = subresourceRange.layerCount
      };

      //
      decltype(auto) regions = std::vector<vk::BufferImageCopy2>{ vk::BufferImageCopy2{
        .bufferOffset = 0ull, .imageSubresource = subresourceLayers, .imageOffset = imageRegion->region.offset, .imageExtent = imageRegion->region.extent
      } };

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
          .offset = 0ull,
          .size = size
        }
      };

      //
      decltype(auto) imageBarriersBegin = std::vector<vk::ImageMemoryBarrier2>{
        vk::ImageMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
          .oldLayout = imageObj->cInfo->imageInfo->layout,
          .newLayout = vk::ImageLayout::eTransferDstOptimal,
          .srcQueueFamilyIndex = imageRegion->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .subresourceRange = subresourceRange
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
          .offset = 0ull,
          .size = size
        }
      };

      // 
      decltype(auto) imageBarriersEnd = std::vector<vk::ImageMemoryBarrier2>{
        vk::ImageMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .oldLayout = vk::ImageLayout::eTransferDstOptimal,
          .newLayout = imageObj->cInfo->imageInfo->layout,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = imageRegion->queueFamilyIndex,
          .subresourceRange = subresourceRange
        }
      };

      //
      memcpy(lxvc::context->get<DeviceObj>(this->base)->get<ResourceObj>(uploadBuffer)->mappedMemory, host.data(), size);

      // 
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _copyInfo = copyInfo;
        auto _depInfo = depInfo;
        cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin).setImageMemoryBarriers(imageBarriersBegin));
        cmdBuf.copyBufferToImage2(_copyInfo.setRegions(regions));
        cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd).setImageMemoryBarriers(imageBarriersEnd));
        return cmdBuf;
      });

      //
      return lxvc::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };



    //
    virtual FenceType executeUploadToBufferOnce(std::span<char8_t> const& host, std::optional<BufferRegion> bufferRegion) {
      decltype(auto) submission = CommandOnceSubmission{ .info = this->cInfo->info };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) size = std::min(host.size(), bufferRegion->region.size);
      decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = uploadBuffer, .dstBuffer = bufferRegion->buffer };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      //
      decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{.srcOffset = 0ull, .dstOffset = bufferRegion->region.offset, .size = size} };

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
          .offset = 0ull, 
          .size = size
        },
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
          .srcQueueFamilyIndex = bufferRegion->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = bufferRegion->buffer,
          .offset = bufferRegion->region.offset,
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
          .offset = 0ull,
          .size = size
        },
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = bufferRegion->queueFamilyIndex,
          .buffer = bufferRegion->buffer,
          .offset = bufferRegion->region.offset,
          .size = size
        }
      };

      //
      memcpy(lxvc::context->get<DeviceObj>(this->base)->get<ResourceObj>(uploadBuffer)->mappedMemory, host.data(), size);

      // 
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _copyInfo = copyInfo;
        auto _depInfo = depInfo;
        cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
        cmdBuf.copyBuffer2(_copyInfo.setRegions(regions));
        cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
        return cmdBuf;
      });

      //
      return lxvc::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType executeDownloadFromBufferOnce(std::optional<BufferRegion> bufferRegion, std::span<char8_t> const& host = {}) {
      decltype(auto) submission = CommandOnceSubmission{ .info = this->cInfo->info };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) size = std::min(host.size(), bufferRegion->region.size);
      decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{ .srcOffset = bufferRegion->region.offset, .dstOffset = 0ull, .size = size } };
      decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = bufferRegion->buffer, .dstBuffer = downloadBuffer };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      //
      decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
          .srcQueueFamilyIndex = bufferRegion->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = bufferRegion->buffer,
          .offset = bufferRegion->region.offset,
          .size = size
        },
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
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = bufferRegion->queueFamilyIndex,
          .buffer = bufferRegion->buffer,
          .offset = bufferRegion->region.offset,
          .size = size
        },
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

      // 
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _copyInfo = copyInfo;
        auto _depInfo = depInfo;
        cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
        cmdBuf.copyBuffer2(_copyInfo.setRegions(regions));
        cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
        return cmdBuf;
      });

      //
      submission.onDone.push_back([=](vk::Result const& result) {
        auto _host = host;
        memcpy(_host.data(), lxvc::context->get<DeviceObj>(this->base)->get<ResourceObj>(downloadBuffer)->mappedMemory, size);
      });

      //
      return lxvc::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

  protected:

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      this->cInfo = cInfo;
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;

      this->uploadBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .type = BufferType::eHostMap,
          .size = this->cInfo->cacheSize,
        }
      }).as<vk::Buffer>();

      this->downloadBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .type = BufferType::eHostMap,
          .size = this->cInfo->cacheSize,
        }
      }).as<vk::Buffer>();
    };
  };

};
