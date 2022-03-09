#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {
  
  // 
  class InstanceLevelObj : public BaseObj {
  public:
    //using BaseObj;
    using tType = WrapShared<InstanceLevelObj>;
    using BaseObj::BaseObj;
    
  protected: 
    friend DeviceObj;

    // 
    //vk::InstanceLevelKHR acceleration = {};
    std::optional<AllocatedMemory> allocated = AllocatedMemory{};
    std::optional<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{};
    std::optional<MemoryRequirements> mReqs = {};

    //
    //std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

  public:
    // 
    InstanceLevelObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    InstanceLevelObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) {
      auto shared = std::make_shared<InstanceLevelObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  protected:
    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());

      //return this->SFT();
    };

  };
  
};