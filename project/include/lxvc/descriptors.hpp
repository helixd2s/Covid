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
    using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    friend DeviceObj;

    // 
    vk::AccelerationStructureKHR acceleration = {};
    AllocatedMemory allocated = {};
    std::optional<AccelerationCreateInfo> cInfo = {};
    MSS infoMap = {};

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
      this->infoMap = {};

      return this->SFT();
    };

  };

};
