#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"
#include "./Descriptors.hpp"

// 
namespace lxvc {

  // 
  class PipelineObj : public BaseObj {
  public: 
    using tType = std::shared_ptr<PipelineObj>;
    using BaseObj::BaseObj;
    
  protected:
    friend DeviceObj;

    // 
    //vk::Pipeline pipeline = {};
    std::optional<PipelineCreateInfo> cInfo = {};
    //std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return std::dynamic_pointer_cast<std::decay_t<decltype(*this)>>(shared_from_this()); };
    inline decltype(auto) SFT() const { return std::dynamic_pointer_cast<const std::decay_t<decltype(*this)>>(shared_from_this()); };

  public:
    // 
    PipelineObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    PipelineObj(Handle const& handle, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : cInfo(cInfo) {
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

  protected:
    //
    virtual void createCompute(cpp21::optional_ref<ComputePipelineCreateInfo> compute = {}) {
      decltype(auto) device = this->base.as<vk::Device>();
      this->handle = std::move<vk::Pipeline>(device.createComputePipeline(this->cInfo->descriptors->cache, vk::ComputePipelineCreateInfo{
        .flags = vk::PipelineCreateFlags{},
        .stage = makeComputePipelineStageInfo(device, *(compute->code)),
        .layout = this->cInfo->descriptors->handle.as<vk::PipelineLayout>()
      }));
      //
      lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());
      //return this->SFT();
    };

    //
    virtual void createGraphics(cpp21::optional_ref<GraphicsPipelineCreateInfo> graphics = {}) {
      //this->pipeline = makeComputePipelineStageInfo(this->deviceObj->device, compute->code);
      //
      lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());
      //return this->SFT();
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();
      if (this->cInfo->compute) { this->createCompute(this->cInfo->compute); };
      if (this->cInfo->graphics) { this->createGraphics(this->cInfo->graphics); };
      //return this->SFT();
    };

  };
  
};
