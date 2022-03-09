#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

// 
namespace lxvc {

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

    //
    //vk::AccelerationStructureKHR accelStruct = {};

    //
    std::vector<vk::AccelerationStructureGeometryKHR> instances = {};
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR> instanceRanges = {};
    std::vector<vk::MultiDrawInfoEXT> multiDraw = {};

    //
    //uintptr_t deviceAddress = 0ull;
    vk::AccelerationStructureKHR accelStruct = {};

    // 
    friend PipelineObj;
    friend SwapchainObj;
    friend InstanceLevelObj;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

  public:

    // 
    InstanceLevelObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    InstanceLevelObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : cInfo(cInfo) {
      this->construct(lxvc::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual tType registerSelf() {
      lxvc::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
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
      return lxvc::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->instanceBuffer);
    };

    //
    virtual vk::Buffer& getInstancedBuffer() { return this->instanceBuffer; };
    virtual vk::Buffer const& getInstancedBuffer() const { return this->instanceBuffer; };

    //
    virtual std::vector<InstanceInfo>& getInstancedData() { return this->cInfo->instanceData; };
    virtual std::vector<InstanceInfo> const& getInstancedData() const { return this->cInfo->instanceData; };

    //
    virtual std::vector<vk::MultiDrawInfoEXT>& getMultiDraw() { return this->multiDraw; };
    virtual std::vector<vk::MultiDrawInfoEXT> const& getMultiDraw() const { return this->multiDraw; };

    //
    virtual uintptr_t const& getInstancedDeviceAddress() const { return this->getInstancedResource()->getDeviceAddress(); };
    virtual uintptr_t& getInstancedDeviceAddress() { return this->getInstancedResource()->getDeviceAddress(); };

    //
    virtual uintptr_t& getDeviceAddress() { return this->handle.as<uintptr_t>(); };
    virtual uintptr_t const& getDeviceAddress() const { return this->handle.as<uintptr_t>(); };


  protected:

    //
    virtual void updateInstances() {
      this->instances = {};
      this->instanceRanges = {};
      this->multiDraw = {};
      {
        instances.push_back(vk::AccelerationStructureGeometryKHR{
          .geometryType = vk::GeometryTypeKHR::eInstances,
          .geometry = vk::AccelerationStructureGeometryDataKHR{.instances = vk::AccelerationStructureGeometryInstancesDataKHR{
            .arrayOfPointers = false,
            .data = vk::DeviceOrHostAddressConstKHR(this->getInstancedDeviceAddress()),
          }},
          .flags = vk::GeometryFlagBitsKHR{}
        });
        instanceRanges.push_back(vk::AccelerationStructureBuildRangeInfoKHR{
          .primitiveCount = uint32_t(this->cInfo->instanceData.size()),
          .primitiveOffset = 0u,
          .firstVertex = 0u,
          .transformOffset = 0u
        });
        multiDraw.push_back(vk::MultiDrawInfoEXT{
          .firstVertex = 0u,
          .vertexCount = uint32_t(this->cInfo->instanceData.size())
        });
      };
      if (this->cInfo->maxPrimitiveCounts.size() <= 0) {
         this->cInfo->maxPrimitiveCounts.push_back(this->cInfo->instanceData.size());
      };
    };

    //
    virtual FenceType buildStructure(cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      this->updateInstances();

      //
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info } };
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(this->cInfo->uploader);

      // 
      memcpy(deviceObj->get<ResourceObj>(uploaderObj->uploadBuffer)->mappedMemory, this->cInfo->instanceData.data(), this->cInfo->instanceData.size()*sizeof(InstanceInfo));

      // TODO: Acceleration Structure Build Barriers per Buffers
      submission.commandInits.push_back([=, this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
          .cmdBuf = cmdBuf,
          .dstBuffer = BufferRegion{this->instanceBuffer, DataRegion{ 0ull, this->cInfo->instanceData.size() * sizeof(InstanceInfo) }}
        });
        cmdBuf->buildAccelerationStructuresKHR(1u, &infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR)->setGeometries(this->instances), cpp21::rvalue_to_ptr(instanceRanges.data()), deviceObj->dispatch);
        return cmdBuf;
      });

      //
      return lxvc::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual vk::Result createStructure() {
      this->updateInstances();

      // 
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) accelInstInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelInstInfo->setGeometries(this->instances), this->cInfo->maxPrimitiveCounts, deviceObj->dispatch));
      decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);
      

      //
      this->instanceScratch = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = std::max(accelSizes->buildScratchSize, accelSizes->updateScratchSize),
          .type = BufferType::eStorage
        }
      }).as<vk::Buffer>();

      //
      this->instanceBuild = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = accelSizes->accelerationStructureSize,
          .type = BufferType::eStorage
        }
      }).as<vk::Buffer>();

      //
      //accelInfo->type = vk::AccelerationStructureTypeKHR::eBottomLevel;
      accelInfo->buffer = this->instanceBuild;
      accelInfo->offset = 0ull;
      accelInfo->size = accelSizes->accelerationStructureSize;

      //
      accelInstInfo->type = accelInfo->type;
      accelInstInfo->scratchData = vk::DeviceOrHostAddressKHR(lxvc::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->instanceScratch)->getDeviceAddress());
      this->accelStruct = (accelInstInfo->dstAccelerationStructure = device.createAccelerationStructureKHR(accelInfo.ref(), nullptr, deviceObj->dispatch));
      this->handle = device.getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR{ .accelerationStructure = this->accelStruct }, deviceObj->dispatch);

      //
      return std::get<0>(*this->buildStructure()).get();
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());
      decltype(auto) device = this->base.as<vk::Device>();
      //decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);

      // 
      this->instanceBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = std::max(cInfo->instanceData.size(), cInfo->maxPrimitiveCounts.size()) * sizeof(InstanceInfo),
          .type = BufferType::eStorage
        }
      }).as<vk::Buffer>();

      //
      decltype(auto) accelInfo = infoMap->set(vk::StructureType::eAccelerationStructureCreateInfoKHR, vk::AccelerationStructureCreateInfoKHR{
        .createFlags = vk::AccelerationStructureCreateFlagsKHR{},
        .type = vk::AccelerationStructureTypeKHR::eTopLevel
      });

      //
      decltype(auto) accelInstInfo = infoMap->set(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR, vk::AccelerationStructureBuildGeometryInfoKHR{
        .type = accelInfo->type,
        .mode = vk::BuildAccelerationStructureModeKHR::eBuild
      });

      //
      if (this->cInfo->maxPrimitiveCounts.size() <= 0) {
        this->cInfo->maxPrimitiveCounts.push_back(this->cInfo->instanceData.size());
      };

      //
      //decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelInstInfo->setInstances(this->instances), this->cInfo->maxPrimitiveCounts, deviceObj->dispatch));

      //
      if (this->cInfo->instanceData.size() > 0 && !this->handle) {
        this->createStructure();
      };

      //
      if (!this->handle) {
        this->handle = uintptr_t(this);
      };
    };
    
  };

};
