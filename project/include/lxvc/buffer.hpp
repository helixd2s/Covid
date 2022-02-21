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
    std::optional<AllocatedMemory> allocated = {};
    std::optional<BufferCreateInfo> cInfo = {};
    std::optional<MemoryRequirements> mReqs = {};
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
    virtual std::optional<AllocatedMemory>& allocateMemory(cpp21::optional_ref<MemoryRequirements> requirements) {
      decltype(auto) physicalDevice = this->deviceObj->physicalDevices[this->deviceObj->cInfo->physicalDeviceIndex];
      decltype(auto) memTypeHeap = this->deviceObj->findMemoryTypeAndHeapIndex(physicalDevice, *requirements);
      decltype(auto) allocated = this->allocated;

      // 
      allocated = AllocatedMemory{
        .memory = this->deviceObj->device.allocateMemory(infoMap.set(vk::StructureType::eMemoryAllocateInfo, vk::MemoryAllocateInfo{
          .pNext = infoMap.set(vk::StructureType::eMemoryDedicatedAllocateInfo, vk::MemoryDedicatedAllocateInfo{ .buffer = requirements->dedicated ? this->buffer : vk::Buffer{} }),
          .allocationSize = requirements->size,
          .memoryTypeIndex = std::get<0>(memTypeHeap)
        })),
        .offset = 0ull,
        .size = requirements->size
      };

      // 
      return allocated;
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

      default:;
      };

      // 
      decltype(auto) bufferInfo = infoMap.set(vk::StructureType::eBufferCreateInfo, vk::BufferCreateInfo{
        .size = this->cInfo->size,
        .usage = bufferUsage
      });

      // 
      decltype(auto) memReqInfo2 = infoMap.set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap.set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{})
      });

      //
      this->deviceObj->device.getBufferMemoryRequirements2(infoMap.set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{
        .buffer = (this->buffer = this->deviceObj->device.createBuffer(bufferInfo->setQueueFamilyIndices(*this->deviceObj->cInfo->queueFamilyIndices)))
      }).get(), memReqInfo2.get());

      // 
      decltype(auto) memReqInfo = memReqInfo2->memoryRequirements;
      this->allocated = this->allocateMemory(opt_ref((this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .memoryTypeBits = memReqInfo.memoryTypeBits,
        .size = memReqInfo.size
      }).value()));

      //
      std::vector<vk::BindBufferMemoryInfo> bindInfos = { *infoMap.set(vk::StructureType::eBindBufferMemoryInfo, vk::BindBufferMemoryInfo{
        .buffer = this->buffer, .memory = this->allocated->memory, .memoryOffset = this->allocated->offset
      }) };
      this->deviceObj->device.bindBufferMemory2(bindInfos);

      // 
      return this->SFT();
    };

  };
  
};
