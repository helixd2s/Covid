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
    friend DeviceObj;

    // 
    vk::Buffer buffer = {};
    AllocatedMemory allocated = {};
    std::optional<BufferCreateInfo> cInfo = {};
    MSS infoMap = {};

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
      this->infoMap = {};

      // 
      auto bufferUsage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
      auto memoryUsage = MemoryUsage::eGpuOnly;

      // 
      switch (this->cInfo->type) {
      case BufferType::eDevice:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |=
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT |
          vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT |
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
          vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
          vk::BufferUsageFlagBits::eShaderBindingTableKHR |
          vk::BufferUsageFlagBits::eStorageTexelBuffer;
        break;

      case BufferType::eHostMap:
        memoryUsage = MemoryUsage::eCpuToGpu;
        bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eUniformTexelBuffer;
        break;

      case BufferType::eUniform:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eUniformTexelBuffer;
        break;

      default:
      };

      decltype(auto) bufferInfo = infoMap.set(vk::StructureType::eBufferCreateInfo, vk::BufferCreateInfo{
        .size = this->cInfo->size,
        .usage = bufferUsage
      });

      this->buffer = this->deviceObj->device.createBuffer(bufferInfo->setQueueFamilyIndices(*this->deviceObj->cInfo->queueFamilyIndices));
      

      return this->SFT();
    };

  };
  
};
