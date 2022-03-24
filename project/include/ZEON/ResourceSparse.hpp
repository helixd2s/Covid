#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

// 
namespace ZNAMED {

  //
  struct SparseMemoryPage {
    vk::SparseMemoryBind bind = {};
    std::function<void()> destructor = {};

    //
    SparseMemoryPage(vk::SparseMemoryBind const& bind, std::function<void()> const& destructor = {}) : bind(bind), destructor(destructor) {

    };

    //
    ~SparseMemoryPage() {
      if (this->destructor) {
        this->destructor();
      };
      this->destructor = {};
    };
  };

  // 
  class ResourceSparseObj : public ResourceObj {
  public: 
    using tType = WrapShared<ResourceSparseObj>;
    using ResourceObj::ResourceObj;

  protected:
    //using BaseObj;
    friend DeviceObj;
    friend DescriptorsObj;
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
    void createBuffer(cpp21::const_wrap_arg<BufferCreateInfo> cInfo = {}) override {
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();

      //
      decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryBufferCreateInfo, vk::ExternalMemoryBufferCreateInfo{
        .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
        });

      // 
      decltype(auto) bufferUsage = this->handleBufferUsage(cInfo->type);
      decltype(auto) bufferInfo = infoMap->set(vk::StructureType::eBufferCreateInfo, vk::BufferCreateInfo{
        .pNext = memoryUsage == MemoryUsage::eGpuOnly ? externalInfo.get() : nullptr,
        .flags = vk::BufferCreateFlagBits::eSparseResidency | vk::BufferCreateFlagBits::eSparseBinding,
        .size = cpp21::tiled(cInfo->size, pageSize) * pageSize,
        .usage = bufferUsage,
        .sharingMode = vk::SharingMode::eExclusive
        });

      //
      decltype(auto) bindSparseInfo = infoMap->set(vk::StructureType::eBindSparseInfo, vk::BindSparseInfo{});

      // not-mask (i.e. incompatible)
      bufferInfo->usage &= ~vk::BufferUsageFlagBits::eShaderDeviceAddress;
      memoryUsage = MemoryUsage::eGpuOnly;

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{}).get()
        });

      //
      device.getBufferMemoryRequirements2(infoMap->set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{
        .buffer = (this->handle = this->cInfo->buffer ? this->cInfo->buffer.value() : device.createBuffer(bufferInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices)))
        }).get(), memReqInfo2.get());

      //
      sparseBufferBindInfo[0u].buffer = this->handle.as<vk::Buffer>();

      // 
      memoryRequirements = memReqInfo2->memoryRequirements;

      //
      destructors.push_back([device, buffer = this->handle.as<vk::Buffer>()](BaseObj const*) {
        device.waitIdle();
        device.destroyBuffer(buffer);
      });
    };

  public:
    // 
    ResourceSparseObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : ResourceObj(deviceObj, cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    ResourceSparseObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : ResourceObj(handle, cInfo) {
      //this->construct(ZNAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    // 
     std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
     WrapShared<ResourceObj> registerSelf() override {
      ZNAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return std::dynamic_pointer_cast<ResourceObj>(shared_from_this());
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
      auto shared = std::make_shared<ResourceSparseObj>(handle, cInfo);
      shared->construct(ZNAMED::context->get<DeviceObj>(handle), cInfo);
      auto wrap = shared->registerSelf();
      return tType(shared);
    };

  protected:

    //
    virtual std::shared_ptr<SparseMemoryPage> allocatePage(uintptr_t pageOffsetId) {
      //
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();

      // 
      decltype(auto) memReq = memoryRequirements; memReq.size = pageSize;
      decltype(auto) allocated = this->allocateMemory(this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .requirements = memReq,
        .hasDeviceAddress = false,
        .needsDestructor = false
      });

      // 
      this->sparseMemoryPages->push_back(std::make_shared<SparseMemoryPage>(vk::SparseMemoryBind{
        .resourceOffset = pageSize * pageOffsetId,
        .size = pageSize,
        .memory = allocated->memory,
        .memoryOffset = allocated->offset
      }, [device, allocated](){
        device.waitIdle();
        device.freeMemory(allocated->memory);
      }));

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
    virtual FenceType bindSparseMemory(cpp21::const_wrap_arg<SubmissionInfo> submission = {}){
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) queue = deviceObj->getQueue(submission->info);
      decltype(auto) bindSparseInfo = infoMap->get<vk::BindSparseInfo>(vk::StructureType::eBindSparseInfo);

      //
      std::vector<vk::Semaphore> waitSemaphores = {};
      std::vector<vk::Semaphore> signalSemaphores = {};
      std::vector<vk::SparseMemoryBind> sparseMemoryBinds = {};

      //
      for (auto& semInfo : *(submission->waitSemaphores)) {
        waitSemaphores.push_back(semInfo.semaphore);
      };

      //
      for (auto& semInfo : *(submission->signalSemaphores)) {
        signalSemaphores.push_back(semInfo.semaphore);
      };

      //
      for (auto& pageInfo : (*sparseMemoryPages)) {
        sparseMemoryBinds.push_back(pageInfo->bind);
      };

      // 
      sparseBufferBindInfo[0].setBinds(sparseMemoryBinds);
      if (bindSparseInfo) {
        bindSparseInfo->setBufferBinds(sparseBufferBindInfo);
      };

      // 
      decltype(auto) fences = deviceObj->getFences();
      fences.push_back(std::make_shared<FenceStatus>(device, device.createFence(vk::FenceCreateInfo{ .flags = {} })));
      if (bindSparseInfo) {
        queue.bindSparse(std::vector<vk::BindSparseInfo>{bindSparseInfo}, fences.back()->fence);
      };
      deviceObj->tickProcessing();
      return fences.back();
    };

  public:

  };

  //
  inline vk::Buffer& DescriptorsObj::createCacheBuffer() {
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
    this->cacheBufferObj->bindSparseMemory(SubmissionInfo{
      .info = QueueGetInfo{0u, 0u}
    });

    //
    return this->cacheBuffer;
  };

  
};
