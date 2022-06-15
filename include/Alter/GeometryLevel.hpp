#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

// 
namespace ANAMED {

    // 
    // GEOMETRIES BUFFER BROKEN FOR NO "/fsanitize=address"
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
        WrapShared<ResourceObj> bindGeometryBuffer = {};
        WrapShared<ResourceObj> bindGeometryScratch = {};
        WrapShared<ResourceObj> bindGeometryBuild = {};

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
        GeometryLevelObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            //this->construct(deviceObj, cInfo);
        };

        // 
        GeometryLevelObj(Handle const& handle, cpp21::optional_ref<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
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
        inline static tType make(Handle const& handle, cpp21::optional_ref<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) {
            auto shared = std::make_shared<GeometryLevelObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

        //
        virtual WrapShared<ResourceObj> getGeometryResource() const {
            return ANAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->geometryBuffer);
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
            //
            if (this->cInfo->limits.size() <= 0) {
                for (decltype(auto) geometry : (*this->cInfo->geometries)) {
                    this->cInfo->limits.push_back(geometry.primitiveCount);
                };
            };

            //
            if (this->cInfo->geometries->size() < this->cInfo->limits.size()) {
                for (uintptr_t i = this->cInfo->geometries->size(); i < this->cInfo->limits.size(); i++) {
                    this->cInfo->geometries->push_back(GeometryInfo{});
                };
            };

            //
            this->geometryInfos = {};
            this->geometryRanges = {};
            this->multiDraw = std::vector<vk::MultiDrawInfoEXT>{};
            for (decltype(auto) geometry : (*this->cInfo->geometries)) {
                decltype(auto) vertices = geometry.bufferViews[0];
                geometryInfos.push_back(vk::AccelerationStructureGeometryKHR{
                  .geometryType = vk::GeometryTypeKHR::eTriangles,
                  .geometry = vk::AccelerationStructureGeometryDataKHR{.triangles = vk::AccelerationStructureGeometryTrianglesDataKHR{
                    .vertexFormat = cvtFormatRT(vertices.format),
                    .vertexData = vertices.region.deviceAddress,
                    .vertexStride = vertices.region.stride,
                    .maxVertex = uint32_t(cpp21::tiled(vertices.region.size, vertices.region.stride)),
                    .indexType = cvtIndex(geometry.indices.format),
                    .indexData = geometry.indices.region.deviceAddress,
                    .transformData = geometry.transform.region.deviceAddress
                  }},
                  .flags = geometry.flags
                    });
                geometryRanges.push_back(vk::AccelerationStructureBuildRangeInfoKHR{
                  .primitiveCount = geometry.primitiveCount,
                  .primitiveOffset = 0u,
                  .firstVertex = 0u,
                  .transformOffset = 0u
                    });
                multiDraw->push_back(vk::MultiDrawInfoEXT{
                  .firstVertex = 0u,
                  .vertexCount = geometry.primitiveCount
                    });
            };
        };

        //
        virtual vk::CommandBuffer const& writeBuildStructureCmd(vk::CommandBuffer const& cmdBuf = {}, vk::Buffer const& geometryOffset = {}) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(Handle(this->cInfo->uploader, HandleType::eUploader));
            decltype(auto) accelGeomInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
            decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);
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
            uploaderObj->writeUploadToResourceCmd(UploadCommandWriteInfo{
              .cmdBuf = cmdBuf,
              .bunchBuffer = geometryOffset,
              .dstBuffer = BufferRegion{this->geometryBuffer, DataRegion{ 0ull, sizeof(GeometryInfo), cpp21::bytesize(*this->cInfo->geometries) }}
                });

            //
            cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin).setMemoryBarriers(memoryBarriersBegin));
            cmdBuf.buildAccelerationStructuresKHR(1u, &accelGeomInfo->setGeometries(this->geometryInfos).setMode(accelGeomInfo->srcAccelerationStructure ? vk::BuildAccelerationStructureModeKHR::eUpdate : vk::BuildAccelerationStructureModeKHR::eBuild), cpp21::rvalue_to_ptr(geometryRanges.data()), deviceObj->getDispatch());
            cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd).setMemoryBarriers(memoryBarriersEnd));

            //
            accelGeomInfo->srcAccelerationStructure = accelGeomInfo->dstAccelerationStructure;

            // 
            return cmdBuf;
        };

        //
        virtual FenceType buildStructure(cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}) {
            //
            if (this->cInfo->geometries->size() > 0) {
                if (!this->accelStruct) {
                    this->createStructure(info, false);
                }
                else {
                    this->updateGeometries();
                };
            }
            else {

            };

            //
            if (this->cInfo->geometries->size() > 0) {
                decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info.value() } };
                decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
                decltype(auto) uploaderObj = deviceObj->get<UploaderObj>(Handle(this->cInfo->uploader, HandleType::eUploader));

                //
                decltype(auto) memReqs = uploaderObj->getMemoryRequirements();
                uintptr_t geometryOffset = 0ull;
                uintptr_t geometrySize = cpp21::tiled(cpp21::bytesize(*this->cInfo->geometries), memReqs.alignment) * memReqs.alignment;

                //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
                decltype(auto) geometryAlloc = uploaderObj->allocateMappedTemp(geometrySize, geometryOffset);
                decltype(auto) memPage = uploaderObj->allocatePage(geometryOffset, geometrySize);//allocatePage
#endif

        // 
                memcpy(memPage->mapped, this->cInfo->geometries->data(), geometrySize);

                // TODO: Acceleration Structure Build Barriers per Buffers
                submission.commandInits.push_back([geometryOffset, dispatch = deviceObj->getDispatch(), memPage, this](vk::CommandBuffer const& cmdBuf) {
                    return this->writeBuildStructureCmd(cmdBuf, memPage->bunchBuffer);
                });

                //
#ifdef AMD_VULKAN_MEMORY_ALLOCATOR_H
                submission.submission.onDone.push_back([memPage, mappedBlock = uploaderObj->getMappedBlock(), geometryAlloc, devicePtr = deviceObj.get()](cpp21::optional_ref<vk::Result> result) {
                    vmaVirtualFree(mappedBlock, geometryAlloc);
                    (*memPage->destructor)(devicePtr);
                });
#endif

                //
                //uploaderObj->bindMemoryPages(submission.submission);
                return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
            };
            return FenceType();
        };

        //
        virtual FenceType createStructure(cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}, bool const& needsBuild = true) {
            this->updateGeometries();

            // 
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) accelGeomInfo = infoMap->get<vk::AccelerationStructureBuildGeometryInfoKHR>(vk::StructureType::eAccelerationStructureBuildGeometryInfoKHR);
            decltype(auto) accelSizes = infoMap->set(vk::StructureType::eAccelerationStructureBuildSizesInfoKHR, device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, accelGeomInfo->setGeometries(this->geometryInfos), this->cInfo->limits, deviceObj->getDispatch()));
            decltype(auto) accelInfo = infoMap->get<vk::AccelerationStructureCreateInfoKHR>(vk::StructureType::eAccelerationStructureCreateInfoKHR);

            // 
            this->geometryBuffer = (this->bindGeometryBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
                .bufferInfo = BufferCreateInfo{
                  .size = cpp21::bytesize(*this->cInfo->geometries),
                  .type = BufferType::eStorage
                }
            })).as<vk::Buffer>();

            //
            this->geometryScratch = (this->bindGeometryScratch = ResourceObj::make(this->base, ResourceCreateInfo{
                .bufferInfo = BufferCreateInfo{
                  .size = std::max(accelSizes->buildScratchSize, accelSizes->updateScratchSize),
                  .type = BufferType::eStorage
                }
            })).as<vk::Buffer>();

            //
            this->geometryBuild = (this->bindGeometryBuild = ResourceObj::make(this->base, ResourceCreateInfo{
                .bufferInfo = BufferCreateInfo{
                  .size = accelSizes->accelerationStructureSize,
                  .type = BufferType::eStorage
                }
            })).as<vk::Buffer>();

            //
            //accelInfo->type = vk::AccelerationStructureTypeKHR::eBottomLevel;
            accelInfo->buffer = this->geometryBuild;
            accelInfo->offset = 0ull;
            accelInfo->size = accelSizes->accelerationStructureSize;

            //
            accelGeomInfo->type = accelInfo->type;
            accelGeomInfo->scratchData = reinterpret_cast<vk::DeviceOrHostAddressKHR&>(ANAMED::context->get<DeviceObj>(this->base)->get<ResourceObj>(this->geometryScratch)->getDeviceAddress());
            accelGeomInfo->srcAccelerationStructure = vk::AccelerationStructureKHR{};
            accelGeomInfo->dstAccelerationStructure = (this->accelStruct = device.createAccelerationStructureKHR(accelInfo.value(), nullptr, deviceObj->getDispatch()));

            // 
            this->handle = device.getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR{ .accelerationStructure = this->accelStruct }, deviceObj->getDispatch());

            //
            this->destructors.push_back(std::make_shared<std::function<DFun>>([this, device, accellStruct = accelGeomInfo->dstAccelerationStructure, dispatch = deviceObj->getDispatch()](BaseObj const* baseObj) {
                //device.waitIdle();
                device.destroyAccelerationStructureKHR(accellStruct, nullptr, dispatch);
                this->bindGeometryBuffer->destroy(baseObj);
                this->bindGeometryScratch->destroy(baseObj);
                this->bindGeometryBuild->destroy(baseObj);
            }));

            //
            if (needsBuild) {
                return this->buildStructure(info);
            };
            return FenceType();
        };

        // 
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<GeometryLevelCreateInfo> cInfo = GeometryLevelCreateInfo{}) {
            if (cInfo) { this->cInfo = cInfo; };

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
            if (this->cInfo->geometries->size() > 0 && !this->handle) {
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
