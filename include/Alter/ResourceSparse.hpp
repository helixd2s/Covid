#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

// 
namespace ANAMED {

  

  // 
  class ResourceSparseObj : public ResourceObj {
  public: 
    using tType = WrapShared<ResourceSparseObj>;
    using ResourceObj::ResourceObj;

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
    cpp21::vector_of_shared<SparseMemoryPage> sparseMemoryPages = std::vector<std::shared_ptr<SparseMemoryPage>>{};

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

      //
      decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryBufferCreateInfo, vk::ExternalMemoryBufferCreateInfo{
        .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
        });

      //
      decltype(auto) bufferInfo = this->makeBufferCreateInfo(cInfo);
      decltype(auto) bufferUsage = bufferInfo->usage;
      decltype(auto) bindSparseInfo = infoMap->set(vk::StructureType::eBindSparseInfo, vk::BindSparseInfo{});

      //
      bufferInfo->flags |= vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency | vk::BufferCreateFlagBits::eSparseAliased;
      bufferInfo->size = cpp21::tiled(cInfo->size, pageSize) * pageSize;
      bufferInfo->usage &= ~vk::BufferUsageFlagBits::eShaderDeviceAddress;

      //
      memoryUsage = MemoryUsage::eGpuOnly;

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{}).get()
        });

      //
      device.getBufferMemoryRequirements2(infoMap->set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{
        .buffer = (this->handle = this->cInfo->buffer ? this->cInfo->buffer.value() : device.createBuffer(bufferInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices))).as<vk::Buffer>()
        }).get(), memReqInfo2.get());

      //
      sparseBufferBindInfo[0u].buffer = this->handle.as<vk::Buffer>();

      // 
      memoryRequirements = memReqInfo2->memoryRequirements;

      //
      destructors.push_back(std::make_shared<std::function<DFun>>([device, buffer = this->handle.as<vk::Buffer>()](BaseObj const*) {
        //device.waitIdle();
        device.destroyBuffer(buffer);
      }));

      // 
      return FenceType{};
    };

  public:
    // 
    ResourceSparseObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : ResourceObj(deviceObj, cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    ResourceSparseObj(Handle const& handle, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : ResourceObj(handle, cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    // 
     std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    WrapShared<ResourceObj> registerSelf() override {
      ANAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return std::dynamic_pointer_cast<ResourceObj>(shared_from_this());
    };

    //
    inline static tType make(Handle const& handle, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
      auto shared = std::make_shared<ResourceSparseObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
      return std::dynamic_pointer_cast<ResourceSparseObj>(shared->registerSelf().shared());
    };

  protected:

    //
    virtual std::shared_ptr<SparseMemoryPage> allocatePage(uintptr_t pageOffsetId) {
      //
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();

      // 
      decltype(auto) memReq = memoryRequirements; memReq.size = pageSize;
      decltype(auto) allocated = this->allocateMemory(MemoryRequirements{
        .memoryUsage = memoryUsage,
        .hasDeviceAddress = false,
        .needsDestructor = false,
        .requirements = std::optional<vk::MemoryRequirements>(memReq)
      });

      // 
      this->sparseMemoryPages->push_back(std::make_shared<SparseMemoryPage>(vk::SparseMemoryBind{
        .resourceOffset = pageSize * pageOffsetId,
        .size = pageSize,
        .memory = allocated->memory,
        .memoryOffset = allocated->offset
      }, allocated->mapped, [device, allocated](){
        //device.waitIdle();
        if (allocated && allocated->destructor) {
          (*allocated->destructor)(nullptr);
        };
      }));

      //
      this->sparseMemoryPages->back()->allocated = allocated;

      //
      return this->sparseMemoryPages->back();
    };

    

    //
    virtual cpp21::vector_of_shared<SparseMemoryPage>& getSparseMemoryBinds() {
      return sparseMemoryPages;
    };

    //
    virtual cpp21::vector_of_shared<SparseMemoryPage> const& getSparseMemoryBinds() const {
      return sparseMemoryPages;
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
      std::vector<vk::SparseMemoryBind> sparseMemoryBinds = {};

      //
      for (auto& semInfo : *(submission.waitSemaphores)) {
        waitSemaphores.push_back(semInfo.semaphore);
      };

      //
      for (auto& semInfo : *(submission.signalSemaphores)) {
        signalSemaphores.push_back(semInfo.semaphore);
      };

      // 
      for (auto& pageInfo : (*sparseMemoryPages)) {
        sparseMemoryBinds.push_back(pageInfo->bind);
      };
      sparseMemoryPages = std::vector<std::shared_ptr<SparseMemoryPage>>{};

      // 
      sparseBufferBindInfo[0].setBinds(sparseMemoryBinds);
      if (bindSparseInfo) {
        bindSparseInfo->setBufferBinds(sparseBufferBindInfo);
      };

      // 
      decltype(auto) fences = deviceObj->getFences();
      decltype(auto) fence = std::make_shared<vk::Fence>(device.createFence(vk::FenceCreateInfo{ .flags = {} }));

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
          auto status = device.getFenceStatus(*fence);
          if (fn) { cl->add(std::bind(fn, status)); };
        };
        cl->add(deAllocation);
      };

      //
      fences.push_back(std::make_shared<FenceStatus>([device, fence]() {
        if (fence && *fence) { return device.getFenceStatus(*fence) != vk::Result::eNotReady; };
        return true;
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

  //
  inline vk::Buffer& PipelineLayoutObj::createCacheBuffer() {
    this->cacheBuffer = (this->cacheBufferObj = ResourceSparseObj::make(this->base, ResourceCreateInfo{
      .bufferInfo = BufferCreateInfo{
        .size = this->cachePages * this->cachePageSize,
        .type = BufferType::eStorage
      }
    })).as<vk::Buffer>();

    //
    for (uint32_t i = 0; i < this->cachePages; i++) {
      this->cacheBufferObj->allocatePage(i);
      this->cacheBufferDescs.push_back(vk::DescriptorBufferInfo{ this->cacheBuffer, i * this->cachePageSize, this->cachePageSize });
    };

    //
    auto status = this->cacheBufferObj->bindSparseMemory(SubmissionInfo{
      .info = this->cInfo->info ? this->cInfo->info.value() : QueueGetInfo{this->base.family, 0u}
    });

    //
    return this->cacheBuffer;
  };

  
};
#endif
