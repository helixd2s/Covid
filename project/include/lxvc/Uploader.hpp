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
      decltype(auto) submission = CommandSubmission{ .info = this->cInfo->info };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->deviceObj->device;
      decltype(auto) size = std::min(host.size(), bufferRegion->region.size);
      decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{ .srcOffset = 0ull, .dstOffset = bufferRegion->region.offset, .size = size} };
      decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = uploadBuffer->buffer, .dstBuffer = bufferRegion->buffer->buffer };

      //
      memcpy(uploadBuffer->mappedMemory, host.data(), size);

      // 
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _copyInfo = copyInfo;
        cmdBuf.copyBuffer2(_copyInfo.setRegions(regions));
        return cmdBuf;
      });

      //
      return this->deviceObj->executeCommandOnce(submission);
    };

    //
    virtual FenceType downloadFromBuffer(std::optional<BufferRegionObj> bufferRegion, cpp21::data_view<char8_t> const& host = {}) {
      decltype(auto) submission = CommandSubmission{ .info = this->cInfo->info };
      decltype(auto) uploadBuffer = this->uploadBuffer;
      decltype(auto) downloadBuffer = this->downloadBuffer;
      decltype(auto) device = this->deviceObj->device;
      decltype(auto) size = std::min(host.size(), bufferRegion->region.size);
      decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{ .srcOffset = bufferRegion->region.offset, .dstOffset = 0ull, .size = size } };
      decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = bufferRegion->buffer->buffer, .dstBuffer = downloadBuffer->buffer };

      // 
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _copyInfo = copyInfo;
        cmdBuf.copyBuffer2(_copyInfo.setRegions(regions));
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
