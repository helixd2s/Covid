#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"
#include "./GeometryLevel.hpp"

//
#ifdef Z_ENABLE_VMA
#include "./MemoryAllocatorVma.hpp"
#endif

// 
namespace ZNAMED {

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

    WrapShared<ResourceObj> bindInstanceBuffer = {};
    WrapShared<ResourceObj> bindInstanceScratch = {};
    WrapShared<ResourceObj> bindInstanceBuild = {};
    WrapShared<ResourceObj> bindInstanceExtBuffer = {};

    //
    std::array<vk::AccelerationStructureGeometryKHR, 1> instances = {};
    std::array<vk::AccelerationStructureBuildRangeInfoKHR, 1> instanceRanges = {};

    //
    cpp21::shared_vector<InstanceDraw> instanceDraw = std::vector<InstanceDraw>{};
    cpp21::shared_vector<InstanceInfo> instanceInfo = std::vector<InstanceInfo>{};

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
    InstanceLevelObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : BaseObj(deviceObj), cInfo(cInfo), instanceDraw(std::vector<InstanceDraw>{}), instanceInfo(std::vector<InstanceInfo>{}) {
      this->base = deviceObj->getHandle();
      this->construct(deviceObj, cInfo);
    };

    // 
    InstanceLevelObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : BaseObj(handle), cInfo(cInfo), instanceDraw(std::vector<InstanceDraw>{}), instanceInfo(std::vector<InstanceInfo>{}) {
      this->construct(ZNAMED::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual tType registerSelf() {
      ZNAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) {
      auto shared = std::make_shared<InstanceLevelObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual WrapShared<ResourceObj> getInstancedResource() const {
      return ZNAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->instanceBuffer);
    };

    //
    virtual WrapShared<ResourceObj> getInstanceInfoResource() const {
      return ZNAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->instanceExtBuffer);
    };

    //
    virtual InstanceAddressInfo& getAddressInfo() { return addressInfo; };
    virtual InstanceAddressInfo const& getAddressInfo() const { return addressInfo; };

    //
    virtual vk::Buffer& getInstancedBuffer() { return this->instanceBuffer; };
    virtual vk::Buffer const& getInstancedBuffer() const { return this->instanceBuffer; };

    //
    virtual std::vector<InstanceDevInfo>& getInstances() { return this->cInfo->instances; };
    virtual std::vector<InstanceDevInfo> const& getInstances() const { return this->cInfo->instances; };

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
    virtual uintptr_t& getDeviceAddress() { return this->handle.as<uintptr_t>(); };
    virtual uintptr_t const& getDeviceAddress() const { return this->handle.as<uintptr_t>(); };

    //
    virtual void updateInstances() {
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);

      // 
      this->instances = {};
      this->instanceRanges = {};

      //
      if (this->instanceDraw->size() < this->cInfo->instances.size()) {
        for (uintptr_t idx = this->instanceDraw->size(); idx < this->cInfo->instances.size(); idx++) {
          this->instanceDraw->push_back(InstanceDraw{});
          this->instanceInfo->push_back(InstanceInfo{});
        };
      };

      // 
      for (uintptr_t idx = 0ull; idx < this->cInfo->instances.size(); idx++) {
        auto& instances = this->cInfo->instances[idx];
        auto& instanceDraw = this->instanceDraw[idx];
        auto& instanceInfo = this->instanceInfo[idx];

        // 
        decltype(auto) geometryLevel = deviceObj->get<GeometryLevelObj>(instances.accelerationStructureReference);

        // 
        if (this->getInstanceInfoResource()) {
          instanceDraw.drawInfos = geometryLevel->getDrawInfo();
          instanceDraw.drawConst = PushConstantData{ .dataAddress = this->getInstanceInfoDeviceAddress() + sizeof(InstanceInfo) * idx, .instanceIndex = uint32_t(idx), .drawIndex = 0u };
        };

        // 
        instanceInfo = InstanceInfo{ .transform = reinterpret_cast<glm::mat3x4&>(instances.transform), .reference = geometryLevel->getGeometryDeviceAddress() };
      };

      {
        instances[0] = vk::AccelerationStructureGeometryKHR{
          .geometryType = vk::GeometryTypeKHR::eInstances,
          .geometry = vk::AccelerationStructureGeometryDataKHR{.instances = vk::AccelerationStructureGeometryInstancesDataKHR{
            .arrayOfPointers = false,
            .data = vk::DeviceOrHostAddressConstKHR(this->getInstancedDeviceAddress()),
          }},
          .flags = vk::GeometryFlagBitsKHR{}
        };
        instanceRanges[0] = vk::AccelerationStructureBuildRangeInfoKHR{
          .primitiveCount = uint32_t(this->cInfo->instances.size()),
          .primitiveOffset = 0u,
          .firstVertex = 0u,
          .transformOffset = 0u
        };
      };
    };

    //
    virtual vk::CommandBuffer const& writeBuildStructureCmd(cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf = {}, uintptr_t const& instanceDevOffset = 0ull, uintptr_t const& instanceOffset = 0ull) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(Handle(this->cInfo->uploader, HandleType::eUploader));

      // 
      uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
        .cmdBuf = cmdBuf,
        .hostMapOffset = instanceDevOffset,
        .dstBuffer = BufferRegion{this->instanceBuffer, DataRegion{ 0ull, sizeof(InstanceDevInfo), this->cInfo->instances.size() * sizeof(InstanceDevInfo) }}
      });

      // parallelize by offset
      uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
        .cmdBuf = cmdBuf,
        .hostMapOffset = instanceOffset,
        .dstBuffer = BufferRegion{this->instanceExtBuffer, DataRegion{ 0ull, sizeof(InstanceInfo), this->cInfo->instances.size() * sizeof(InstanceInfo) }}
      });

      //
      decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);
      decltype(auto) accelGeomInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelGeomInfo->setGeometries(this->instances), this->cInfo->limit, deviceObj->getDispatch()));
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(deviceObj->get<ResourceObj>(this->instanceBuild)->getBufferUsage()));

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
      cmdBuf->pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
      cmdBuf->buildAccelerationStructuresKHR(1u, &infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR)->setGeometries(this->instances), cpp21::rvalue_to_ptr(instanceRanges.data()), deviceObj->getDispatch());
      cmdBuf->pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd));

      //
      return cmdBuf;
    };

    //
    virtual FenceType buildStructure(cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      this->updateInstances();

      //
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info } };
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(Handle(this->cInfo->uploader, HandleType::eUploader));

      //
      uintptr_t instanceDevOffset = 0ull;
      uintptr_t instanceOffset = this->cInfo->instances.size() * sizeof(InstanceDevInfo);

      //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      decltype(auto) instanceDevAlloc = uploaderObj->allocateUploadTemp(this->cInfo->instances.size() * sizeof(InstanceDevInfo), instanceDevOffset);
      decltype(auto) instanceAlloc = uploaderObj->allocateUploadTemp(this->cInfo->instances.size() * sizeof(InstanceInfo), instanceOffset);
      decltype(auto) uploadBlock = uploaderObj->getUploadBlock();
#endif

      // 
      memcpy(uploaderObj->getUploadMapped(instanceDevOffset), this->cInfo->instances.data(), this->cInfo->instances.size() * sizeof(InstanceDevInfo));
      memcpy(uploaderObj->getUploadMapped(instanceOffset), this->instanceInfo->data(), this->instanceInfo->size() * sizeof(InstanceInfo));

      // TODO: Acceleration Structure Build Barriers per Buffers
      submission.commandInits.push_back([instanceDevOffset, instanceOffset, dispatch = deviceObj->getDispatch(), this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        return this->writeBuildStructureCmd(cmdBuf, instanceDevOffset, instanceOffset);
      });

      //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      submission.onDone.push_back([uploadBlock, instanceDevAlloc, instanceAlloc](cpp21::const_wrap_arg<vk::Result> result) {
        vmaVirtualFree(uploadBlock, instanceDevAlloc);
        vmaVirtualFree(uploadBlock, instanceAlloc);
      });
#endif

      //
      return ZNAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType createStructure() {
      this->updateInstances();

      //
      if (this->cInfo->limit <= 0u) {
        this->cInfo->limit = this->cInfo->instances.size();
      };

      //
      if (this->cInfo->instances.size() < this->cInfo->limit) {
        for (uintptr_t i = this->cInfo->instances.size(); i < this->cInfo->limit; i++) {
          this->cInfo->instances.push_back(InstanceDevInfo{
            .transform = reinterpret_cast<vk::TransformMatrixKHR&&>(glm::mat3x4(1.f)),
            .instanceCustomIndex = 0u,
            .mask = 0u,
            .instanceShaderBindingTableRecordOffset = 0u,
            .flags = 0u,
            .accelerationStructureReference = 0u
          });
        };
      };

      // 
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) accelInstInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelInstInfo->setGeometries(this->instances), this->cInfo->limit, deviceObj->getDispatch()));
      decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);

      // 
      this->instanceBuffer = (this->bindInstanceBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = std::max(cInfo->instances.size(), size_t(cInfo->limit)) * sizeof(InstanceDevInfo),
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
          .size = cInfo->instances.size() * sizeof(InstanceInfo),
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
      accelInstInfo->scratchData = vk::DeviceOrHostAddressKHR(this->bindInstanceScratch->getDeviceAddress());
      accelInstInfo->srcAccelerationStructure = accelInstInfo->dstAccelerationStructure;
      accelInstInfo->dstAccelerationStructure = (this->accelStruct = device.createAccelerationStructureKHR(accelInfo.ref(), nullptr, deviceObj->getDispatch()));

      //
      this->handle = device.getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR{ .accelerationStructure = this->accelStruct }, deviceObj->getDispatch());
      this->addressInfo = InstanceAddressInfo{ .data = this->getInstanceInfoDeviceAddress(), .accelStruct = this->handle.as<uintptr_t>() };

      //
      this->destructors.push_back([this, device, accellStruct = accelInstInfo->dstAccelerationStructure, dispatch = deviceObj->getDispatch()](BaseObj const* baseObj) {
        device.waitIdle();
        device.destroyAccelerationStructureKHR(accellStruct, nullptr, dispatch);
        this->bindInstanceBuffer->destroy(baseObj);
        this->bindInstanceScratch->destroy(baseObj);
        this->bindInstanceBuild->destroy(baseObj);
        this->bindInstanceExtBuffer->destroy(baseObj);
      });

      //
      //return std::get<0>(*this->buildStructure())->get();
      return this->buildStructure();
    };

  protected:

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) {
      this->base = deviceObj->getHandle();
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());
      decltype(auto) device = this->base.as<vk::Device>();
      //decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);

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
      if (this->cInfo->instances.size() > 0 && !this->handle) {
        this->createStructure();
      };

      //
      if (!this->handle) {
        this->handle = uintptr_t(this);
      };
    };
    
  };

};
