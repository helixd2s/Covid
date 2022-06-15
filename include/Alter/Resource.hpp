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
    class ResourceObj : public BaseObj {
    public:
        using tType = WrapShared<ResourceObj>;
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
        //vk::Buffer buffer = {};
        //vk::Image image = {};
        //vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
        void* mappedMemory = nullptr;
        uintptr_t deviceAddress = 0ull;

        // 
        std::shared_ptr<AllocatedMemory> allocated = {};
        std::optional<ResourceCreateInfo> cInfo = ResourceCreateInfo{};
        std::optional<MemoryRequirements> mReqs = {};
        //std::shared_ptr<MSS> infoMap = {};

        //
        //std::shared_ptr<DeviceObj> deviceObj = {};

        //
        std::vector<vk::ImageView> imageViews = {};
        std::vector<vk::BufferView> bufferViews = {};

        //
        vk::BufferUsageFlags bufferUsage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
        vk::ImageUsageFlags imageUsage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
        MemoryUsage memoryUsage = MemoryUsage::eGpuOnly;

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

        //
        vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;


    public:
        // 
        ResourceObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            //this->construct(deviceObj, cInfo);
        };

        // 
        ResourceObj(Handle const& handle, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
            //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
        };

        // 
        virtual vk::ImageAspectFlagBits aspectMask() {
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
            return this->cInfo->imageInfo->type == ImageType::eDepthStencilAttachment ? (vk::ImageAspectFlagBits::eDepth) :
                (this->cInfo->imageInfo->type == ImageType::eDepthAttachment ? vk::ImageAspectFlagBits::eDepth :
                    (this->cInfo->imageInfo->type == ImageType::eStencilAttachment ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor));
        };

        //
        virtual vk::ImageSubresourceRange subresourceRange(uint32_t baseArrayLayout = 0u, uint32_t layerCount = 1u, uint32_t baseMipLevel = 0u, uint32_t levelCount = 1u) {
            return vk::ImageSubresourceRange{
              .aspectMask = this->aspectMask(),
              .baseMipLevel = baseMipLevel,
              .levelCount = levelCount,
              .baseArrayLayer = baseArrayLayout,
              .layerCount = layerCount
            };
        };

        //
        virtual vk::ImageSubresourceLayers subresourceLayers(uint32_t baseArrayLayout = 0u, uint32_t layerCount = 1u, uint32_t mipLevel = 0u) {
            return vk::ImageSubresourceLayers{
              .aspectMask = this->aspectMask(),
              .mipLevel = mipLevel,
              .baseArrayLayer = baseArrayLayout,
              .layerCount = layerCount
            };
        };

        //
        virtual vk::ComponentMapping componentMapping(cpp21::optional_ref<vk::ComponentMapping> mapping_) {
            decltype(auto) mapping = vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eG, .b = vk::ComponentSwizzle::eB, .a = vk::ComponentSwizzle::eA };
            decltype(auto) imageInfo = this->cInfo->imageInfo;

            // 
            if (imageInfo->type == ImageType::eDepthStencilAttachment) {
                mapping.r = vk::ComponentSwizzle::eR,
                    mapping.g = vk::ComponentSwizzle::eG,
                    mapping.b = vk::ComponentSwizzle::eZero,
                    mapping.a = vk::ComponentSwizzle::eZero;
            }
            else
                if (imageInfo->type == ImageType::eDepthAttachment || imageInfo->type == ImageType::eStencilAttachment) {
                    mapping.r = vk::ComponentSwizzle::eR,
                        mapping.g = vk::ComponentSwizzle::eZero,
                        mapping.b = vk::ComponentSwizzle::eZero,
                        mapping.a = vk::ComponentSwizzle::eZero;
                }
                else
                    if (mapping_) {
                        mapping = mapping_.value();
                    }
                    else
                        if (cpp21::orEqual(this->cInfo->imageInfo->format, std::vector<vk::Format>{vk::Format::eR8G8B8A8Unorm, vk::Format::eR16G16B16A16Unorm, vk::Format::eR16G16B16A16Sfloat, vk::Format::eR32G32B32A32Sfloat})) {
                            mapping.r = vk::ComponentSwizzle::eR,
                                mapping.g = vk::ComponentSwizzle::eG,
                                mapping.b = vk::ComponentSwizzle::eB,
                                mapping.a = vk::ComponentSwizzle::eA;
                        }
                        else
                            if (cpp21::orEqual(this->cInfo->imageInfo->format, std::vector<vk::Format>{vk::Format::eR8G8B8Unorm, vk::Format::eR16G16B16Unorm, vk::Format::eR16G16B16Sfloat, vk::Format::eR32G32B32Sfloat})) {
                                mapping.r = vk::ComponentSwizzle::eR,
                                    mapping.g = vk::ComponentSwizzle::eG,
                                    mapping.b = vk::ComponentSwizzle::eB,
                                    mapping.a = vk::ComponentSwizzle::eOne;
                            }
                            else
                                if (cpp21::orEqual(this->cInfo->imageInfo->format, std::vector<vk::Format>{vk::Format::eR8G8Unorm, vk::Format::eR16G16Unorm, vk::Format::eR16G16Sfloat, vk::Format::eR32G32Sfloat})) {
                                    mapping.r = vk::ComponentSwizzle::eR,
                                        mapping.g = vk::ComponentSwizzle::eG,
                                        mapping.b = vk::ComponentSwizzle::eB,
                                        mapping.a = vk::ComponentSwizzle::eOne;
                                }
                                else
                                    if (cpp21::orEqual(this->cInfo->imageInfo->format, std::vector<vk::Format>{vk::Format::eR8Unorm, vk::Format::eR16Unorm, vk::Format::eR16Sfloat, vk::Format::eR32Sfloat})) {
                                        mapping.r = vk::ComponentSwizzle::eR,
                                            mapping.g = vk::ComponentSwizzle::eG,
                                            mapping.b = vk::ComponentSwizzle::eB,
                                            mapping.a = vk::ComponentSwizzle::eA;
                                    };

            // 
            return mapping;
        };

        //
        ImageViewIndex createImageView(cpp21::optional_ref<ImageViewCreateInfo> info = {}) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) descriptorsObj = this->cInfo->descriptors ? deviceObj->get<PipelineLayoutObj>(this->cInfo->descriptors) : WrapShared<PipelineLayoutObj>{};
            decltype(auto) imageInfo = this->cInfo->imageInfo;

            // 
            decltype(auto) components = this->componentMapping(info->componentMapping);
            decltype(auto) imageView = device.createImageView(vk::ImageViewCreateInfo{
              .image = this->handle.as<vk::Image>(),
              .viewType = info->viewType,
              .format = imageInfo->format,
              .components = components,
              .subresourceRange = vk::ImageSubresourceRange(info->subresourceRange ? info->subresourceRange.value() : this->subresourceRange(0u,imageInfo->layerCount,0u,imageInfo->mipLevelCount)).setAspectMask(this->aspectMask())
                });

            //
            this->imageViews.push_back(imageView);
            uint32_t descriptorId = 0xFFFFFFFFu;

            // 
            uint32_t imvType = 0u;
            if (descriptorsObj) {
                if ((this->imageUsage & vk::ImageUsageFlagBits::eStorage) && info->preference == ImageViewPreference::eStorage || !(this->imageUsage & vk::ImageUsageFlagBits::eSampled)) {
                    descriptorId = descriptorsObj->images.add(vk::DescriptorImageInfo{ .imageView = imageView, .imageLayout = this->getImageLayout() }); imvType = 1u;
                }
                else
                    if ((this->imageUsage & vk::ImageUsageFlagBits::eSampled) && info->preference == ImageViewPreference::eSampled || !(this->imageUsage & vk::ImageUsageFlagBits::eStorage)) {
                        descriptorId = descriptorsObj->textures.add(vk::DescriptorImageInfo{ .imageView = imageView, .imageLayout = this->getImageLayout() }); imvType = 2u;
                    };

                // 
                this->destructors.insert(this->destructors.begin(), 1, std::make_shared<std::function<DFun>>([device, descriptorId, images = (imvType == 1u ? descriptorsObj->getImageDescriptors() : descriptorsObj->getTextureDescriptors())](BaseObj const* baseObj) {
                    const_cast<cpp21::bucket<vk::DescriptorImageInfo>&>(images).removeByIndex(descriptorId);
                }));
            };

            //
            this->destructors.insert(this->destructors.begin(), 1, std::make_shared<std::function<DFun>>([device, imageView](BaseObj const* baseObj) {
                device.destroyImageView(imageView);
                }));

            // 
            return ImageViewIndex{ imageView, descriptorId }; // don't return reference, may broke vector
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
        inline static tType make(Handle const& handle, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
            auto shared = std::make_shared<ResourceObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

        //
        virtual uintptr_t& getDeviceAddress() { return this->deviceAddress; };
        virtual uintptr_t const& getDeviceAddress() const { return this->deviceAddress; };

        //
        virtual vk::ImageLayout& getImageLayout() { return imageLayout; };
        virtual vk::ImageLayout const& getImageLayout() const { return imageLayout; };

        //
        virtual vk::ImageUsageFlags& getImageUsage() { return imageUsage; };
        virtual vk::ImageUsageFlags const& getImageUsage() const { return imageUsage; };

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
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
            if (cInfo) { this->cInfo = cInfo; };
            if (this->cInfo) {
                if (this->cInfo->imageInfo) { this->createImage(this->cInfo->imageInfo.value()); };
                if (this->cInfo->bufferInfo) { this->createBuffer(this->cInfo->bufferInfo.value()); };
            };
        };

        //
        virtual vk::ImageUsageFlags& handleImageUsage(ImageType const& imageType) {
            // 
            switch (imageType) {
            case ImageType::eSwapchain:
                memoryUsage = MemoryUsage::eGpuOnly;
                imageUsage |= vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
                break;

            case ImageType::eStorage:
                memoryUsage = MemoryUsage::eGpuOnly;
                imageUsage |= vk::ImageUsageFlagBits::eStorage;
                break;

            case ImageType::eTexture:
                memoryUsage = MemoryUsage::eGpuOnly;
                imageUsage |= vk::ImageUsageFlagBits::eSampled;
                break;

            case ImageType::eColorAttachment:
                memoryUsage = MemoryUsage::eGpuOnly;
                imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
                break;

            case ImageType::eDepthAttachment:
                memoryUsage = MemoryUsage::eGpuOnly;
                imageUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
                break;

            case ImageType::eDepthStencilAttachment:
                memoryUsage = MemoryUsage::eGpuOnly;
                imageUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
                break;

            case ImageType::eStencilAttachment:
                memoryUsage = MemoryUsage::eGpuOnly;
                imageUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
                break;

            case ImageType::eUniversal:
                memoryUsage = MemoryUsage::eGpuOnly;
                imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
                break;

            default:;
            };
            return imageUsage;
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
        virtual cpp21::wrap_shared_ptr<vk::ImageCreateInfo> makeImageCreateInfo(ImageCreateInfo const& cInfo) {
            //
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryImageCreateInfo, vk::ExternalMemoryImageCreateInfo{
              .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
                });

            // 
            decltype(auto) imageUsage = this->handleImageUsage(cInfo.type);
            decltype(auto) imageInfo = infoMap->set(vk::StructureType::eImageCreateInfo, vk::ImageCreateInfo{
              .pNext = memoryUsage == MemoryUsage::eGpuOnly ? externalInfo.get() : nullptr,
              .flags = cInfo.flags,
              .imageType = cInfo.imageType,
              .format = cInfo.format,
              .extent = cInfo.extent,
              .mipLevels = cInfo.mipLevelCount,
              .arrayLayers = cInfo.layerCount, // TODO: correct array layers
              .samples = vk::SampleCountFlagBits::e1,
              .tiling = vk::ImageTiling::eOptimal,
              .usage = imageUsage,
              .sharingMode = vk::SharingMode::eExclusive,
              .initialLayout = vk::ImageLayout::eUndefined
                });

            //
            imageInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices);
            return imageInfo;
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
        virtual FenceType createImage(cpp21::optional_ref<ImageCreateInfo> cInfo = {}) {
            // default layout
            this->getImageLayout() = cInfo->layout;

            // 
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) imageInfo = this->makeImageCreateInfo(cInfo);
            decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorObj>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocator);

            //
            this->allocated = std::make_shared<AllocatedMemory>();
            if (this->cInfo->image && *this->cInfo->image) {
                this->handle = this->cInfo->image.value();
            }
            else {
                this->handle = memoryAllocatorObj->createImageAndAllocateMemory(allocated, MemoryRequirements{ .memoryUsage = memoryUsage }, infoMap, destructors);
            };

            // 
            if (cInfo->info) {
                // # another reason of internal error (mostly with std::optional)
                return this->executeSwitchLayoutOnce(ImageLayoutSwitchInfo{
                  .info = cInfo->info,
                  .switchInfo = ImageLayoutSwitchWriteInfo{
                    .newImageLayout = this->getImageLayout(),
                    .oldImageLayout = std::optional<vk::ImageLayout>(imageInfo->initialLayout),
                  },
                    });
            };

            return FenceType{};
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
        virtual void writeClearCommand(cpp21::optional_ref<ImageClearWriteInfo> clearInfo) {
            if (this->cInfo->imageInfo && this->handle.type == HandleType::eImage) {
                //decltype(auto) info = switchInfo.info ? switchInfo.info : this->cInfo->imageInfo->info;
                decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
                decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
                decltype(auto) subresourceRange = clearInfo->subresourceRange ? clearInfo->subresourceRange.value() : this->subresourceRange(0u, imageInfo->arrayLayers, 0u, imageInfo->mipLevels);

                // 
                bool isValidLayout = imageLayout == vk::ImageLayout::eGeneral || imageLayout == vk::ImageLayout::eSharedPresentKHR || imageLayout == vk::ImageLayout::eTransferDstOptimal;
                auto clearColor = reinterpret_cast<vk::ClearColorValue const&>(clearInfo->clearColor);
                auto clearValue = vk::ClearValue{ .color = clearColor };
                if (!isValidLayout) {
                    this->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{ .cmdBuf = clearInfo->cmdBuf, .newImageLayout = vk::ImageLayout::eTransferDstOptimal, .queueFamilyIndex = clearInfo->queueFamilyIndex });
                };
                clearInfo->cmdBuf.clearColorImage(this->handle.as<vk::Image>(), isValidLayout ? imageLayout : vk::ImageLayout::eTransferDstOptimal, clearColor, std::vector<vk::ImageSubresourceRange>{ subresourceRange });
                if (!isValidLayout) {
                    this->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{ .cmdBuf = clearInfo->cmdBuf, .newImageLayout = imageLayout, .queueFamilyIndex = clearInfo->queueFamilyIndex });
                };
            };
        };

        //
        virtual void writeSwitchLayoutCommand(cpp21::optional_ref<ImageLayoutSwitchWriteInfo> switchInfo) {
            if (this->cInfo->imageInfo && this->handle.type == HandleType::eImage) {
                //decltype(auto) info = switchInfo.info ? switchInfo.info : this->cInfo->imageInfo->info;
                decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
                decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
                decltype(auto) oldImageLayout = switchInfo->oldImageLayout ? switchInfo->oldImageLayout.value() : this->getImageLayout();

                // cupola
                if (oldImageLayout != switchInfo->newImageLayout) {
                    decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
                    decltype(auto) externalAccessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(this->getImageUsage()));
                    decltype(auto) correctAccessMask = vku::getCorrectAccessMaskByImageLayout<vk::AccessFlagBits2>(switchInfo->newImageLayout);
                    decltype(auto) transferBarrier = std::vector<vk::ImageMemoryBarrier2>{
                      vk::ImageMemoryBarrier2{
                        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(externalAccessMask) | (externalAccessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                        .srcAccessMask = externalAccessMask,
                        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(correctAccessMask) | (correctAccessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                        .dstAccessMask = correctAccessMask,
                        .oldLayout = oldImageLayout,
                        .newLayout = switchInfo->newImageLayout,
                        .srcQueueFamilyIndex = switchInfo->queueFamilyIndex, // TODO: SRC queueFamilyIndex
                        .dstQueueFamilyIndex = switchInfo->queueFamilyIndex, // TODO: DST queueFamilyIndex
                        .image = this->handle.as<vk::Image>(),
                        .subresourceRange = switchInfo->subresourceRange ? switchInfo->subresourceRange.value() : this->subresourceRange(0u,imageInfo->arrayLayers,0u,imageInfo->mipLevels)
                      }
                    };
                    switchInfo->cmdBuf.pipelineBarrier2(depInfo.setImageMemoryBarriers(transferBarrier));
                    this->getImageLayout() = switchInfo->newImageLayout;
                };
            };

            //return SFT();
        };

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
            if (this->cInfo->bufferInfo && this->handle.type == HandleType::eBuffer) {
                info->commandInits.push_back([this, bufferInfo, depInfo, bufferBarriersBegin, bufferBarriersEnd](vk::CommandBuffer const& cmdBuf) {
                    auto _depInfo = depInfo;
                    cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
                    cmdBuf.fillBuffer(this->handle.as<vk::Buffer>(), 0ull, bufferInfo->size, 0u);
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

        //
        virtual FenceType executeSwitchLayoutOnce(cpp21::optional_ref<ImageLayoutSwitchInfo> execInfo = {}) {
            decltype(auto) switchInfo = execInfo->switchInfo.value();
            decltype(auto) info = execInfo->info ? execInfo->info : this->cInfo->imageInfo->info;
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
            decltype(auto) oldImageLayout = switchInfo.oldImageLayout ? switchInfo.oldImageLayout.value() : this->cInfo->imageInfo->layout;
            decltype(auto) submission = CommandOnceSubmission{ .submission = execInfo->submission };

            // cupola
            if (oldImageLayout != switchInfo.newImageLayout) {
                if (this->cInfo->imageInfo && this->handle.type == HandleType::eImage) {
                    submission.commandInits.push_back([this, switchInfo](vk::CommandBuffer const& cmdBuf) {
                        this->writeSwitchLayoutCommand(switchInfo.with(cmdBuf));
                        return cmdBuf;
                        });

                    //
                    //this->cInfo->imageInfo->layout = switchInfo.newImageLayout;
                    return deviceObj->executeCommandOnce(submission);
                };
            };

            // 
            return FenceType{};
        };

    };

    //
    inline void PipelineLayoutObj::createNullImages() {
        //
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

        //
        decltype(auto) textureObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
          .descriptors = this->handle.as<vk::PipelineLayout>(),
          .imageInfo = ANAMED::ImageCreateInfo{
            .format = vk::Format::eR8G8B8A8Unorm,
            .extent = vk::Extent3D{2u, 2u, 1u},
            .type = ANAMED::ImageType::eTexture
          }
            });

        //
        decltype(auto) imageObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
          .descriptors = this->handle.as<vk::PipelineLayout>(),
          .imageInfo = ANAMED::ImageCreateInfo{
            .format = vk::Format::eR8G8B8A8Unorm,
            .extent = vk::Extent3D{2u, 2u, 1u},
            .type = ANAMED::ImageType::eStorage
          }
            });

        //
        decltype(auto) texImageView = textureObj->createImageView(ANAMED::ImageViewCreateInfo{ .viewType = vk::ImageViewType::e2D });
        decltype(auto) imgImageView = imageObj->createImageView(ANAMED::ImageViewCreateInfo{ .viewType = vk::ImageViewType::e2D });
        decltype(auto) samplerObj = ANAMED::SamplerObj::make(deviceObj, ANAMED::SamplerCreateInfo{
          .descriptors = this->handle.as<vk::PipelineLayout>(),
          .native = vk::SamplerCreateInfo {
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat
          }
            });

        //
        this->nullImage = imageObj.as<vk::Image>();
        this->nullSampler = samplerObj.as<vk::Sampler>();
        this->nullTexture = textureObj.as<vk::Image>();
    };

    //
    inline vk::Buffer& PipelineLayoutObj::createUniformBuffer() {
        this->uniformBuffer = (this->uniformBufferObj = ResourceObj::make(this->base, ResourceCreateInfo{
            .bufferInfo = BufferCreateInfo{
              .size = uniformSize,
              .type = BufferType::eUniform
            }
        })).as<vk::Buffer>();
        this->uniformBufferDesc = vk::DescriptorBufferInfo{ this->uniformBuffer, 0ull, this->uniformSize };
        return this->uniformBuffer;
    };

    // 
    inline WrapShared<DeviceObj> DeviceObj::writeCopyBuffersCommand(CopyBufferWriteInfo const& copyInfoRaw) {
        //decltype(auto) submission = CommandOnceSubmission{ .info = QueueGetInfo {.queueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex } };
        decltype(auto) device = this->base.as<vk::Device>();
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) size = std::min(copyInfoRaw.src->region.size, copyInfoRaw.dst->region.size);
        decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = copyInfoRaw.src->buffer, .dstBuffer = copyInfoRaw.dst->buffer };
        decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
        decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{.srcOffset = copyInfoRaw.src->region.offset, .dstOffset = copyInfoRaw.dst->region.offset, .size = size} };

        //
        decltype(auto) srcAccessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(deviceObj->get<ResourceObj>(copyInfoRaw.src->buffer)->getBufferUsage()));
        decltype(auto) dstAccessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(deviceObj->get<ResourceObj>(copyInfoRaw.dst->buffer)->getBufferUsage()));

        //
        decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(srcAccessMask),
            .srcAccessMask = srcAccessMask,
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
            .srcQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
            .dstQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
            .buffer = copyInfoRaw.src->buffer,
            .offset = copyInfoRaw.src->region.offset,
            .size = size
          },
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(dstAccessMask),
            .srcAccessMask = dstAccessMask,
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
            .srcQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
            .dstQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
            .buffer = copyInfoRaw.dst->buffer,
            .offset = copyInfoRaw.dst->region.offset,
            .size = size
          }
        };

        //
        decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(srcAccessMask),
            .dstAccessMask = srcAccessMask,
            .srcQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
            .dstQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
            .buffer = copyInfoRaw.src->buffer,
            .offset = copyInfoRaw.src->region.offset,
            .size = size
          },
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(dstAccessMask),
            .dstAccessMask = dstAccessMask,
            .srcQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
            .dstQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
            .buffer = copyInfoRaw.dst->buffer,
            .offset = copyInfoRaw.dst->region.offset,
            .size = size
          }
        };

        // 
        //submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _copyInfo = copyInfo;
        auto _depInfo = depInfo;
        copyInfoRaw.cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
        copyInfoRaw.cmdBuf.copyBuffer2(_copyInfo.setRegions(regions));
        copyInfoRaw.cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
        //});

        //
        //return this->executeCommandOnce(submission);
        return this->SFT();
    };


    // you can copy from host to device Buffer and Image together!
    // TODO: per-type role based barriers...
    // TODO: image, imageType and imageLayout supports...


    inline size_t UploaderObj::getImagePixelSize(vk::Image const& image) {
        decltype(auto) device = this->base.as<vk::Device>();
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) imageObj = deviceObj->get<ResourceObj>(image);
        if (imageObj) {
            decltype(auto) cInfo = imageObj->infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
            if (cInfo) {
                return vku::vk_format_table.at(VkFormat(cInfo->format)).size;
            };
        };
        return VK_WHOLE_SIZE;
    };

    //
    inline WrapShared<UploaderObj> UploaderObj::writeUploadToResourceCmd(cpp21::optional_ref<UploadCommandWriteInfo> copyRegionInfo) {
        //decltype(auto) submission = CommandOnceSubmission{ .info = this->cInfo->info };
        decltype(auto) mappedBuffer = copyRegionInfo->bunchBuffer ? copyRegionInfo->bunchBuffer : this->mappedBuffer;
        decltype(auto) device = this->base.as<vk::Device>();
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) size = copyRegionInfo->dstBuffer ? copyRegionInfo->dstBuffer->region.size : VK_WHOLE_SIZE;
        decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
        decltype(auto) hostMapOffset = /*hostMapOffset_ ? (*hostMapOffset_) :*/ copyRegionInfo->bunchBuffer ? 0ull : copyRegionInfo->hostMapOffset;

        //
        decltype(auto) subresourceRange = vk::ImageSubresourceRange{};
        decltype(auto) subresourceLayers = vk::ImageSubresourceLayers{};

        // 
        decltype(auto) BtI = vk::CopyBufferToImageInfo2{};
        decltype(auto) BtB = vk::CopyBufferInfo2{};
        decltype(auto) BtBRegions = std::vector<vk::BufferCopy2>{  };
        decltype(auto) BtIRegions = std::vector<vk::BufferImageCopy2>{  };

        //
        decltype(auto) imageBarriersBegin = std::vector<vk::ImageMemoryBarrier2>{};
        decltype(auto) imageBarriersEnd = std::vector<vk::ImageMemoryBarrier2>{};

        //
        decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .buffer = mappedBuffer,
            .offset = hostMapOffset,
            .size = size
          }
        };

        //
        decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .buffer = mappedBuffer,
            .offset = hostMapOffset,
            .size = size
          }
        };

        // 
        if (copyRegionInfo->dstImage) {
            auto& imageRegion = copyRegionInfo->dstImage.value();

            //
            decltype(auto) imageObj = deviceObj->get<ResourceObj>(imageRegion.image);
            decltype(auto) imageInfo = imageObj->infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);

            // 
            subresourceRange = imageObj->subresourceRange(imageRegion.region.baseLayer, imageRegion.region.layerCount, imageRegion.region.baseMipLevel, 1u);
            subresourceLayers = vk::ImageSubresourceLayers{
              .aspectMask = subresourceRange.aspectMask,
              .mipLevel = subresourceRange.baseMipLevel,
              .baseArrayLayer = subresourceRange.baseArrayLayer,
              .layerCount = subresourceRange.layerCount
            };

            BtIRegions.push_back(vk::BufferImageCopy2{
              .bufferOffset = hostMapOffset, .imageSubresource = subresourceLayers, .imageOffset = imageRegion.region.offset, .imageExtent = imageRegion.region.extent
                });
            BtI = vk::CopyBufferToImageInfo2{ .srcBuffer = mappedBuffer, .dstImage = imageRegion.image, .dstImageLayout = vk::ImageLayout::eTransferDstOptimal };

            //
            decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(imageObj->getImageUsage()));

            //
            imageBarriersBegin.push_back(vk::ImageMemoryBarrier2{
                .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                .srcAccessMask = vk::AccessFlagBits2(accessMask),
                .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
                .oldLayout = imageObj->cInfo->imageInfo->layout,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .srcQueueFamilyIndex = imageRegion.queueFamilyIndex,
                .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                .image = imageRegion.image,
                .subresourceRange = subresourceRange
                });

            //
            imageBarriersEnd.push_back(vk::ImageMemoryBarrier2{
                .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
                .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                .dstAccessMask = vk::AccessFlagBits2(accessMask),
                .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                .newLayout = imageObj->cInfo->imageInfo->layout,
                .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                .dstQueueFamilyIndex = imageRegion.queueFamilyIndex,
                .image = imageRegion.image,
                .subresourceRange = subresourceRange
                });
        };

        // 
        if (copyRegionInfo->dstBuffer) {
            auto& bufferRegion = copyRegionInfo->dstBuffer.value();

            //
            decltype(auto) bufferObj = deviceObj->get<ResourceObj>(bufferRegion.buffer);
            decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(bufferObj->getBufferUsage()));

            //
            BtBRegions.push_back(vk::BufferCopy2{ .srcOffset = hostMapOffset, .dstOffset = bufferRegion.region.offset, .size = size });
            BtB = vk::CopyBufferInfo2{ .srcBuffer = mappedBuffer, .dstBuffer = bufferRegion.buffer };

            //
            bufferBarriersBegin.push_back(vk::BufferMemoryBarrier2{
              .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
              .srcAccessMask = accessMask,
              .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
              .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
              .srcQueueFamilyIndex = bufferRegion.queueFamilyIndex,
              .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
              .buffer = bufferRegion.buffer,
              .offset = bufferRegion.region.offset,
              .size = size
                });

            //
            bufferBarriersEnd.push_back(vk::BufferMemoryBarrier2{
              .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
              .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
              .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
              .dstAccessMask = accessMask,
              .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
              .dstQueueFamilyIndex = bufferRegion.queueFamilyIndex,
              .buffer = bufferRegion.buffer,
              .offset = bufferRegion.region.offset,
              .size = size
                });
        };

        // 
        copyRegionInfo->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin).setImageMemoryBarriers(imageBarriersBegin));
        if (copyRegionInfo->dstImage) copyRegionInfo->cmdBuf.copyBufferToImage2(BtI.setRegions(BtIRegions));
        if (copyRegionInfo->dstBuffer) copyRegionInfo->cmdBuf.copyBuffer2(BtB.setRegions(BtBRegions));
        copyRegionInfo->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd).setImageMemoryBarriers(imageBarriersEnd));

        //
        return SFT();
    };

    //
    inline WrapShared<UploaderObj> UploaderObj::writeDownloadToResourceCmd(cpp21::optional_ref<DownloadCommandWriteInfo> info) {
        decltype(auto) mappedBuffer = info->bunchBuffer ? info->bunchBuffer : this->mappedBuffer;
        decltype(auto) device = this->base.as<vk::Device>();
        decltype(auto) regions = std::vector<vk::BufferCopy2>{  };
        decltype(auto) copyInfo = vk::CopyBufferInfo2{};
        decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
        decltype(auto) size = info->srcBuffer ? info->srcBuffer->region.size : VK_WHOLE_SIZE;
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) hostMapOffset = /*hostMapOffset_ ? (*hostMapOffset_) :*/ info->bunchBuffer ? 0ull : info->hostMapOffset;

        //
        decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .buffer = mappedBuffer,
            .offset = hostMapOffset,
            .size = size
          },
        };

        //
        decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
          vk::BufferMemoryBarrier2{
            .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapWrite),
            .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapWrite),
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eHostMapRead),
            .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eHostMapRead),
            .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
            .buffer = mappedBuffer,
            .offset = hostMapOffset,
            .size = size
          }
        };

        // 
        if (info->srcBuffer) {
            //
            decltype(auto) bufferObj = deviceObj->get<ResourceObj>(info->srcBuffer->buffer);
            decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(bufferObj->getBufferUsage()));

            //
            copyInfo = vk::CopyBufferInfo2{ .srcBuffer = info->srcBuffer->buffer, .dstBuffer = mappedBuffer };
            regions.push_back(vk::BufferCopy2{ .srcOffset = info->srcBuffer->region.offset, .dstOffset = hostMapOffset, .size = size });

            //
            bufferBarriersBegin.push_back(vk::BufferMemoryBarrier2{
              .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask),
              .srcAccessMask = accessMask,
              .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
              .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
              .srcQueueFamilyIndex = info->srcBuffer->queueFamilyIndex,
              .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
              .buffer = info->srcBuffer->buffer,
              .offset = info->srcBuffer->region.offset,
              .size = size
                });

            //
            bufferBarriersEnd.push_back(vk::BufferMemoryBarrier2{
              .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
              .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
              .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask),
              .dstAccessMask = accessMask,
              .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
              .dstQueueFamilyIndex = info->srcBuffer->queueFamilyIndex,
              .buffer = info->srcBuffer->buffer,
              .offset = info->srcBuffer->region.offset,
              .size = size
                });
        };

        if (info->srcBuffer) {
            info->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
            info->cmdBuf.copyBuffer2(copyInfo.setRegions(regions));
            info->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
        };

        return SFT();
    };

    /*
    //
    inline vk::Buffer& PipelineLayoutObj::createCacheBuffer() {
        this->cacheBuffer = (this->cacheBufferObj = ResourceObj::make(this->base, ResourceCreateInfo{
            .bufferInfo = BufferCreateInfo{
              .size = this->cachePages * this->cachePageSize,
              .type = BufferType::eStorage
            }
        })).as<vk::Buffer>();

        //
        for (uint32_t i = 0; i < this->cachePages; i++) {
            this->cacheBufferDescs.push_back(vk::DescriptorBufferInfo{ this->cacheBuffer, i * this->cachePageSize, this->cachePageSize });
        };

        //
        return this->cacheBuffer;
    };*/

};
#endif
