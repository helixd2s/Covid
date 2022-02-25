#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {
  
  // 
  class QueueFamilyObj : std::enable_shared_from_this<QueueFamilyObj> {
  public:
    using tType = std::shared_ptr<QueueFamilyObj>;
    friend DeviceObj;

    // 
    std::vector<vk::Queue> queues = {};
    std::optional<QueueFamilyCreateInfo> cInfo = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    QueueFamilyObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();

      return this->SFT();
    };

  };
  
};
