#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Device.hpp"
#include "./ResourceBuffer.hpp"

// 
namespace ANAMED {

  

  // 
  class ResourceSparseObj : public ResourceBufferObj {
  public: 
    using tType = WrapShared<ResourceSparseObj>;
    using ResourceBufferObj::ResourceBufferObj;

  protected:
    //using BaseObj;
    friend DeviceObj;
    friend PipelineLayoutObj;
    friend UploaderObj;
    friend FramebufferObj;
    friend SwapchainObj;
    friend GeometryLevelObj;
    friend InstanceLevelObj;
    friend MemoryAllocatorObj;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    //
    cpp21::vector_of_shared<AllocatedMemory> allocatedMemories = std::vector<std::shared_ptr<AllocatedMemory>>{};
    std::vector<vk::SparseMemoryBind> sparseMemoryBinds = std::vector<vk::SparseMemoryBind>{};


    //
    size_t pageSize = 65536u;

    //
    vk::MemoryRequirements memoryRequirements = {};
    std::vector<vk::SparseBufferMemoryBindInfo> sparseBufferBindInfo = { vk::SparseBufferMemoryBindInfo{} };

  protected:

    // 
    FenceType createBuffer(cpp21::optional_ref<BufferCreateInfo> cInfo = {}) override {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) bufferInfo = this->makeBufferCreateInfo(cInfo);
      decltype(auto) bufferUsage = bufferInfo->usage;
      decltype(auto) bindSparseInfo = infoMap->set(vk::StructureType::eBindSparseInfo, vk::BindSparseInfo{});
      decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryBufferCreateInfo, vk::ExternalMemoryBufferCreateInfo{
        .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
      });

      //
      bufferInfo->flags |= vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency | vk::BufferCreateFlagBits::eSparseAliased;
      bufferInfo->size = cpp21::tiled(cInfo->size, pageSize) * pageSize;
      bufferInfo->usage &= ~vk::BufferUsageFlagBits::eShaderDeviceAddress;

      //
      decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorObj>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocator);

      //
      this->allocated = std::make_shared<AllocatedMemory>();
      if (this->cInfo->buffer && *this->cInfo->buffer) {
          this->handle = this->cInfo->buffer.value();
      }
      else {
          this->handle = memoryAllocatorObj->createBuffer(infoMap, destructors);
      };

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{}).get()
      });

      //
      device.getBufferMemoryRequirements2(infoMap->set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{ .buffer = this->handle.as<vk::Buffer>() }).get(), memReqInfo2.get());

      //
      sparseBufferBindInfo[0u].buffer = this->handle.as<vk::Buffer>();
      memoryRequirements = memReqInfo2->memoryRequirements;
      memoryUsage = MemoryUsage::eGpuOnly;

      // 
      return FenceType{};
    };

  public:
    // 
    ResourceSparseObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<BufferCreateInfo> cInfo = BufferCreateInfo{}) : ResourceBufferObj(deviceObj, cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    ResourceSparseObj(Handle const& handle, cpp21::optional_ref<BufferCreateInfo> cInfo = BufferCreateInfo{}) : ResourceBufferObj(handle, cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    // 
     std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    WrapShared<ResourceBufferObj> registerSelf() override {
      ANAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return std::dynamic_pointer_cast<ResourceBufferObj>(shared_from_this());
    };

    //
    inline static tType make(Handle const& handle, cpp21::optional_ref<BufferCreateInfo> cInfo = BufferCreateInfo{}) {
      auto shared = std::make_shared<ResourceSparseObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
      return std::dynamic_pointer_cast<ResourceSparseObj>(shared->registerSelf().shared());
    };

  protected:

    //
    virtual vk::SparseMemoryBind allocatePage(uintptr_t pageOffsetId) {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) memReq = memoryRequirements; memReq.size = pageSize;
      decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorObj>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocator);

      //
      std::shared_ptr<AllocatedMemory> allocated = std::make_shared<AllocatedMemory>();
      this->handle = memoryAllocatorObj->allocateMemory(allocated, MemoryRequirements{ .requirements = memReq },infoMap);

      //
      this->sparseMemoryBinds.push_back(vk::SparseMemoryBind{ 
        .resourceOffset = pageSize * pageOffsetId,
        .size = pageSize,
        .memory = allocated->memory,
        .memoryOffset = allocated->offset 
      });
      this->allocatedMemories->push_back(allocated);

      //
      return this->sparseMemoryBinds.back();
    };

    // 
    virtual FenceType bindSparseMemory(SubmissionInfo const& submission = {}){
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) queue = deviceObj->getQueue(submission.info.value());
      decltype(auto) bindSparseInfo = infoMap->get<vk::BindSparseInfo>(vk::StructureType::eBindSparseInfo);

      //
      std::vector<vk::Semaphore> waitSemaphores = {};
      std::vector<vk::Semaphore> signalSemaphores = {};

      //
      for (auto& semInfo : *(submission.waitSemaphores)) {
        waitSemaphores.push_back(semInfo.semaphore);
      };

      //
      for (auto& semInfo : *(submission.signalSemaphores)) {
        signalSemaphores.push_back(semInfo.semaphore);
      };

      // 
      sparseBufferBindInfo[0].setBinds(sparseMemoryBinds);
      if (bindSparseInfo) {
        bindSparseInfo->setBufferBinds(sparseBufferBindInfo);
      };

      // 
      decltype(auto) fences = deviceObj->getFences();
      decltype(auto) fence = std::make_shared<vk::Fence>(handleResult(device.createFence(vk::FenceCreateInfo{ .flags = {} })));

      // 
      auto deAllocation = [device, fence]() {
        if (fence && *fence) {
          device.destroyFence(*fence);
          *fence = vk::Fence{};
        };
      };

      //
      auto onDone = [device, fence, callstack = std::weak_ptr<CallStack>(deviceObj->getCallstack()), submission, deAllocation]() {
        auto cl = callstack.lock();
        for (auto fn : submission.onDone) {
          if (fn) { cl->add(std::bind(fn, handleResult(device.getFenceStatus(*fence)))); };
        };
        cl->add(deAllocation);
      };

      //
      fences.push_back(std::make_shared<FenceStatus>([device, fence]() {
        vk::Result status = vk::Result::eSuccess;
        if (fence && *fence) { status = device.getFenceStatus(*fence); };
        return handleResult(status);
      }, onDone));
      decltype(auto) status = fences.back();
      if (bindSparseInfo) {
        queue.bindSparse(std::vector<vk::BindSparseInfo>{bindSparseInfo}, *fence);
      };
      deviceObj->tickProcessing();
      return status;
    };

  public:

  };

  /*
  //
  inline vk::Buffer& PipelineLayoutObj::createCacheBuffer() {
      this->cacheBuffer = (this->cacheBufferObj = ResourceSparseObj::make(this->base, BufferCreateInfo{
        .size = this->cachePages * this->cachePageSize,
        .type = BufferType::eStorage
      })).as<vk::Buffer>();

      //
      for (uint32_t i = 0; i < this->cachePages; i++) {
          this->cacheBufferObj->allocatePage(i);
          this->cacheBufferDescs.push_back(vk::DescriptorBufferInfo{ this->cacheBuffer, i * this->cachePageSize, this->cachePageSize });
      };

      // currently, sparse doesn't supported
      auto status = this->cacheBufferObj->bindSparseMemory(SubmissionInfo{
        .info = this->cInfo->info ? this->cInfo->info.value() : QueueGetInfo{this->base.family, 0u}
          });

      //
      return this->cacheBuffer;
  };*/

  
};
#endif
