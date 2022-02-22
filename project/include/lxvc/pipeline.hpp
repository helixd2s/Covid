#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {

  // 
  class PipelineObj : std::enable_shared_from_this<PipelineObj> {
  public:
    using tType = std::shared_ptr<PipelineObj>;
    using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    friend DeviceObj;

    // 
    vk::Pipeline pipeline = {};
    AllocatedMemory allocated = {};
    std::optional<PipelineCreateInfo> cInfo = {};
    MSS infoMap = {};

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
      this->infoMap = {};

      return this->SFT();
    };

  };
  
};
