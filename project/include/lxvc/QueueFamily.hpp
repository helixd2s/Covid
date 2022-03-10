#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {
  
  // 
  class QueueFamilyObj : public BaseObj {
  public:
    //using BaseObj;
    using tType = WrapShared<QueueFamilyObj>;
    using BaseObj::BaseObj;

  protected:
    friend DeviceObj;

    // 
    std::vector<vk::Queue> queues = {};
    std::optional<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{};
    //std::shared_ptr<MSS> infoMap = {};

    //
    //std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    // 
  public:
    QueueFamilyObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) : cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    QueueFamilyObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) {
      auto shared = std::make_shared<QueueFamilyObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    // 
  protected:
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());

      //return this->SFT();
    };

  };
  
};
