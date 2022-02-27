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
    virtual FenceType uploadToBuffer(cpp21::data_view<char8_t> const& host, std::optional<BufferRegion> bufferRegion) {
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
          .srcStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .srcAccessMask = vk::AccessFlagBits2::eHostWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eMemoryWrite,
          .dstStageMask = vk::PipelineStageFlagBits2::eHost | vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .dstAccessMask = vk::AccessFlagBits2::eHostRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eMemoryRead,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = uploadBuffer,
          .offset = 0ull, 
          .size = size
        },
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
          .srcAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eShaderRead,
          .dstStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
          .buffer = bufferRegion->buffer,
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
          .buffer = uploadBuffer,
          .offset = 0ull,
          .size = size
        },
        vk::BufferMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .srcAccessMask =  vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
          .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
          .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eShaderRead,
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
    virtual FenceType downloadFromBuffer(std::optional<BufferRegion> bufferRegion, cpp21::data_view<char8_t> const& host = {}) {
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
          .srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
          .srcAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eShaderWrite,
          .dstStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
          .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead,
          .buffer = bufferRegion->buffer,
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
          .buffer = downloadBuffer,
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
          .dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eShaderWrite,
          .buffer = bufferRegion->buffer,
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
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      this->cInfo = cInfo;
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;

      this->uploadBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .type = BufferType::eHostMap,
          .size = this->cInfo->cacheSize
        }
      }).as<vk::Buffer>();

      this->downloadBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .type = BufferType::eHostMap,
          .size = this->cInfo->cacheSize
        }
      }).as<vk::Buffer>();
    };
  };

};
