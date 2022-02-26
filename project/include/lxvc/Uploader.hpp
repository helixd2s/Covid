#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

// 
namespace lxvc {
  
  // 
  class UploaderObj : std::enable_shared_from_this<UploaderObj> {
  protected: 
    using tType = std::shared_ptr<UploaderObj>;
    friend DeviceObj;
    friend PipelineObj;
    friend ResourceObj;

    // 
    cpp21::vector_of_shared<MSS> layoutInfoMaps = {};
    std::optional<UploaderCreateInfo> cInfo = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};
    std::shared_ptr<ResourceObj> uploadBuffer = {};
    std::shared_ptr<ResourceObj> downloadBuffer = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

  public:
    // 
    UploaderObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    //
    virtual FenceType uploadToBuffer(cpp21::data_view<char8_t> const& host, std::optional<BufferRegionObj> bufferRegion) {
      decltype(auto) submission = CommandOnceSubmission{ .info = this->cInfo->info };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->deviceObj->device;
      decltype(auto) size = std::min(host.size(), bufferRegion->region.size);
      decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = uploadBuffer->buffer, .dstBuffer = bufferRegion->buffer->buffer };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      //
      decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{.srcOffset = 0ull, .dstOffset = bufferRegion->region.offset, .size = size} };

      //
      decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .srcAccessMask = vk::AccessFlagBits2::eHostWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eMemoryWrite,
          .dstStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .dstAccessMask = vk::AccessFlagBits2::eHostRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eMemoryRead,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = uploadBuffer->buffer,
          .offset = 0ull, 
          .size = size
        },
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
          .srcAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead,
          .dstStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
          .buffer = bufferRegion->buffer->buffer,
          .offset = bufferRegion->region.offset,
          .size = size
        }
      };

      //
      decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .srcAccessMask = vk::AccessFlagBits2::eHostRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eMemoryRead,
          .dstStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .dstAccessMask = vk::AccessFlagBits2::eHostWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eMemoryWrite,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = uploadBuffer->buffer,
          .offset = 0ull,
          .size = size
        },
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .srcAccessMask =  vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
          .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
          .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead,
          .buffer = bufferRegion->buffer->buffer,
          .offset = bufferRegion->region.offset,
          .size = size
        }
      };


      //
      memcpy(uploadBuffer->mappedMemory, host.data(), size);

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
      return this->deviceObj->executeCommandOnce(submission);
    };

    //
    virtual FenceType downloadFromBuffer(std::optional<BufferRegionObj> bufferRegion, cpp21::data_view<char8_t> const& host = {}) {
      decltype(auto) submission = CommandOnceSubmission{ .info = this->cInfo->info };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->deviceObj->device;
      decltype(auto) size = std::min(host.size(), bufferRegion->region.size);
      decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{ .srcOffset = bufferRegion->region.offset, .dstOffset = 0ull, .size = size } };
      decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = bufferRegion->buffer->buffer, .dstBuffer = downloadBuffer->buffer };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      //
      decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
          .srcAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
          .dstStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead,
          .buffer = bufferRegion->buffer->buffer,
          .offset = bufferRegion->region.offset,
          .size = size
        },
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .srcAccessMask = vk::AccessFlagBits2::eHostRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eMemoryRead,
          .dstStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .dstAccessMask = vk::AccessFlagBits2::eHostWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eMemoryWrite,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = downloadBuffer->buffer,
          .offset = 0ull,
          .size = size
        },
      };

      //
      decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .srcAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead,
          .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
          .dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
          .buffer = bufferRegion->buffer->buffer,
          .offset = bufferRegion->region.offset,
          .size = size
        },
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .srcAccessMask = vk::AccessFlagBits2::eHostWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eMemoryWrite,
          .dstStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .dstAccessMask = vk::AccessFlagBits2::eHostRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eMemoryRead,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = downloadBuffer->buffer,
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
        memcpy(_host.data(), downloadBuffer->mappedMemory, size);
      });

      //
      return this->deviceObj->executeCommandOnce(submission);
    };

  protected:

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      this->cInfo = cInfo;
      this->deviceObj = deviceObj;

      this->uploadBuffer = std::make_shared<ResourceObj>(this->deviceObj, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .type = BufferType::eHostMap,
          .size = this->cInfo->cacheSize
        }
      });

      this->downloadBuffer = std::make_shared<ResourceObj>(this->deviceObj, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .type = BufferType::eHostMap,
          .size = this->cInfo->cacheSize
        }
      });
    };
  };

};
