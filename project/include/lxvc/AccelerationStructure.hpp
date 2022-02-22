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
    AccelerationStructureObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<AccelerationCreateInfo> cInfo = AccelerationCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
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
