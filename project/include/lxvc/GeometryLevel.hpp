#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

// 
namespace lxvc {

  // 
  class GeometryLevelObj : public BaseObj {
  public: 
    using tType = WrapShared<GeometryLevelObj>;
    using BaseObj::BaseObj;
    //using BaseObj;

  protected: 
    //
    std::optional<GeometryLevelCreateInfo> cInfo = {};
    vk::Buffer geometryBuffer = {};
    vk::Buffer geometryScratch = {};
    vk::Buffer geometryBuild = {};

    //
    vk::AccelerationStructureKHR accelStruct = {};

    //
    std::vector<vk::AccelerationStructureGeometryKHR> geometries = {};
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR> geometryRanges = {};

    // 
    friend PipelineObj;
    friend SwapchainObj;
    friend GeometryLevelObj;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

  public:

    // 
    GeometryLevelObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    GeometryLevelObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) {
      auto shared = std::make_shared<GeometryLevelObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual WrapShared<ResourceObj> getGeometryResource() const {
      return lxvc::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->geometryBuffer);
    };

    //
    virtual vk::Buffer& getGeometryBuffer() { return this->geometryBuffer; };
    virtual vk::Buffer const& getGeometryBuffer() const { return this->geometryBuffer; };

    //
    virtual std::vector<GeometryInfo>& getGeometryData() { return this->cInfo->geometryData; };
    virtual std::vector<GeometryInfo> const& getGeometryData() const { return this->cInfo->geometryData; };

    //
    virtual uintptr_t getGeometryDeviceAddress() const { return this->getGeometryResource()->getDeviceAddress(); };


  protected:

    //
    virtual void updateGeometries() {
      this->geometries = {};
      this->geometryRanges = {};
      for (decltype(auto) geometry : this->cInfo->geometryData) {
        geometries.push_back(vk::AccelerationStructureGeometryKHR{
          .geometryType = vk::GeometryTypeKHR::eTriangles,
          .geometry = vk::AccelerationStructureGeometryDataKHR{.triangles = vk::AccelerationStructureGeometryTrianglesDataKHR{
            .vertexFormat = cvtFormat(geometry.vertices.format),
            .vertexData = geometry.vertices.deviceAddress,
            .vertexStride = geometry.vertices.stride,
            .maxVertex = geometry.primitiveCount * 3u,
            .indexType = cvtIndex(geometry.indices.format),
            .indexData = geometry.indices.deviceAddress,
            .transformData = geometry.transform.deviceAddress
          }},
          .flags = geometry.opaque ? vk::GeometryFlagBitsKHR::eOpaque : vk::GeometryFlagBitsKHR{}
        });
        geometryRanges.push_back(vk::AccelerationStructureBuildRangeInfoKHR{
          .primitiveCount = geometry.primitiveCount,
          .primitiveOffset = 0u,
          .firstVertex = 0u,
          .transformOffset = 0u
        });
      };
    };

    //
    virtual FenceType buildStructure(cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      this->updateGeometries();

      //
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info } };
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(this->cInfo->uploader);

      // 
      memcpy(deviceObj->get<ResourceObj>(uploaderObj->uploadBuffer)->mappedMemory, this->cInfo->geometryData.data(), this->cInfo->geometryData.size()*sizeof(GeometryInfo));

      // TODO: Acceleration Structure Build Barriers per Buffers
      submission.commandInits.push_back([=, this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
          .cmdBuf = cmdBuf,
          .dstBuffer = BufferRegion{this->geometryBuffer, DataRegion{ 0ull, this->cInfo->geometryData.size() * sizeof(GeometryInfo) }}
        });
        cmdBuf->buildAccelerationStructuresKHR(1u, infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR).get(), cpp21::rvalue_to_ptr(geometryRanges.data()));
        return cmdBuf;
      });

      //
      return lxvc::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType createStructure() {
      this->updateGeometries();

      // 
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) accelGeomInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelGeomInfo->setGeometries(this->geometries), this->cInfo->maxPrimitiveCounts));
      decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);

      //
      this->geometryScratch = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = std::max(accelSizes->buildScratchSize, accelSizes->updateScratchSize),
          .type = BufferType::eStorage
        }
      }).as<vk::Buffer>();

      //
      this->geometryBuild = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = accelSizes->accelerationStructureSize,
          .type = BufferType::eStorage
        }
      }).as<vk::Buffer>();

      //
      accelInfo->buffer = this->geometryBuild;
      accelInfo->offset = 0ull;
      accelInfo->size = accelSizes->accelerationStructureSize;

      //
      accelGeomInfo->scratchData = vk::DeviceOrHostAddressKHR(lxvc::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->geometryScratch)->getDeviceAddress());
      accelGeomInfo->dstAccelerationStructure = this->accelStruct = device.createAccelerationStructureKHR(accelInfo.ref());

      //
      return this->buildStructure();
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());
      decltype(auto) device = this->base.as<vk::Device>();

      // 
      this->geometryBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = cInfo->geometryCount,
          .type = BufferType::eStorage
        }
      }).as<vk::Buffer>();

      //
      decltype(auto) accelInfo = infoMap->set(vk::StructureType::eAccelerationStructureCreateInfoKHR, vk::AccelerationStructureCreateInfoKHR{
        .createFlags = vk::AccelerationStructureCreateFlagsKHR{},
        .type = vk::AccelerationStructureTypeKHR::eBottomLevel
      });

      //
      decltype(auto) accelGeomInfo = infoMap->set(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR, vk::AccelerationStructureBuildGeometryInfoKHR{
        .mode = vk::BuildAccelerationStructureModeKHR::eBuild
      });

      //
      if (this->cInfo->maxPrimitiveCounts.size() <= 0) {
        for (decltype(auto) geometry : this->cInfo->geometryData) {
          this->cInfo->maxPrimitiveCounts.push_back(geometry.primitiveCount);
        };
      };

      //
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelGeomInfo->setGeometries(this->geometries), this->cInfo->maxPrimitiveCounts));

      //
      this->handle = uintptr_t(this);

    };
    
  };

};
