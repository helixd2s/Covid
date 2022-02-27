#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {
  
  // 
  class AccelerationStructureObj : public BaseObj {
  public:
    //using BaseObj;
    using tType = std::shared_ptr<AccelerationStructureObj>;
    using BaseObj::BaseObj;
    
  protected: 
    friend DeviceObj;

    // 
    //vk::AccelerationStructureKHR acceleration = {};
    std::optional<AllocatedMemory> allocated = {};
    std::optional<AccelerationStructureCreateInfo> cInfo = {};
    std::optional<MemoryRequirements> mReqs = {};

    //
    //std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return std::dynamic_pointer_cast<std::decay_t<decltype(*this)>>(shared_from_this()); };
    inline decltype(auto) SFT() const { return std::dynamic_pointer_cast<const std::decay_t<decltype(*this)>>(shared_from_this()); };

  public:
    // 
    AccelerationStructureObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<AccelerationStructureCreateInfo> cInfo = AccelerationStructureCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    AccelerationStructureObj(Handle const& handle, std::optional<AccelerationStructureCreateInfo> cInfo = AccelerationStructureCreateInfo{}) : cInfo(cInfo) {
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
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<AccelerationStructureCreateInfo> cInfo = AccelerationStructureCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();

      //return this->SFT();
    };

  };
  
};
