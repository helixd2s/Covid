#pragma once

// 
#include "./core.hpp"
#include "./device.hpp"

// 
namespace lxvc {

  // 
  class ImageObj : std::enable_shared_from_this<ImageObj> {
  public:
    using tType = std::shared_ptr<ImageObj>;
    using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    friend InstanceObj;

    // 
    vk::Image image = {};
    AllocatedMemory allocated = {};
    ImageCreateInfo cInfo = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    ImageObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<ImageCreateInfo> cInfo = ImageCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<ImageCreateInfo> cInfo = ImageCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      

      return this->SFT();
    };

  };
  
};
