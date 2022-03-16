#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"
#include "./GeometryLevel.hpp"

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

    //
    //InstanceDrawData
    //
    //vk::AccelerationStructureKHR accelStruct = {};

    //
    std::vector<vk::AccelerationStructureGeometryKHR> instances = {};
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR> instanceRanges = {};
    //std::vector<vk::MultiDrawInfoEXT> multiDraw = {};

    //
    //cpp21::bucket<std::shared_ptr<InstanceDrawInfo>> instanceDrawInfo = {};
    cpp21::shared_vector<InstanceDrawInfo> instanceDrawInfo = std::vector<InstanceDrawInfo>{};
    cpp21::shared_vector<InstanceDrawData> instanceDrawData = std::vector<InstanceDrawData>{};

    //
    //uintptr_t deviceAddress = 0ull;
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
    InstanceLevelObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : cInfo(cInfo), instanceDrawInfo(std::vector<InstanceDrawInfo>{}), instanceDrawData(std::vector<InstanceDrawData>{}) {
      this->base = deviceObj->getHandle();
      this->construct(deviceObj, cInfo);
    };

    // 
    InstanceLevelObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<InstanceLevelCreateInfo> cInfo = InstanceLevelCreateInfo{}) : cInfo(cInfo), instanceDrawInfo(std::vector<InstanceDrawInfo>{}), instanceDrawData(std::vector<InstanceDrawData>{}) {
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
    virtual WrapShared<ResourceObj> getDrawDataResource() const {
      return ZNAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->instanceExtBuffer);
    };

    //
    virtual vk::Buffer& getInstancedBuffer() { return this->instanceBuffer; };
    virtual vk::Buffer const& getInstancedBuffer() const { return this->instanceBuffer; };

    //
    virtual std::vector<InstanceInfo>& getInstancedData() { return this->cInfo->instanceData; };
    virtual std::vector<InstanceInfo> const& getInstancedData() const { return this->cInfo->instanceData; };

    //
    virtual cpp21::shared_vector<InstanceDrawInfo>& getDrawInfo() { return this->instanceDrawInfo; };
    virtual cpp21::shared_vector<InstanceDrawInfo> const& getDrawInfo() const { return this->instanceDrawInfo; };

    //
    virtual uintptr_t const& getDrawDataDeviceAddress() const { return this->getDrawDataResource()->getDeviceAddress(); };
    virtual uintptr_t& getDrawDataDeviceAddress() { return this->getDrawDataResource()->getDeviceAddress(); };

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
      this->addressInfo = InstanceAddressInfo{ .data = this->getDrawDataDeviceAddress(), .accelStruct = this->handle.as<uintptr_t>() };

      //
      if (this->instanceDrawInfo->size() < this->cInfo->instanceData.size()) {
        for (uintptr_t idx = this->instanceDrawInfo->size(); idx < this->cInfo->instanceData.size(); idx++) {
          this->instanceDrawInfo->push_back(InstanceDrawInfo{});
          this->instanceDrawData->push_back(InstanceDrawData{});
        };
      };

      // 
      for (uintptr_t idx = 0ull; idx < this->cInfo->instanceData.size(); idx++) {
        auto& instanceData = this->cInfo->instanceData[idx];
        auto& instanceDraw = this->instanceDrawInfo[idx];
        auto& instanceDrawData = this->instanceDrawData[idx];

        // 
        decltype(auto) geometryLevel = deviceObj->get<GeometryLevelObj>(instanceData.accelerationStructureReference);

        // 
        instanceDraw.drawInfos = geometryLevel->getDrawInfo();
        instanceDraw.drawData = PushConstantData{ .addressInfo = this->addressInfo, .drawIndex = uint32_t(idx) };

        // 
        instanceDrawData = InstanceDrawData{ .transform = reinterpret_cast<glm::mat3x4&>(instanceData.transform), .reference = geometryLevel->getGeometryDeviceAddress() };
      };
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
      };
    };

    //
    virtual FenceType buildStructure(cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      this->updateInstances();

      //
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info } };
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(this->cInfo->uploader);

      // 
      memcpy(deviceObj->get<ResourceObj>(uploaderObj->uploadBuffer)->mappedMemory, this->cInfo->instanceData.data(), this->cInfo->instanceData.size()*sizeof(InstanceInfo));
      memcpy(cpp21::shift(deviceObj->get<ResourceObj>(uploaderObj->uploadBuffer)->mappedMemory, this->cInfo->instanceData.size() * sizeof(InstanceInfo)), this->instanceDrawData->data(), this->cInfo->instanceData.size() * sizeof(InstanceDrawData));

      // TODO: Acceleration Structure Build Barriers per Buffers
      submission.commandInits.push_back([dispatch = deviceObj->getDispatch(), this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
        decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(this->cInfo->uploader);

        // 
        uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
          .cmdBuf = cmdBuf,
          .hostMapOffset = 0ull,
          .dstBuffer = BufferRegion{this->instanceBuffer, DataRegion{ 0ull, this->cInfo->instanceData.size() * sizeof(InstanceInfo) }}
        });

        // parallelize by offset
        uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
          .cmdBuf = cmdBuf,
          .hostMapOffset = this->cInfo->instanceData.size() * sizeof(InstanceInfo),
          .dstBuffer = BufferRegion{this->instanceExtBuffer, DataRegion{ 0ull, this->cInfo->instanceData.size() * sizeof(InstanceDrawData) }}
        });

        // 
        cmdBuf->buildAccelerationStructuresKHR(1u, &infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR)->setGeometries(this->instances), cpp21::rvalue_to_ptr(instanceRanges.data()), dispatch);
        return cmdBuf;
      });

      //
      return ZNAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual vk::Result createStructure() {
      this->updateInstances();

      //
      if (this->cInfo->limits.size() <= 0) {
        this->cInfo->limits.push_back(this->cInfo->instanceData.size());
      };

      //
      if (this->cInfo->instanceData.size() < this->cInfo->limits.size()) {
        for (uintptr_t i = this->cInfo->instanceData.size(); i < this->cInfo->limits.size(); i++) {
          this->cInfo->instanceData.push_back(InstanceInfo{
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
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelInstInfo->setGeometries(this->instances), this->cInfo->limits, deviceObj->getDispatch()));
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
      this->instanceExtBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = cInfo->instanceData.size() * sizeof(InstanceDrawData),
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
      accelInstInfo->scratchData = vk::DeviceOrHostAddressKHR(ZNAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->instanceScratch)->getDeviceAddress());
      accelInstInfo->srcAccelerationStructure = accelInstInfo->dstAccelerationStructure;
      accelInstInfo->dstAccelerationStructure = (this->accelStruct = device.createAccelerationStructureKHR(accelInfo.ref(), nullptr, deviceObj->getDispatch()));

      //
      this->handle = device.getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR{ .accelerationStructure = this->accelStruct }, deviceObj->getDispatch());

      //
      return std::get<0>(*this->buildStructure()).get();
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
      this->instanceBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = std::max(cInfo->instanceData.size(), cInfo->limits.size()) * sizeof(InstanceInfo),
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
        .flags = vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate | vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
        .mode = vk::BuildAccelerationStructureModeKHR::eBuild
      });

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
