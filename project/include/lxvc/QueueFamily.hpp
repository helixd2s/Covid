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
    using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    friend DeviceObj;

    // 
    std::vector<vk::Queue> queues = {};
    std::optional<QueueFamilyCreateInfo> cInfo = {};
    MSS infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    QueueFamilyObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<QueueFamilyCreateInfo> cInfo = QueueFamilyCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = {};

      return this->SFT();
    };

  };
  
};
