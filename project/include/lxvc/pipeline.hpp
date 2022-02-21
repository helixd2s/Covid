#pragma once

// 
#include "./core.hpp"
#include "./device.hpp"

// 
namespace lxvc {

  // 
  class PipelineObj : std::enable_shared_from_this<PipelineObj> {
  public:
    using tType = std::shared_ptr<PipelineObj>;
    using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    friend InstanceObj;

    // 
    vk::Pipeline pipeline = {};
    AllocatedMemory allocated = {};
    PipelineCreateInfo cInfo = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    PipelineObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      

      return this->SFT();
    };

  };
  
};
