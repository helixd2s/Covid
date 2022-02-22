#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"

// 
namespace lxvc {
  
  // 
  class DescriptorsObj : std::enable_shared_from_this<DescriptorsObj> {
  public:
    using tType = std::shared_ptr<DescriptorsObj>;
    friend DeviceObj;

    // 
    vk::AccelerationStructureKHR acceleration = {};
    AllocatedMemory allocated = {};
    std::optional<AccelerationCreateInfo> cInfo = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    DescriptorsObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<AccelerationCreateInfo> cInfo = AccelerationCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<AccelerationCreateInfo> cInfo = AccelerationCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();

      return this->SFT();
    };

  };

};
