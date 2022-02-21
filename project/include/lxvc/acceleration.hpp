#pragma once

// 
#include "./core.hpp"
#include "./device.hpp"

// 
namespace lxvc {
  
  // 
  class AccelerationObj : std::enable_shared_from_this<AccelerationObj> {
  public:
    using tType = std::shared_ptr<AccelerationObj>;
    using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    friend DeviceObj;

    // 
    vk::AccelerationStructureKHR acceleration = {};
    std::optional<AllocatedMemory> allocated = {};
    std::optional<AccelerationCreateInfo> cInfo = {};
    std::optional<MemoryRequirements> mReqs = {};
    MSS infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    AccelerationObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<AccelerationCreateInfo> cInfo = AccelerationCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<AccelerationCreateInfo> cInfo = AccelerationCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = {};

      return this->SFT();
    };

  };
  
};
