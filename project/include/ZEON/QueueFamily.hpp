#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace ZNAMED {
  
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

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    // 
  public:
    QueueFamilyObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) : BaseObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    QueueFamilyObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
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
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) {
      auto shared = std::make_shared<QueueFamilyObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    // 
  protected:
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
    };

    

  };
  
};
