#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"
#include "./Descriptors.hpp"

// 
namespace lxvc {

  // 
  class PipelineObj : std::enable_shared_from_this<PipelineObj> {
  protected: 
    using tType = std::shared_ptr<PipelineObj>;
    friend DeviceObj;

    // 
    vk::Pipeline pipeline = {};
    std::optional<PipelineCreateInfo> cInfo = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

  public:
    // 
    PipelineObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

  protected:
    //
    virtual tType createCompute(cpp21::optional_ref<ComputePipelineCreateInfo> compute = {}) {
      this->pipeline = std::forward<vk::Pipeline>(deviceObj->device.createComputePipeline(this->cInfo->descriptors->cache, vk::ComputePipelineCreateInfo{
        .flags = vk::PipelineCreateFlags{},
        .stage = makeComputePipelineStageInfo(this->deviceObj->device, *(compute->code)),
        .layout = this->cInfo->descriptors->layout
      }));
      return this->SFT();
    };

    //
    virtual tType createGraphics(cpp21::optional_ref<GraphicsPipelineCreateInfo> graphics = {}) {
      //this->pipeline = makeComputePipelineStageInfo(this->deviceObj->device, compute->code);
      return this->SFT();
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();
      if (this->cInfo->compute) { this->createCompute(this->cInfo->compute); };
      if (this->cInfo->graphics) { this->createGraphics(this->cInfo->graphics); };
      return this->SFT();
    };

  };
  
};
