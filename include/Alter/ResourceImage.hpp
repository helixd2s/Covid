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
    class ResourceImageObj : public BaseObj {
    public:
        using tType = WrapShared<ResourceImageObj>;
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
        vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
        vk::ImageUsageFlags imageUsage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
        MemoryUsage memoryUsage = MemoryUsage::eGpuOnly;

        // 
        std::shared_ptr<AllocatedMemory> allocated = {};
        std::optional<ImageCreateInfo> cInfo = ImageCreateInfo{};
        std::optional<MemoryRequirements> mReqs = {};
        std::vector<vk::ImageView> imageViews = {};

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    public:
        // 
        ResourceImageObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<ImageCreateInfo> cInfo = ImageCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            //this->construct(deviceObj, cInfo);
        };

        // 
        ResourceImageObj(Handle const& handle, cpp21::optional_ref<ImageCreateInfo> cInfo = ImageCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
            //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
        };

        // 
        virtual vk::ImageAspectFlagBits aspectMask() {
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
            return   cInfo->type == ImageType::eDepthStencilAttachment ? (vk::ImageAspectFlagBits::eDepth) :
                    (cInfo->type == ImageType::eDepthAttachment ? vk::ImageAspectFlagBits::eDepth :
                    (cInfo->type == ImageType::eStencilAttachment ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor));
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
            decltype(auto) imageInfo = this->cInfo;

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
                        if (cpp21::orEqual(imageInfo->format, std::vector<vk::Format>{vk::Format::eR8G8B8A8Unorm, vk::Format::eR16G16B16A16Unorm, vk::Format::eR16G16B16A16Sfloat, vk::Format::eR32G32B32A32Sfloat})) {
                            mapping.r = vk::ComponentSwizzle::eR,
                                mapping.g = vk::ComponentSwizzle::eG,
                                mapping.b = vk::ComponentSwizzle::eB,
                                mapping.a = vk::ComponentSwizzle::eA;
                        }
                        else
                            if (cpp21::orEqual(imageInfo->format, std::vector<vk::Format>{vk::Format::eR8G8B8Unorm, vk::Format::eR16G16B16Unorm, vk::Format::eR16G16B16Sfloat, vk::Format::eR32G32B32Sfloat})) {
                                mapping.r = vk::ComponentSwizzle::eR,
                                    mapping.g = vk::ComponentSwizzle::eG,
                                    mapping.b = vk::ComponentSwizzle::eB,
                                    mapping.a = vk::ComponentSwizzle::eOne;
                            }
                            else
                                if (cpp21::orEqual(imageInfo->format, std::vector<vk::Format>{vk::Format::eR8G8Unorm, vk::Format::eR16G16Unorm, vk::Format::eR16G16Sfloat, vk::Format::eR32G32Sfloat})) {
                                    mapping.r = vk::ComponentSwizzle::eR,
                                        mapping.g = vk::ComponentSwizzle::eG,
                                        mapping.b = vk::ComponentSwizzle::eB,
                                        mapping.a = vk::ComponentSwizzle::eOne;
                                }
                                else
                                    if (cpp21::orEqual(imageInfo->format, std::vector<vk::Format>{vk::Format::eR8Unorm, vk::Format::eR16Unorm, vk::Format::eR16Sfloat, vk::Format::eR32Sfloat})) {
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
            decltype(auto) imageInfo = this->cInfo;

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
        inline static tType make(Handle const& handle, cpp21::optional_ref<ImageCreateInfo> cInfo = ImageCreateInfo{}) {
            auto shared = std::make_shared<ResourceImageObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

        //
        virtual vk::ImageLayout& getImageLayout() { return imageLayout; };
        virtual vk::ImageLayout const& getImageLayout() const { return imageLayout; };

        //
        virtual vk::ImageUsageFlags& getImageUsage() { return imageUsage; };
        virtual vk::ImageUsageFlags const& getImageUsage() const { return imageUsage; };

        //
        virtual uintptr_t getAllocationOffset() const {
            return this->allocated ? this->allocated->offset : 0;
        };


        virtual ExtHandle& getExtHandle() { return allocated->extHandle; };
        virtual ExtHandle const& getExtHandle() const { return allocated->extHandle; };


    protected:

        // 
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<ImageCreateInfo> cInfo = ImageCreateInfo{}) {
            if (cInfo) { this->cInfo = cInfo; };
            if (this->cInfo) { this->createImage(this->cInfo.value()); };
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

    public:

        //
        virtual void writeClearCommand(cpp21::optional_ref<ImageClearWriteInfo> clearInfo) {
            if (this->cInfo && this->handle.type == HandleType::eImage) {
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
            if (this->cInfo && this->handle.type == HandleType::eImage) {
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
        virtual FenceType executeSwitchLayoutOnce(cpp21::optional_ref<ImageLayoutSwitchInfo> execInfo = {}) {
            decltype(auto) switchInfo = execInfo->switchInfo.value();
            decltype(auto) info = execInfo->info ? execInfo->info : this->cInfo->info;
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
            decltype(auto) oldImageLayout = switchInfo.oldImageLayout ? switchInfo.oldImageLayout.value() : this->cInfo->layout;
            decltype(auto) submission = CommandOnceSubmission{ .submission = execInfo->submission };

            // cupola
            if (oldImageLayout != switchInfo.newImageLayout) {
                if (this->cInfo && this->handle.type == HandleType::eImage) {
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
        decltype(auto) textureObj = ANAMED::ResourceImageObj::make(deviceObj, ANAMED::ImageCreateInfo{
            .descriptors = this->handle.as<vk::PipelineLayout>(),
            .format = vk::Format::eR8G8B8A8Unorm,
            .extent = vk::Extent3D{2u, 2u, 1u},
            .type = ANAMED::ImageType::eTexture
        });

        //
        decltype(auto) imageObj = ANAMED::ResourceImageObj::make(deviceObj, ANAMED::ImageCreateInfo{
            .descriptors = this->handle.as<vk::PipelineLayout>(),
            .format = vk::Format::eR8G8B8A8Unorm,
            .extent = vk::Extent3D{2u, 2u, 1u},
            .type = ANAMED::ImageType::eStorage
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


    // you can copy from host to device Buffer and Image together!
    // TODO: per-type role based barriers...
    // TODO: image, imageType and imageLayout supports...
    inline size_t UploaderObj::getImagePixelSize(vk::Image const& image) {
        decltype(auto) device = this->base.as<vk::Device>();
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) imageObj = deviceObj->get<ResourceImageObj>(image);
        if (imageObj) {
            decltype(auto) cInfo = imageObj->infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
            if (cInfo) {
                return vku::vk_format_table.at(VkFormat(cInfo->format)).size;
            };
        };
        return VK_WHOLE_SIZE;
    };

};
#endif
