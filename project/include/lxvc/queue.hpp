#pragma once

// 
#include "./core.hpp"
#include "./device.hpp"

// 
namespace lxvc {
  
  // 
  class QueueObj : std::enable_shared_from_this<QueueObj> {
  public:
    using tType = std::shared_ptr<QueueObj>;
    using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    friend DeviceObj;

    // 
    vk::Queue queue = {};
    std::optional<QueueCreateInfo> cInfo = {};
    MSS infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    QueueObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<QueueCreateInfo> cInfo = QueueCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<QueueCreateInfo> cInfo = QueueCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = {};

      return this->SFT();
    };

  };
  
};
