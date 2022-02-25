#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {
  
  // 
  class AccelerationStructureObj : std::enable_shared_from_this<AccelerationStructureObj> {
  public:
    using tType = std::shared_ptr<AccelerationStructureObj>;
    friend DeviceObj;

    // 
    vk::AccelerationStructureKHR acceleration = {};
    std::optional<AllocatedMemory> allocated = {};
    std::optional<AccelerationStructureCreateInfo> cInfo = {};
    std::optional<MemoryRequirements> mReqs = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    AccelerationStructureObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<AccelerationStructureCreateInfo> cInfo = AccelerationStructureCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<AccelerationStructureCreateInfo> cInfo = AccelerationStructureCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();

      return this->SFT();
    };

  };
  
};
