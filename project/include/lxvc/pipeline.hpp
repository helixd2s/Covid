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
    using tType = WrapShared<PipelineObj>;
    using BaseObj::BaseObj;
    
  protected:
    friend DeviceObj;

    // 
    //vk::Pipeline pipeline = {};
    std::optional<PipelineCreateInfo> cInfo = {};
    //std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

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

    //
    inline static tType make(Handle const& handle, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
      return std::make_shared<PipelineObj>(handle, cInfo)->registerSelf();
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
      //lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());
      //return this->SFT();
    };

    //
    virtual void createGraphics(cpp21::optional_ref<GraphicsPipelineCreateInfo> graphics = {}) {
      //this->pipeline = makeComputePipelineStageInfo(this->deviceObj->device, compute->code);
      //
      //lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());
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

    // TODO: using multiple-command
    virtual FenceType executeComputeOnce(std::optional<ExecuteComputeInfo> exec = ExecuteComputeInfo{}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(exec->layout);
      decltype(auto) submission = CommandOnceSubmission{ .info = exec->info };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      //
      decltype(auto) memoryBarriersBegin = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eShaderReadWrite) | vk::PipelineStageFlagBits2::eComputeShader,
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite)
        }
      };

      //
      decltype(auto) memoryBarriersEnd = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eShaderReadWrite) | vk::PipelineStageFlagBits2::eComputeShader,
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite)
        }
      };

      // 
      std::vector<uint32_t> offsets = {};
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _depInfo = depInfo;
        cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersBegin));
        cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eCompute, descriptorsObj->handle.as<vk::PipelineLayout>(), 0u, descriptorsObj->sets, offsets);
        cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, this->handle.as<vk::Pipeline>());
        cmdBuf.dispatch(exec->dispatch.width, exec->dispatch.height, exec->dispatch.depth);
        cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersEnd));
        return cmdBuf;
      });

      //
      return deviceObj->executeCommandOnce(submission);
    };

  };
  
};
