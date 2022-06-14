#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"

//
#ifdef ALT_ENABLE_VMA
#include "./MemoryAllocatorVma.hpp"
#endif

// 
namespace ANAMED {
  
  // 
  class UploaderObj : public BaseObj {
  public: 
    //using BaseObj;
    using tType = WrapShared<UploaderObj>;
    using BaseObj::BaseObj;

  protected: 
    friend DeviceObj;
    friend PipelineObj;
    friend ResourceObj;
    friend GeometryLevelObj;
    friend InstanceLevelObj;

    // 
    cpp21::vector_of_shared<MSS> layoutInfoMaps = {};
    std::optional<UploaderCreateInfo> cInfo = UploaderCreateInfo{};
    
    //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
    VmaVirtualBlock mappedBlock;
#endif

    //
    vk::Buffer mappedBuffer = {};

    //
    size_t pageSize = 65536u;

    //
    vk::MemoryRequirements memoryRequirements = {};
    std::vector<vk::SparseBufferMemoryBindInfo> sparseBufferBindInfo = { vk::SparseBufferMemoryBindInfo{} };
    cpp21::vector_of_shared<SparseMemoryPage> sparseMemoryPages = std::vector<std::shared_ptr<SparseMemoryPage>>{};
    void* mappedMemory = nullptr;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:

    // 
    UploaderObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    UploaderObj(Handle const& handle, cpp21::optional_ref<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    // 
     std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual tType registerSelf() {
      ANAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(Handle const& handle, cpp21::optional_ref<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      auto shared = std::make_shared<UploaderObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual vk::MemoryRequirements& getMemoryRequirements() {
      return memoryRequirements;
    };

    //
    virtual vk::MemoryRequirements const& getMemoryRequirements() const {
      return memoryRequirements;
    };

    // you can copy from host to device Buffer and Image together!
    // TODO: per-type role based barriers...
    // TODO: image, imageType and imageLayout supports...
    virtual tType writeDownloadToResourceCmd(cpp21::optional_ref<DownloadCommandWriteInfo> info);
    virtual tType writeUploadToResourceCmd(cpp21::optional_ref<UploadCommandWriteInfo> copyRegionInfo);

    //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
    virtual VmaVirtualBlock& getMappedBlock() { return mappedBlock; };
    virtual VmaVirtualBlock const& getMappedBlock() const { return mappedBlock; };

    //
    virtual VmaVirtualAllocation allocateMappedTemp(size_t const& size, uintptr_t& offset) {
      // 
      VmaVirtualAllocationCreateInfo allocCreateInfo = {};
      allocCreateInfo.size = size; // 4 KB
      allocCreateInfo.flags = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT | VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT | VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
      allocCreateInfo.alignment = memoryRequirements.alignment;

      //
      VmaVirtualAllocation alloc;
      VkResult upRes = vmaVirtualAllocate(mappedBlock, &allocCreateInfo, &alloc, &offset);

      //
      return alloc;
    };

    //
    virtual std::shared_ptr<AllocatedMemory> allocateMemory(cpp21::optional_ref<MemoryRequirements> requirements) {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorObj>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocator);
      decltype(auto) allocated = std::make_shared<AllocatedMemory>();
      return memoryAllocatorObj->allocateMemory(requirements, allocated);
    };

    //
    virtual std::shared_ptr<SparseMemoryPage> allocatePage(uintptr_t bufferOffset, uintptr_t memorySize) {
      //
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) bufferInfo = infoMap->get<vk::BufferCreateInfo>(vk::StructureType::eBufferCreateInfo);
      decltype(auto) memReqInfo2 = infoMap->get<vk::MemoryRequirements2>(vk::StructureType::eMemoryRequirements2);
      bufferInfo->size = memorySize;

      //
      vk::DeviceBufferMemoryRequirements memReqIn = vk::DeviceBufferMemoryRequirements{ .pCreateInfo = bufferInfo.get() };
      //vkGetDeviceBufferMemoryRequirements(VkDevice(device), reinterpret_cast<VkDeviceBufferMemoryRequirements*>(&memReqIn), reinterpret_cast<VkMemoryRequirements2*>(memReqInfo2.get()));
      device.getBufferMemoryRequirements(&memReqIn, memReqInfo2.get());

      // 
      decltype(auto) memReq = memReqInfo2->memoryRequirements; memReq.size = memorySize;
      decltype(auto) allocated = this->allocateMemory(MemoryRequirements{
        .memoryUsage = MemoryUsage::eCpuToGpu,
        .requirements = memReq,
        .hasDeviceAddress = false,
        .needsDestructor = true
        });

      //
      decltype(auto) sparsePage = std::make_shared<SparseMemoryPage>();
      sparsePage->bind = vk::SparseMemoryBind{
        .resourceOffset = bufferOffset,
        .size = memReq.size,
        .memory = allocated->memory,
        .memoryOffset = allocated->offset
      };
      sparsePage->mapped = allocated->mapped;
      sparsePage->bunchBuffer = device.createBuffer(bufferInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices));
      sparsePage->destructor = [device, allocated, &buffer=sparsePage->bunchBuffer]() {
        //device.waitIdle();
        if (allocated && allocated->destructor) {
          (*allocated->destructor)(nullptr);
        };
        if (buffer) {
          device.destroyBuffer(buffer); buffer = vk::Buffer{};
        };
      };
      sparsePage->allocated = allocated;

      // 
      std::vector<vk::BindBufferMemoryInfo> bindInfos = { *infoMap->set(vk::StructureType::eBindBufferMemoryInfo, vk::BindBufferMemoryInfo{
        .buffer = sparsePage->bunchBuffer, .memory = allocated->memory, .memoryOffset = allocated->offset
      }) };
      device.bindBufferMemory2(bindInfos);

      // 
      this->sparseMemoryPages->push_back(sparsePage);

      //
      return this->sparseMemoryPages->back();
    };

    //
    virtual FenceType bindMemoryPages(SubmissionInfo const& submission = {}) {
      decltype(auto) bindSparseInfo = infoMap->get<vk::BindSparseInfo>(vk::StructureType::eBindSparseInfo);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) queue = deviceObj->getQueue(submission.info.value());

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

      // 
      sparseBufferBindInfo[0].setBinds(sparseMemoryBinds);
      if (bindSparseInfo) {
        bindSparseInfo->setBufferBinds(sparseBufferBindInfo);
      };
      sparseMemoryPages = std::vector<std::shared_ptr<SparseMemoryPage>>{};

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
        for (auto& fn : submission.onDone) {
          cl->add(std::bind(fn, device.getFenceStatus(*fence)));
        };
        cl->add(deAllocation);
      };

      //
      fences.push_back(std::make_shared<FenceStatus>([device, fence]() {
        if (fence && *fence) { return device.getFenceStatus(*fence) != vk::Result::eNotReady; };
        return true;
        }, onDone));
      decltype(auto) status = fences.back();
      if (bindSparseInfo && mappedBuffer) {
        //queue.bindSparse(std::vector<vk::BindSparseInfo>{bindSparseInfo}, * fence);
      };
      deviceObj->tickProcessing();
      return status;
    };

    //
    virtual std::tuple<VkDeviceSize, VmaVirtualAllocation> allocateMappedTemp(size_t const& size) {
      VkDeviceSize offset = 0ull;
      return std::make_tuple(offset, allocateMappedTemp(size, offset));
    };
#endif

    //
    virtual size_t getImagePixelSize(vk::Image const& image);

    //
    virtual FenceType executeUploadToResourceOnce(UploadExecutionOnce const& exec) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = this->cInfo->info } };
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) size = exec.host ? (exec.writeInfo.dstBuffer ? std::min(exec.host.size(), exec.writeInfo.dstBuffer->region.size) : exec.host.size()) : (exec.writeInfo.dstBuffer ? exec.writeInfo.dstBuffer->region.size : VK_WHOLE_SIZE);
      decltype(auto) mappedBuffer = this->mappedBuffer;

      //
      if (exec.writeInfo.dstImage) {
        decltype(auto) pixelCount = size_t(exec.writeInfo.dstImage->region.extent.width) * size_t(exec.writeInfo.dstImage->region.extent.height) * size_t(exec.writeInfo.dstImage->region.extent.depth) * size_t(exec.writeInfo.dstImage->region.layerCount);
        size = std::min(size, pixelCount * getImagePixelSize(exec.writeInfo.dstImage->image));
      };

      // 
      VkDeviceSize offset = exec.writeInfo.hostMapOffset;

      // 
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      decltype(auto) allocSize = cpp21::tiled(size, memoryRequirements.alignment) * memoryRequirements.alignment;
      decltype(auto) alloc = allocateMappedTemp(allocSize, offset);
      decltype(auto) memPage = allocatePage(offset, allocSize);//allocatePage
#endif

      // 
      if (exec.host) {
        memcpy(memPage->mapped, exec.host.data(), size);
      };

      // 
      submission.commandInits.push_back([exec, offset, memPage, this](vk::CommandBuffer const& cmdBuf) {
        this->writeUploadToResourceCmd(exec.writeInfo.with(cmdBuf, memPage->bunchBuffer).mapOffset(offset));
        return cmdBuf;
      });

      //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      submission.submission.onDone.push_back([memPage, mappedBlock=this->mappedBlock, alloc](cpp21::optional_ref<vk::Result> result) {
        vmaVirtualFree(mappedBlock, alloc);
        memPage->destructor();
      });
#endif

      //
      this->bindMemoryPages(submission.submission);
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType executeDownloadToResourceOnce(DownloadExecutionOnce const& exec) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = this->cInfo->info } };
      decltype(auto) mappedBuffer = this->mappedBuffer;
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) size = exec.host ? (exec.writeInfo.srcBuffer ? std::min(exec.host.size(), exec.writeInfo.srcBuffer->region.size) : exec.host.size()) : (exec.writeInfo.srcBuffer ? exec.writeInfo.srcBuffer->region.size : VK_WHOLE_SIZE);
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      // 
      VkDeviceSize offset = exec.writeInfo.hostMapOffset;

      // 
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      decltype(auto) allocSize = cpp21::tiled(size, memoryRequirements.alignment) * memoryRequirements.alignment;
      decltype(auto) alloc = allocateMappedTemp(allocSize, offset);
      decltype(auto) memPage = allocatePage(offset, allocSize);//allocatePage
#endif

      // 
      submission.commandInits.push_back([exec, memPage, offset, this](vk::CommandBuffer const& cmdBuf) {
        this->writeDownloadToResourceCmd(exec.writeInfo.with(cmdBuf, memPage->bunchBuffer).mapOffset(offset));
        return cmdBuf;
      });

      //
      if (exec.host) {
        submission.submission.onDone.push_back([offset, size, _host = exec.host, mapped = memPage->mapped](cpp21::optional_ref<vk::Result> result) {
          memcpy(_host.data(), cpp21::shift(mapped, 0), size);
        });

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        submission.submission.onDone.push_back([mappedBlock = this->mappedBlock, alloc](cpp21::optional_ref<vk::Result> result) {
          vmaVirtualFree(mappedBlock, alloc);
        });
#endif
      };

      //
      this->bindMemoryPages(submission.submission);
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };


  protected:

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<UploaderCreateInfo> cInfo = UploaderCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      //this->deviceObj = deviceObj;

      //
      //decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      
      // 
      decltype(auto) bufferInfo = infoMap->set(vk::StructureType::eBufferCreateInfo, vk::BufferCreateInfo{
        .pNext = nullptr,
        .size = this->cInfo->cacheSize,
        .usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive
        });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{}).get()
        });

      //
      decltype(auto) bindSparseInfo = infoMap->set(vk::StructureType::eBindSparseInfo, vk::BindSparseInfo{});

      //
      vk::DeviceBufferMemoryRequirements memReqIn = vk::DeviceBufferMemoryRequirements{ .pCreateInfo = bufferInfo.get() };
      //vkGetDeviceBufferMemoryRequirements(VkDevice(device), reinterpret_cast<VkDeviceBufferMemoryRequirements*>(&memReqIn), reinterpret_cast<VkMemoryRequirements2*>(memReqInfo2.get()));
      device.getBufferMemoryRequirements(&memReqIn, memReqInfo2.get());
      //device.getBufferMemoryRequirements2(infoMap->set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{
        //.buffer = (this->mappedBuffer = device.createBuffer(bufferInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices)))
        //}).get(), memReqInfo2.get());

      //
      destructors.push_back(std::make_shared<std::function<DFun>>([device, buffer = this->mappedBuffer](BaseObj const*) {
        //device.waitIdle();
        device.destroyBuffer(buffer);
      }));

      //
      sparseBufferBindInfo[0u].buffer = this->mappedBuffer;
      memoryRequirements = memReqInfo2->memoryRequirements;

#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      // 
      VmaVirtualBlockCreateInfo blockCreateInfo = {};
      blockCreateInfo.size = this->cInfo->cacheSize;
      //blockCreateInfo.flags = VMA_VIRTUAL_BLOCK_CREATE_LINEAR_ALGORITHM_BIT;

      // 
      VkResult result = vmaCreateVirtualBlock(&blockCreateInfo, &mappedBlock);
#endif

      // 
      this->handle = uintptr_t(this);//Handle(uintptr_t(this), HandleType::eUploader);
      this->handle.type = HandleType::eUploader; // Unable to Map without specific type
    };
  };

};
#endif
