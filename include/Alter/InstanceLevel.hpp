#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"
#include "./GeometryLevel.hpp"

//
#ifdef ALT_ENABLE_VMA
#include "./MemoryAllocatorVma.hpp"
#endif

// 
namespace ANAMED {

  

  // 
  class InstanceLevelObj : public BaseObj {
  public: 
    using tType = WrapShared<InstanceLevelObj>;
    using BaseObj::BaseObj;
    //using BaseObj;

  protected: 
    //
    std::optional<InstanceLevelCreateInfo> cInfo = {};
    vk::Buffer instanceBuffer = {};
    vk::Buffer instanceScratch = {};
    vk::Buffer instanceBuild = {};
    vk::Buffer instanceExtBuffer = {};

    // 
    WrapShared<ResourceObj> bindInstanceBuffer = {};
    WrapShared<ResourceObj> bindInstanceScratch = {};
    WrapShared<ResourceObj> bindInstanceBuild = {};
    WrapShared<ResourceObj> bindInstanceExtBuffer = {};

    //
    std::array<vk::AccelerationStructureGeometryKHR, 1> instances = {};
    std::array<vk::AccelerationStructureBuildRangeInfoKHR, 1> instanceRanges = {};

    //
    cpp21::shared_vector<InstanceDraw> instanceDraw = std::vector<InstanceDraw>{};
    cpp21::shared_vector<InstanceDevInfo> instanceDevInfo = std::vector<InstanceDevInfo>{};
    cpp21::shared_vector<InstanceInfo> instanceInfo = std::vector<InstanceInfo>{};
    //cpp21::shared_vector<bool> firstUpdate = std::vector<bool>{};

    //
    vk::AccelerationStructureKHR accelStruct = {};

    //
    InstanceAddressInfo addressInfo = { .data = 0ull, .accelStruct = 0ull };

    // 
    friend PipelineObj;
    friend SwapchainObj;
    friend InstanceLevelObj;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:

    // 
    InstanceLevelObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::carg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo), instanceDraw(std::vector<InstanceDraw>{}), instanceDevInfo(std::vector<InstanceDevInfo>{}) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    InstanceLevelObj(cpp21::carg<Handle> handle, cpp21::carg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : BaseObj(handle), cInfo(cInfo), instanceDraw(std::vector<InstanceDraw>{}), instanceDevInfo(std::vector<InstanceDevInfo>{}) {
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
    inline static tType make(cpp21::carg<Handle> handle, cpp21::carg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) {
      auto shared = std::make_shared<InstanceLevelObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual WrapShared<ResourceObj> getInstancedResource() const {
      return ANAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->instanceBuffer);
    };

    //
    virtual WrapShared<ResourceObj> getInstanceInfoResource() const {
      return ANAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->instanceExtBuffer);
    };

    //
    virtual InstanceAddressInfo& getAddressInfo() { return addressInfo; };
    virtual InstanceAddressInfo const& getAddressInfo() const { return addressInfo; };

    //
    virtual vk::Buffer& getInstancedBuffer() { return this->instanceBuffer; };
    virtual vk::Buffer const& getInstancedBuffer() const { return this->instanceBuffer; };

    //
    //virtual std::vector<InstanceDevInfo>& getDevInstances() { return this->instanceDevInfo; };
    //virtual std::vector<InstanceDevInfo> const& getDevInstances() const { return this->instanceDevInfo; };

    //
    virtual std::vector<InstanceInfo>& getInstances() { return this->instanceInfo; };
    virtual std::vector<InstanceInfo> const& getInstances() const { return this->instanceInfo; };

    //
    virtual cpp21::shared_vector<InstanceDraw>& getDrawInfo() { return this->instanceDraw; };
    virtual cpp21::shared_vector<InstanceDraw> const& getDrawInfo() const { return this->instanceDraw; };

    //
    virtual uintptr_t const& getInstanceInfoDeviceAddress() const { return this->getInstanceInfoResource()->getDeviceAddress(); };
    virtual uintptr_t& getInstanceInfoDeviceAddress() { return this->getInstanceInfoResource()->getDeviceAddress(); };

    //
    virtual uintptr_t const& getInstancedDeviceAddress() const { return this->getInstancedResource()->getDeviceAddress(); };
    virtual uintptr_t& getInstancedDeviceAddress() { return this->getInstancedResource()->getDeviceAddress(); };

    //
    virtual uintptr_t const& getDeviceAddress() const { return this->addressInfo.accelStruct; };
    virtual uintptr_t& getDeviceAddress() { return this->addressInfo.accelStruct; };

    //
    virtual void updateInstances() {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

      // 
      this->instances = {};
      this->instanceRanges = {};
      this->addressInfo.instanceCount = this->cInfo->instances->size();

      //
      if (this->instanceDraw->size() < this->cInfo->instances->size()) {
        for (uintptr_t idx = this->instanceDraw->size(); idx < this->cInfo->instances->size(); idx++) {
          this->instanceDraw->push_back(InstanceDraw{});
          this->instanceInfo->push_back(InstanceInfo{});
          this->instanceDevInfo->push_back(InstanceDevInfo{});
          //this->firstUpdate->push_back(true);
        };
      };

      // 
      for (uintptr_t idx = 0ull; idx < this->cInfo->instances->size(); idx++) {
        auto& instances = this->cInfo->instances[idx];
        auto& instanceInfo = this->instanceInfo[idx];
        auto& instanceDraw = this->instanceDraw[idx];
        auto& instanceDevInfo = this->instanceDevInfo[idx];

        //
        instanceInfo = instances.instanceInfo;
        instanceDevInfo = instances.instanceDevInfo;

        // 
        decltype(auto) geometryLevel = deviceObj->get<GeometryLevelObj>(instanceDevInfo.accelerationStructureReference);

        // 
        if (this->instanceExtBuffer && this->getInstanceInfoResource()) {
          instanceDraw.drawInfos = geometryLevel->getDrawInfo();
          instanceDraw.drawConst = InstanceDrawInfo{ .instanceCount = 1u, .instanceIndex = uint32_t(idx), .drawIndex = 0u };
        };

        // 
        //instanceInfo.prevTransform = this->firstUpdate[idx] ? reinterpret_cast<glm::mat3x4&>(instanceDevInfo.transform) : instanceInfo.transform;
        //instanceInfo.transform = reinterpret_cast<glm::mat3x4&>(instanceDevInfo.transform);
        instanceDevInfo.transform = reinterpret_cast<vk::TransformMatrixKHR&>(instanceInfo.transform);
        instanceInfo.geometryCount = geometryLevel->getGeometries().size();
        instanceInfo.geometryReference = geometryLevel->getGeometryDeviceAddress();

        //
        instances.instanceInfo = instanceInfo;
        instances.instanceDevInfo = instanceDevInfo;

        //
        //this->firstUpdate[idx] = false;
      };

      // WIP XREP ALGO
      std::vector<uintptr_t> indices(this->cInfo->instances->size());
      std::iota(indices.begin(), indices.end(), 0);
      std::sort(indices.begin(), indices.end(), [&](int i, int j) {
        return this->instanceDevInfo[i].accelerationStructureReference < this->instanceDevInfo[j].accelerationStructureReference;
      });

      //
      cpp21::apply_permutation<InstanceInfo>(this->instanceInfo, indices);
      cpp21::apply_permutation<InstanceDevInfo>(this->instanceDevInfo, indices);
      cpp21::apply_permutation<InstanceDataInfo>(this->cInfo->instances, indices);

      // WIP XREP ALGO
      // STILL MAY CRASH!!!
      this->instanceDraw = std::vector<InstanceDraw>{};
      uint32_t instanceCount = 0u;
      for (uintptr_t idx = 0ull; idx < this->cInfo->instances->size(); idx++) {
        auto& instances = this->cInfo->instances[idx];
        auto& instanceInfo = this->instanceInfo[idx];
        auto& instanceDevInfo = this->instanceDevInfo[idx];

        //
        auto& prevInstances = this->cInfo->instances[idx > 0 ? (idx - 1) : idx];
        auto& prevInstanceInfo = this->instanceInfo[idx > 0 ? (idx - 1) : idx];
        auto& prevInstanceDevInfo = this->instanceDevInfo[idx > 0 ? (idx - 1) : idx];

        //
        instanceCount++;

        // 
        decltype(auto) geometryLevel = deviceObj->get<GeometryLevelObj>(instanceDevInfo.accelerationStructureReference);

        //
        if (prevInstanceDevInfo.accelerationStructureReference != instanceDevInfo.accelerationStructureReference || (idx <= (this->cInfo->instances->size()-1))) {
          this->instanceDraw->push_back(InstanceDraw{
            .drawConst = InstanceDrawInfo{.instanceCount = instanceCount, .instanceIndex = uint32_t(idx), .drawIndex = 0u},
            .drawInfos = geometryLevel->getDrawInfo()
          });
          instanceCount = 0u;
        };
      };

      //
      instances[0] = vk::AccelerationStructureGeometryKHR{
        .geometryType = vk::GeometryTypeKHR::eInstances,
        .geometry = vk::AccelerationStructureGeometryDataKHR{.instances = vk::AccelerationStructureGeometryInstancesDataKHR{
          .arrayOfPointers = false,
          .data = this->instanceBuild ? reinterpret_cast<vk::DeviceOrHostAddressConstKHR&>(this->getInstancedDeviceAddress()) : vk::DeviceOrHostAddressConstKHR(0ull),
        }},
        .flags = vk::GeometryFlagBitsKHR{}
      };
      instanceRanges[0] = vk::AccelerationStructureBuildRangeInfoKHR{
        .primitiveCount = uint32_t(this->cInfo->instances->size()),
        .primitiveOffset = 0u,
        .firstVertex = 0u,
        .transformOffset = 0u
      };

    };

    //
    virtual vk::CommandBuffer const& writeBuildStructureCmd(cpp21::carg<vk::CommandBuffer> cmdBuf = {}, vk::Buffer const& instanceDevOffset = {}, vk::Buffer const& instanceOffset = {}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(Handle(this->cInfo->uploader, HandleType::eUploader));
      decltype(auto) accelInstInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
      decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);
      decltype(auto) accelGeomInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelGeomInfo->setGeometries(this->instances), this->cInfo->limit, deviceObj->getDispatch()));
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(deviceObj->get<ResourceObj>(this->instanceBuild)->getBufferUsage()));

      // 
      uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
        .cmdBuf = cmdBuf,
        .bunchBuffer = instanceDevOffset,
        .dstBuffer = BufferRegion{this->instanceBuffer, DataRegion{ 0ull, sizeof(InstanceDevInfo), this->cInfo->instances->size() * sizeof(InstanceDevInfo) }}
      });

      // parallelize by offset
      uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
        .cmdBuf = cmdBuf,
        .bunchBuffer = instanceOffset,
        .dstBuffer = BufferRegion{this->instanceExtBuffer, DataRegion{ 0ull, sizeof(InstanceInfo), this->cInfo->instances->size() * sizeof(InstanceInfo) }}
      });

      //
      decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
          .srcAccessMask = accessMask,
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR),
          .dstAccessMask = vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = this->instanceBuild,
          .offset = 0ull,
          .size = std::min(accelSizes->accelerationStructureSize, accelInfo->size)
        },
      };

      //
      decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR),
          .srcAccessMask = vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR,
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
          .dstAccessMask = accessMask,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = this->instanceBuild,
          .offset = 0ull,
          .size = std::min(accelSizes->accelerationStructureSize, accelInfo->size)
        }
      };

      //
      decltype(auto) memoryBarriersBegin = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR),
          .dstAccessMask = vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR
        }
      };

      //
      decltype(auto) memoryBarriersEnd = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR),
          .srcAccessMask = vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR,
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite)
        }
      };

      // 
      cmdBuf->pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin).setMemoryBarriers(memoryBarriersBegin));
      cmdBuf->buildAccelerationStructuresKHR(1u, &accelInstInfo->setGeometries(this->instances).setMode(accelInstInfo->srcAccelerationStructure ? vk::BuildAccelerationStructureModeKHR::eUpdate : vk::BuildAccelerationStructureModeKHR::eBuild), cpp21::rvalue_to_ptr(instanceRanges.data()), deviceObj->getDispatch());
      cmdBuf->pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd).setMemoryBarriers(memoryBarriersEnd));

      //
      accelInstInfo->srcAccelerationStructure = accelInstInfo->dstAccelerationStructure;

      //
      return cmdBuf;
    };

    //
    virtual void clearInstances() {
      this->cInfo->instances.clear();
    };

    //
    virtual FenceType buildStructure(cpp21::carg<QueueGetInfo> info = QueueGetInfo{}) {
      // 
      if (this->cInfo->instances->size() > 0) {
        if (!this->accelStruct) {
          this->createStructure(info, false);
        } else {
          this->updateInstances();
        };
      } else {

      };

      //
      if (this->cInfo->instances->size() > 0) {
        decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info.ref() } };
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(Handle(this->cInfo->uploader, HandleType::eUploader));

        //
        decltype(auto) memReqs = uploaderObj->getMemoryRequirements();
        uintptr_t instanceDevSize = cpp21::tiled(this->cInfo->instances->size() * sizeof(InstanceDevInfo), memReqs.alignment) * memReqs.alignment;
        uintptr_t instanceSize = cpp21::tiled(this->cInfo->instances->size() * sizeof(InstanceInfo), memReqs.alignment) * memReqs.alignment;

        //
        uintptr_t instanceDevOffset = 0ull;
        uintptr_t instanceOffset = instanceDevSize;

        //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        decltype(auto) instanceDevAlloc = uploaderObj->allocateMappedTemp(instanceDevSize, instanceDevOffset);
        decltype(auto) instanceAlloc = uploaderObj->allocateMappedTemp(instanceSize, instanceOffset);
        decltype(auto) mappedBlock = uploaderObj->getMappedBlock();
        decltype(auto) instancePage = uploaderObj->allocatePage(instanceOffset, instanceSize);
        decltype(auto) instanceDevPage = uploaderObj->allocatePage(instanceDevOffset, instanceDevSize);
#endif

        // 
        memcpy(instanceDevPage->mapped, this->instanceDevInfo->data(), this->instanceDevInfo->size() * sizeof(InstanceDevInfo));
        memcpy(instancePage->mapped, this->instanceInfo->data(), this->instanceInfo->size() * sizeof(InstanceInfo));

        // TODO: Acceleration Structure Build Barriers per Buffers
        submission.commandInits.push_back([instanceDevOffset, instanceOffset, instancePage, instanceDevPage, dispatch = deviceObj->getDispatch(), this](cpp21::carg<vk::CommandBuffer> cmdBuf) {
          return this->writeBuildStructureCmd(cmdBuf, instanceDevPage->bunchBuffer, instancePage->bunchBuffer);
        });

        //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
        submission.submission.onDone.push_back([mappedBlock, instanceDevAlloc, instanceAlloc](cpp21::carg<vk::Result> result) {
          vmaVirtualFree(mappedBlock, instanceDevAlloc);
          vmaVirtualFree(mappedBlock, instanceAlloc);
          });
#endif

        //
        uploaderObj->bindMemoryPages(submission.submission);
        return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
      };
      return FenceType{};
    };

    //
    virtual FenceType createStructure(cpp21::carg<QueueGetInfo> info = QueueGetInfo{}, bool const& needsBuild = true) {
      this->updateInstances();

      //
      if (this->cInfo->limit <= 0u) {
        this->cInfo->limit = this->cInfo->instances->size();
      };

      //
      if (this->cInfo->instances->size() < this->cInfo->limit) {
        for (uintptr_t i = this->cInfo->instances->size(); i < this->cInfo->limit; i++) {
          decltype(auto) matrix = glm::mat3x4(1.f);
          this->cInfo->instances->push_back(InstanceDataInfo{
            .instanceDevInfo = InstanceDevInfo{
              .transform = reinterpret_cast<vk::TransformMatrixKHR&>(matrix),
              .instanceCustomIndex = 0u,
              .mask = 0u,
              .instanceShaderBindingTableRecordOffset = 0u,
              .flags = 0u,
              .accelerationStructureReference = 0u
            },
            .instanceInfo = InstanceInfo{

            }
          });
        };
      };

      // 
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) accelInstInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelInstInfo->setGeometries(this->instances), this->cInfo->limit, deviceObj->getDispatch()));
      decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);

      // 
      this->instanceBuffer = (this->bindInstanceBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = std::max(cInfo->instances->size(), size_t(cInfo->limit)) * sizeof(InstanceDevInfo),
          .type = BufferType::eStorage
        }
      })).as<vk::Buffer>();

      //
      this->instanceScratch = (this->bindInstanceScratch = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = std::max(accelSizes->buildScratchSize, accelSizes->updateScratchSize),
          .type = BufferType::eStorage
        }
      })).as<vk::Buffer>();

      //
      this->instanceBuild = (this->bindInstanceBuild = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = accelSizes->accelerationStructureSize,
          .type = BufferType::eStorage
        }
      })).as<vk::Buffer>();

      // 
      this->instanceExtBuffer = (this->bindInstanceExtBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = std::max(cInfo->instances->size(), size_t(cInfo->limit)) * sizeof(InstanceInfo),
          .type = BufferType::eStorage
        }
      })).as<vk::Buffer>();

      //
      //accelInfo->type = vk::AccelerationStructureTypeKHR::eBottomLevel;
      accelInfo->buffer = this->instanceBuild;
      accelInfo->offset = 0ull;
      accelInfo->size = accelSizes->accelerationStructureSize;

      //
      accelInstInfo->type = accelInfo->type;
      accelInstInfo->scratchData = reinterpret_cast<vk::DeviceOrHostAddressKHR&>(this->bindInstanceScratch->getDeviceAddress());
      accelInstInfo->srcAccelerationStructure = vk::AccelerationStructureKHR{};
      accelInstInfo->dstAccelerationStructure = (this->accelStruct = device.createAccelerationStructureKHR(accelInfo.ref(), nullptr, deviceObj->getDispatch()));

      //
      this->handle = device.getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR{ .accelerationStructure = this->accelStruct }, deviceObj->getDispatch());
      this->addressInfo = InstanceAddressInfo{ .data = this->getInstanceInfoDeviceAddress(), .accelStruct = this->accelStruct ? this->handle.as<uintptr_t>() : 0ull, .instanceCount = uint32_t(std::max(cInfo->instances->size(), size_t(cInfo->limit))) };

      //
      this->destructors.push_back(std::make_shared<std::function<DFun>>([this, device, accellStruct = accelInstInfo->dstAccelerationStructure, dispatch = deviceObj->getDispatch()](BaseObj const* baseObj) {
        //device.waitIdle();
        device.destroyAccelerationStructureKHR(accellStruct, nullptr, dispatch);
        this->bindInstanceBuffer->destroy(baseObj);
        this->bindInstanceScratch->destroy(baseObj);
        this->bindInstanceBuild->destroy(baseObj);
        this->bindInstanceExtBuffer->destroy(baseObj);
      }));

      //
      if (needsBuild) {
        return this->buildStructure(info);
      }
      else {
        return FenceType();
      };
    };

  protected:

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::carg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) {
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      decltype(auto) device = this->base.as<vk::Device>();
      //decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

      //
      decltype(auto) accelInfo = infoMap->set(vk::StructureType::eAccelerationStructureCreateInfoKHR, vk::AccelerationStructureCreateInfoKHR{
        .createFlags = vk::AccelerationStructureCreateFlagsKHR{},
        .type = vk::AccelerationStructureTypeKHR::eTopLevel
      });

      //
      decltype(auto) accelInstInfo = infoMap->set(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR, vk::AccelerationStructureBuildGeometryInfoKHR{
        .type = accelInfo->type,
        .flags = vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate | vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
        .mode = vk::BuildAccelerationStructureModeKHR::eBuild
      });

      //
      if (this->cInfo->instances->size() > 0 && !this->handle) {
        this->createStructure(cInfo->info);
      };

      //
      if (!this->handle) {
        this->handle = uintptr_t(this);
      };
    };
    
  };

};
#endif
