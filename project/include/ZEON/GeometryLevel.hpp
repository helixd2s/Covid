#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

// 
namespace ZNAMED {

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
    //vk::AccelerationStructureKHR accelStruct = {};

    //
    std::vector<vk::AccelerationStructureGeometryKHR> geometryInfos = {};
    std::vector<vk::AccelerationStructureBuildRangeInfoKHR> geometryRanges = {};
    cpp21::shared_vector<vk::MultiDrawInfoEXT> multiDraw = std::vector<vk::MultiDrawInfoEXT>{};

    //
    //uintptr_t deviceAddress = 0ull;
    vk::AccelerationStructureKHR accelStruct = {};

    // 
    friend PipelineObj;
    friend SwapchainObj;
    friend GeometryLevelObj;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:

    // 
    GeometryLevelObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->getHandle();
      this->construct(deviceObj, cInfo);
    };

    // 
    GeometryLevelObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) {
      auto shared = std::make_shared<GeometryLevelObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual WrapShared<ResourceObj> getGeometryResource() const {
      return ZNAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->geometryBuffer);
    };

    //
    virtual vk::Buffer& getGeometryBuffer() { return this->geometryBuffer; };
    virtual vk::Buffer const& getGeometryBuffer() const { return this->geometryBuffer; };

    //
    virtual std::vector<GeometryInfo>& getGeometries() { return this->cInfo->geometries; };
    virtual std::vector<GeometryInfo> const& getGeometries() const { return this->cInfo->geometries; };

    //
    virtual cpp21::shared_vector<vk::MultiDrawInfoEXT>& getDrawInfo() { return this->multiDraw; };
    virtual cpp21::shared_vector<vk::MultiDrawInfoEXT> const& getDrawInfo() const { return this->multiDraw; };

    //
    virtual uintptr_t const& getGeometryDeviceAddress() const { return this->getGeometryResource()->getDeviceAddress(); };
    virtual uintptr_t& getGeometryDeviceAddress() { return this->getGeometryResource()->getDeviceAddress(); };

    //
    virtual uintptr_t& getDeviceAddress() { return this->handle.as<uintptr_t>(); };
    virtual uintptr_t const& getDeviceAddress() const { return this->handle.as<uintptr_t>(); };


  protected:

    //
    virtual void updateGeometries() {
      this->geometryInfos = {};
      this->geometryRanges = {};
      this->multiDraw = std::vector<vk::MultiDrawInfoEXT>{};
      for (decltype(auto) geometry : this->cInfo->geometries) {
        geometryInfos.push_back(vk::AccelerationStructureGeometryKHR{
          .geometryType = vk::GeometryTypeKHR::eTriangles,
          .geometry = vk::AccelerationStructureGeometryDataKHR{.triangles = vk::AccelerationStructureGeometryTrianglesDataKHR{
            .vertexFormat = cvtFormat(geometry.vertices.format),
            .vertexData = geometry.vertices.region.deviceAddress,
            .vertexStride = geometry.vertices.region.stride,
            .maxVertex = uint32_t(cpp21::tiled(geometry.vertices.region.size, geometry.vertices.region.stride)),
            .indexType = cvtIndex(geometry.indices.format),
            .indexData = geometry.indices.region.deviceAddress,
            .transformData = geometry.transform.region.deviceAddress
          }},
          .flags = (geometry.flags&1u) ? vk::GeometryFlagBitsKHR::eOpaque : vk::GeometryFlagBitsKHR{}
        });
        geometryRanges.push_back(vk::AccelerationStructureBuildRangeInfoKHR{
          .primitiveCount = geometry.primitiveCount,
          .primitiveOffset = 0u,
          .firstVertex = 0u,
          .transformOffset = 0u
        });
        multiDraw->push_back(vk::MultiDrawInfoEXT{
          .firstVertex = 0u,
          .vertexCount = geometry.primitiveCount * 3u
        });
      };
    };

    //
    virtual vk::CommandBuffer const& writeBuildStructureCmd(cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf = {}, uintptr_t const& geometryOffset = 0ull) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(this->cInfo->uploader);

      // 
      uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
        .cmdBuf = cmdBuf,
        .hostMapOffset = geometryOffset,
        .dstBuffer = BufferRegion{this->geometryBuffer, DataRegion{ 0ull, this->cInfo->geometries.size() * sizeof(GeometryInfo) }}
      }, geometryOffset);

      //
      decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);
      decltype(auto) accelGeomInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelGeomInfo->setGeometries(this->geometryInfos), this->cInfo->limits, deviceObj->getDispatch()));
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(deviceObj->get<ResourceObj>(this->geometryBuild)->getBufferUsage()));

      //
      decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
          .srcAccessMask = accessMask,
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR),
          .dstAccessMask = vk::AccessFlagBits2::eAccelerationStructureWriteKHR | vk::AccessFlagBits2::eAccelerationStructureReadKHR,
          .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
          .buffer = this->geometryBuild,
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
          .buffer = this->geometryBuild,
          .offset = 0ull,
          .size = std::min(accelSizes->accelerationStructureSize, accelInfo->size)
        }
      };

      //
      cmdBuf->pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
      cmdBuf->buildAccelerationStructuresKHR(1u, &infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR)->setGeometries(this->geometryInfos), cpp21::rvalue_to_ptr(geometryRanges.data()), deviceObj->getDispatch());
      cmdBuf->pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd));

      // 
      return cmdBuf;
    };

    //
    virtual FenceType buildStructure(cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      this->updateGeometries();

      //
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info } };
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(this->cInfo->uploader);

      //
      uintptr_t geometryOffset = 0ull;

      //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
      decltype(auto) geometryAlloc = uploaderObj->allocateUploadTemp(this->cInfo->geometries.size() * sizeof(InstanceDevInfo));
      decltype(auto) uploadBlock = uploaderObj->getUploadBlock();

      //
      geometryOffset = std::get<0u>(geometryAlloc);
#endif

      // 
      memcpy(uploaderObj->getUploadMapped(geometryOffset), this->cInfo->geometries.data(), this->cInfo->geometries.size()*sizeof(GeometryInfo));

      // TODO: Acceleration Structure Build Barriers per Buffers
      submission.commandInits.push_back([geometryOffset,dispatch=deviceObj->getDispatch(), this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        return this->writeBuildStructureCmd(cmdBuf, geometryOffset);
      });

      //
      submission.onDone.push_back([uploadBlock, geometryAlloc](cpp21::const_wrap_arg<vk::Result> result) {
        vmaVirtualFree(uploadBlock, std::get<1u>(geometryAlloc));
      });

      //
      return ZNAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual vk::Result createStructure() {
      this->updateGeometries();

      //
      if (this->cInfo->limits.size() <= 0) {
        for (decltype(auto) geometry : this->cInfo->geometries) {
          this->cInfo->limits.push_back(geometry.primitiveCount);
        };
      };

      //
      if (this->cInfo->geometries.size() < this->cInfo->limits.size()) {
        for (uintptr_t i = this->cInfo->geometries.size(); i < this->cInfo->limits.size(); i++) {
          this->cInfo->geometries.push_back(GeometryInfo{});
        };
      };

      // 
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) accelGeomInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
      decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelGeomInfo->setGeometries(this->geometryInfos), this->cInfo->limits, deviceObj->getDispatch()));
      decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);
      
      // 
      this->geometryBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = std::max(cInfo->geometries.size(), cInfo->limits.size()) * sizeof(GeometryInfo),
          .type = BufferType::eStorage
        }
      }).as<vk::Buffer>();

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
      //accelInfo->type = vk::AccelerationStructureTypeKHR::eBottomLevel;
      accelInfo->buffer = this->geometryBuild;
      accelInfo->offset = 0ull;
      accelInfo->size = accelSizes->accelerationStructureSize;

      //
      accelGeomInfo->type = accelInfo->type;
      accelGeomInfo->scratchData = vk::DeviceOrHostAddressKHR(ZNAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->geometryScratch)->getDeviceAddress());
      accelGeomInfo->srcAccelerationStructure = accelGeomInfo->dstAccelerationStructure;
      accelGeomInfo->dstAccelerationStructure = (this->accelStruct = device.createAccelerationStructureKHR(accelInfo.ref(), nullptr, deviceObj->getDispatch()));

      // 
      this->handle = device.getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR{ .accelerationStructure = this->accelStruct }, deviceObj->getDispatch());

      //
      return std::get<0>(*this->buildStructure())->get();
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) {
      this->base = deviceObj->getHandle();
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());
      decltype(auto) device = this->base.as<vk::Device>();
      //decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);

      //
      decltype(auto) accelInfo = infoMap->set(vk::StructureType::eAccelerationStructureCreateInfoKHR, vk::AccelerationStructureCreateInfoKHR{
        .createFlags = vk::AccelerationStructureCreateFlagsKHR{},
        .type = vk::AccelerationStructureTypeKHR::eBottomLevel
      });

      //
      decltype(auto) accelGeomInfo = infoMap->set(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR, vk::AccelerationStructureBuildGeometryInfoKHR{
        .type = accelInfo->type,
        .flags = vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate | vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
        .mode = vk::BuildAccelerationStructureModeKHR::eBuild
      });

      //
      if (this->cInfo->geometries.size() > 0 && !this->handle) {
        this->createStructure();
      };

      //
      if (!this->handle) {
        this->handle = uintptr_t(this);
      };
    };
    
  };

};
