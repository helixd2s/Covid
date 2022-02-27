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
    using tType = std::shared_ptr<QueueFamilyObj>;
    using BaseObj::BaseObj;

  protected:
    friend DeviceObj;

    // 
    std::vector<vk::Queue> queues = {};
    std::optional<QueueFamilyCreateInfo> cInfo = {};
    //std::shared_ptr<MSS> infoMap = {};

    //
    //std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return std::dynamic_pointer_cast<std::decay_t<decltype(*this)>>(shared_from_this()); };
    inline decltype(auto) SFT() const { return std::dynamic_pointer_cast<const std::decay_t<decltype(*this)>>(shared_from_this()); };

    // 
  public:
    QueueFamilyObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) : cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    QueueFamilyObj(Handle const& handle, std::optional<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) : cInfo(cInfo) {
      this->construct(lxvc::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    // 
  protected:
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();

      return this->SFT();
    };

  };
  
};
