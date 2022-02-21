#pragma once

// 
#include "./core.hpp"
#include "./device.hpp"

// 
namespace lxvc {

  // 
  class BufferObj : std::enable_shared_from_this<BufferObj> {
  public:
    using tType = std::shared_ptr<BufferObj>;
    using MSS = cpp21::map_of_shared<vk::StructureType, vk::BaseInStructure>;
    friend InstanceObj;

    // 
    vk::Buffer buffer = {};
    AllocatedMemory allocated = {};
    std::optional<BufferCreateInfo> cInfo = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    BufferObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<BufferCreateInfo> cInfo = BufferCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };
    
    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<BufferCreateInfo> cInfo = BufferCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      

      return this->SFT();
    };

  };
  
};
