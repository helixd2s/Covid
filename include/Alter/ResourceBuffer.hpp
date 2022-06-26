#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Device.hpp"
#include "./MemoryAllocator.hpp"
#include "./PipelineLayout.hpp"
#include "./Uploader.hpp"
#include "./Sampler.hpp"

// 
namespace ANAMED {

    // 
    class ResourceBufferObj : public BaseObj {
    public:
        using tType = WrapShared<ResourceBufferObj>;
        using BaseObj::BaseObj;

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
        friend PingPongObj;

        // 
        uintptr_t deviceAddress = 0ull;
        vk::BufferUsageFlags bufferUsage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
        MemoryUsage memoryUsage = MemoryUsage::eGpuOnly;

        // 
        std::shared_ptr<AllocatedMemory> allocated = {};
        std::optional<BufferCreateInfo> cInfo = BufferCreateInfo{};
        std::optional<MemoryRequirements> mReqs = {};
        std::vector<vk::BufferView> bufferViews = {};
        

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    public:
        // 
        ResourceBufferObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<BufferCreateInfo> cInfo = BufferCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            //this->construct(deviceObj, cInfo);
        };

        // 
        ResourceBufferObj(Handle const& handle, cpp21::optional_ref<BufferCreateInfo> cInfo = BufferCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
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
        inline static tType make(Handle const& handle, cpp21::optional_ref<BufferCreateInfo> cInfo = BufferCreateInfo{}) {
            auto shared = std::make_shared<ResourceBufferObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

        //
        virtual uintptr_t& getDeviceAddress() { return this->deviceAddress; };
        virtual uintptr_t const& getDeviceAddress() const { return this->deviceAddress; };

        //
        virtual vk::BufferUsageFlags& getBufferUsage() { return bufferUsage; };
        virtual vk::BufferUsageFlags const& getBufferUsage() const { return bufferUsage; };

        //
        virtual uintptr_t getAllocationOffset() const {
            return this->allocated ? this->allocated->offset : 0;
        };


        virtual ExtHandle& getExtHandle() { return allocated->extHandle; };
        virtual ExtHandle const& getExtHandle() const { return allocated->extHandle; };


    protected:

        // 
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<BufferCreateInfo> cInfo = BufferCreateInfo{}) {
            if (cInfo) { this->cInfo = cInfo; };
            this->createBuffer(this->cInfo.value());
        };

        //
        virtual vk::BufferUsageFlags& handleBufferUsage(BufferType const& bufferType) {
            // 
            switch (bufferType) {
            case BufferType::eStorage:
                memoryUsage = MemoryUsage::eGpuOnly;
                bufferUsage |=
                    vk::BufferUsageFlagBits::eShaderDeviceAddress |
                    vk::BufferUsageFlagBits::eStorageBuffer |
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                    vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT |
                    vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                    vk::BufferUsageFlagBits::eShaderBindingTableKHR |
                    vk::BufferUsageFlagBits::eStorageTexelBuffer;
                break;

            case BufferType::eVertex:
                memoryUsage = MemoryUsage::eGpuOnly;
                bufferUsage |=
                    vk::BufferUsageFlagBits::eShaderDeviceAddress |
                    vk::BufferUsageFlagBits::eStorageBuffer |
                    vk::BufferUsageFlagBits::eVertexBuffer |
                    vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT |
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                    vk::BufferUsageFlagBits::eStorageTexelBuffer;
                break;

            case BufferType::eIndex:
                memoryUsage = MemoryUsage::eGpuOnly;
                bufferUsage |=
                    vk::BufferUsageFlagBits::eShaderDeviceAddress |
                    vk::BufferUsageFlagBits::eStorageBuffer |
                    vk::BufferUsageFlagBits::eIndexBuffer |
                    vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT |
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                    vk::BufferUsageFlagBits::eStorageTexelBuffer;
                break;

            case BufferType::eHostMap:
                memoryUsage = MemoryUsage::eCpuToGpu;
                bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eUniformTexelBuffer;
                break;

            case BufferType::eUniform:
                memoryUsage = MemoryUsage::eGpuOnly;
                bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eUniformTexelBuffer;
                break;

            case BufferType::eUniversal:
                memoryUsage = MemoryUsage::eGpuOnly;
                bufferUsage |=
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                    vk::BufferUsageFlagBits::eVertexBuffer |
                    vk::BufferUsageFlagBits::eIndexBuffer |
                    vk::BufferUsageFlagBits::eShaderDeviceAddress |
                    vk::BufferUsageFlagBits::eStorageBuffer |
                    vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT |
                    vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT |
                    vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                    vk::BufferUsageFlagBits::eShaderBindingTableKHR |
                    vk::BufferUsageFlagBits::eStorageTexelBuffer
                    ;
                break;

            default:;
            };

            return bufferUsage;
        };

        //
        virtual cpp21::wrap_shared_ptr<vk::BufferCreateInfo> makeBufferCreateInfo(BufferCreateInfo const& cInfo) {
            //
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryBufferCreateInfo, vk::ExternalMemoryBufferCreateInfo{
              .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
                });

            // 
            decltype(auto) bufferUsage = this->handleBufferUsage(cInfo.type);
            decltype(auto) bufferInfo = infoMap->set(vk::StructureType::eBufferCreateInfo, vk::BufferCreateInfo{
              .pNext = memoryUsage == MemoryUsage::eGpuOnly ? externalInfo.get() : nullptr,
              .size = cInfo.size,
              .usage = bufferUsage,
              .sharingMode = vk::SharingMode::eExclusive
                });

            //
            bufferInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices);
            return bufferInfo;
        };

        // 
        virtual FenceType createBuffer(cpp21::optional_ref<BufferCreateInfo> cInfo = {}) {
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) bufferInfo = this->makeBufferCreateInfo(cInfo);
            decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorObj>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocator);

            //
            this->allocated = std::make_shared<AllocatedMemory>();
            if (this->cInfo->buffer && *this->cInfo->buffer) {
                this->handle = this->cInfo->buffer.value();
            }
            else {
                this->handle = memoryAllocatorObj->createBufferAndAllocateMemory(allocated, MemoryRequirements{ .memoryUsage = memoryUsage }, infoMap, destructors);
            };

            // 
            if (bufferInfo->usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
                this->deviceAddress = device.getBufferAddress(vk::BufferDeviceAddressInfo{
                  .buffer = this->handle.as<vk::Buffer>()
                    });
                deviceObj->getAddressSpace().insert({ this->deviceAddress, this->deviceAddress + cInfo->size }, this->handle.as<vk::Buffer>());
            };

            //
            decltype(auto) submission = CommandOnceSubmission{
              .submission = SubmissionInfo{.info = cInfo->info ? cInfo->info.value() : QueueGetInfo{0u, 0u}}
            };

            // 
            return this->executeFillBuffer(submission);
        };

    public:

        //
        virtual FenceType executeFillBuffer(std::optional<CommandOnceSubmission> info) {
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) bufferInfo = infoMap->get<vk::BufferCreateInfo>(vk::StructureType::eBufferCreateInfo);
            decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

            //
            decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{

            };

            //
            decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{

            };

            // injure with current queueFamily
            for (uint32_t q = 0; q < bufferInfo->queueFamilyIndexCount; q++) {
                auto& qf = bufferInfo->pQueueFamilyIndices[q];
                bufferBarriersBegin.push_back(vk::BufferMemoryBarrier2{
                  .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite) | vk::PipelineStageFlagBits2::eAllCommands,
                  .srcAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
                  .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite) | vk::PipelineStageFlagBits2::eAllCommands,
                  .dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
                  .srcQueueFamilyIndex = qf,
                  .dstQueueFamilyIndex = info->submission.info->queueFamilyIndex,
                  .buffer = this->handle.as<vk::Buffer>(),
                  .offset = 0ull,
                  .size = bufferInfo->size
                    });
                bufferBarriersEnd.push_back(vk::BufferMemoryBarrier2{
                  .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite) | vk::PipelineStageFlagBits2::eAllCommands,
                  .srcAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
                  .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite) | vk::PipelineStageFlagBits2::eAllCommands,
                  .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
                  .srcQueueFamilyIndex = info->submission.info->queueFamilyIndex,
                  .dstQueueFamilyIndex = qf,
                  .buffer = this->handle.as<vk::Buffer>(),
                  .offset = 0ull,
                  .size = bufferInfo->size
                    });
            };

            // 
            if (this->cInfo && this->handle.type == HandleType::eBuffer) {
                info->commandInits.push_back([this, bufferInfo, depInfo, bufferBarriersBegin, bufferBarriersEnd, dispatch=deviceObj->getDispatch(), buffer = this->handle.as<vk::Buffer>()](vk::CommandBuffer const& cmdBuf) {
                    auto _depInfo = depInfo;
                    cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
                    cmdBuf.fillBuffer(buffer, 0ull, bufferInfo->size, 0u, dispatch);
                    cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
                    return cmdBuf;
                    });

                //
                //this->cInfo->imageInfo->layout = switchInfo.newImageLayout;
                return deviceObj->executeCommandOnce(info.value());
            };

            // 
            return FenceType{};
        };
    };

    //
    inline vk::Buffer& PipelineLayoutObj::createCacheBuffer() {
        this->cacheBuffer = (this->cacheBufferObj = ResourceBufferObj::make(this->base, BufferCreateInfo{
            .size = this->cachePages * this->cachePageSize,
            .type = BufferType::eStorage
        })).as<vk::Buffer>();

        //
        for (uint32_t i = 0; i < this->cachePages; i++) {
            this->cacheBufferDescs.push_back(vk::DescriptorBufferInfo{ this->cacheBuffer, i * this->cachePageSize, this->cachePageSize });
        };

        //
        return this->cacheBuffer;
    };

    //
    inline vk::Buffer& PipelineLayoutObj::createUniformBuffer() {
        this->uniformBuffer = (this->uniformBufferObj = ResourceBufferObj::make(this->base, BufferCreateInfo{
            .size = uniformSize,
            .type = BufferType::eUniform
        })).as<vk::Buffer>();
        this->uniformBufferDesc = vk::DescriptorBufferInfo{ this->uniformBuffer, 0ull, this->uniformSize };
        return this->uniformBuffer;
    };

};
#endif
