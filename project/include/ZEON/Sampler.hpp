#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"
#include "./MemoryAllocator.hpp"
#include "./Descriptors.hpp"

// 
namespace ZNAMED {

  // 
  class SamplerObj : public BaseObj {
  public: 
    using tType = WrapShared<SamplerObj>;
    using BaseObj::BaseObj;

  protected:
    friend DeviceObj;
    friend DescriptorsObj;
    friend UploaderObj;
    friend FramebufferObj;
    friend SwapchainObj;
    friend GeometryLevelObj;
    friend InstanceLevelObj;
    friend MemoryAllocatorObj;

    //
    uint32_t descriptorId = 0xFFFFFFFFu;

    // 
    std::optional<SamplerCreateInfo> cInfo = SamplerCreateInfo{};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:
    // 
    SamplerObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<SamplerCreateInfo> cInfo = SamplerCreateInfo{}) : BaseObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    SamplerObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<SamplerCreateInfo> cInfo = SamplerCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      this->construct(ZNAMED::context->get<DeviceObj>(this->base), cInfo);
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
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<SamplerCreateInfo> cInfo = SamplerCreateInfo{}) {
      auto shared = std::make_shared<SamplerObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  protected:

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<SamplerCreateInfo> cInfo = SamplerCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };

      // 
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) samplerInfo = infoMap->set(vk::StructureType::eSamplerCreateInfo, this->cInfo->native);
      this->handle = device.createSampler(samplerInfo.ref());

      //
      decltype(auto) descriptorsObj = this->cInfo->descriptors ? deviceObj->get<DescriptorsObj>(this->cInfo->descriptors) : WrapShared<DescriptorsObj>{};
      if (descriptorsObj) {
        descriptorId = descriptorsObj->getSamplerDescriptors().add(vk::DescriptorImageInfo{.sampler = this->handle.as<vk::Sampler>()});
      };

      //
      this->destructors.insert(this->destructors.begin(), 1, [device, sampler = this->handle.as<vk::Sampler>()](BaseObj const* baseObj) {
        device.destroySampler(sampler);
      });

      //
      if (descriptorsObj) {
        this->destructors.insert(this->destructors.begin(), 1, [descriptorId = this->descriptorId, samplers = descriptorsObj->getSamplerDescriptors()](BaseObj const* baseObj) {
          const_cast<cpp21::bucket<vk::DescriptorImageInfo>&>(samplers).removeByIndex(descriptorId);
        });
      };
    };

  public:

    // 
    virtual uint32_t& getId() { return descriptorId; };
    virtual uint32_t const& getId() const { return descriptorId; };

  };

  
};
