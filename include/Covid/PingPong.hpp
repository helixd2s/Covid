#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./ResourceImage.hpp"
#include "./PipelineLayout.hpp"
#include "./Semaphore.hpp"

// 
namespace ANAMED {

    //
    //struct SurfaceCapabilitiesInfo {
      //vk::SurfaceCapabilities2KHR capabilities2 = {};
      //std::vector<vk::SurfaceFormat2KHR> formats2 = {};
      //std::vector<vk::PresentModeKHR> presentModes = {};
      //vk::PhysicalDeviceSurfaceInfo2KHR info2 = {};
      //cpp21::optional_ref<vk::SurfaceCapabilitiesKHR> capabilities = {};
      //cpp21::optional_ref<vk::SurfaceFormatKHR> formats = {};
    //};

    //
    struct SwapchainSet {
        std::vector<vk::Image> images = {};
        std::vector<vk::ImageView> imageViews = {};
        std::vector<uint32_t> imageViewIndices = {};
        std::vector<std::function<void(vk::CommandBuffer const&)>> switchToPresentFns = {};
        std::vector<std::function<void(vk::CommandBuffer const&)>> switchToReadyFns = {};

        // 
        cpp21::wrap_shared_ptr<vk::SemaphoreSubmitInfo> copySemaphoreInfo = {};
        cpp21::wrap_shared_ptr<vk::SemaphoreSubmitInfo> readySemaphoreInfo = {};
        cpp21::wrap_shared_ptr<vk::SemaphoreSubmitInfo> presentSemaphoreInfo = {};
    };

    // 
    class PingPongObj : public BaseObj {
    public:
        using tType = WrapShared<PingPongObj>;
        using BaseObj::BaseObj;
        //using BaseObj;

    protected:
        // 
        friend DeviceObj;
        friend PipelineObj;
        friend ResourceImageObj;
        bool firstWait = true;

        //
        std::optional<PingPongCreateInfo> cInfo = PingPongCreateInfo{};

        //
        std::vector<SwapchainSet> sets = {};

        //
        PingPongStateInfo currentState = {};

        //
        vk::Rect2D renderArea = {};

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    public:
        // 
        PingPongObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<PingPongCreateInfo> cInfo = PingPongCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            //this->construct(deviceObj, cInfo);
        };

        // 
        PingPongObj(Handle const& handle, cpp21::optional_ref<PingPongCreateInfo> cInfo = PingPongCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
            //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
        };

        //
        SwapchainSet& getCurrentSet() { return this->sets[this->getCurrentIndex()]; };
        SwapchainSet const& getCurrentSet() const { return this->sets[this->getCurrentIndex()]; };

        //
        virtual PingPongStateInfo& getStateInfo() { return currentState; };
        virtual PingPongStateInfo const& getStateInfo() const { return currentState; };
        virtual vk::Rect2D const& getRenderArea() const { return renderArea; };

        //
        virtual tType registerSelf() {
            ANAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
            return SFT();
        };

        //
        inline static tType make(Handle const& handle, cpp21::optional_ref<PingPongCreateInfo> cInfo = PingPongCreateInfo{}) {
            auto shared = std::make_shared<PingPongObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

        //
        virtual uintptr_t getNextIndex() const {
            return ((this->currentState.index + 1) % this->sets.size());
        };

        //
        virtual uintptr_t getCurrentIndex() const {
            return (this->currentState.index % this->sets.size());
        };

        //
        virtual std::vector<ExtHandle> getReadySemaphoreExtHandles() {
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            std::vector<ExtHandle> handles = {};
            for (uintptr_t i = 0; i < this->sets.size(); i++) {
                decltype(auto) semaphoreObj = deviceObj->get<SemaphoreObj>(sets[i].readySemaphoreInfo->semaphore);
                handles.push_back(semaphoreObj->getExtHandle());
            };
            return handles;
        };

        //
        virtual std::vector<ExtHandle> getPresentSemaphoreExtHandles() {
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            std::vector<ExtHandle> handles = {};
            for (uintptr_t i = 0; i < this->sets.size(); i++) {
                decltype(auto) semaphoreObj = deviceObj->get<SemaphoreObj>(sets[i].presentSemaphoreInfo->semaphore);
                handles.push_back(semaphoreObj->getExtHandle());
            };
            return handles;
        };

        //
        virtual FenceType switchToPresent(uint32_t const& imageIndex, cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}, bool const& glSemaphore = false) {
            decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info.value() } };
            decltype(auto) nextIndex = (imageIndex + 1u) % this->sets.size();

            // 
            submission.submission.signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
            submission.submission.waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};

            //
            if (glSemaphore) {
                submission.submission.signalSemaphores->push_back(*sets[imageIndex].readySemaphoreInfo); // ready for OpenGL draw
            };

            //
            submission.submission.signalSemaphores->push_back(*sets[nextIndex].copySemaphoreInfo);

            //
            submission.commandInits.push_back([this, imageIndex](vk::CommandBuffer const& cmdBuf) {
                for (auto& switchFn : this->sets[imageIndex].switchToPresentFns) { switchFn(cmdBuf); };
                return cmdBuf;
                });

            //
            firstWait = false;

            //
            return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
        };

        //
        virtual FenceType switchToReady(uint32_t const& imageIndex, cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}, bool const& glSemaphore = false) {
            decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info.value() } };

            // 
            submission.submission.signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
            submission.submission.waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};

            //
            if (glSemaphore) {
                submission.submission.waitSemaphores->push_back(*sets[imageIndex].presentSemaphoreInfo); // waiting OpenGL present
            };

            // 
            if (!firstWait) {
                submission.submission.waitSemaphores->push_back(*sets[imageIndex].copySemaphoreInfo);
            };

            // but currently there is no any commands...
            submission.commandInits.push_back([this, imageIndex](vk::CommandBuffer const& cmdBuf) {
                for (auto& switchFn : this->sets[imageIndex].switchToReadyFns) { switchFn(cmdBuf); };
                return cmdBuf;
                });

            //
            return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
        };

        //
        virtual uint32_t& acquireImage(cpp21::optional_ref<ANAMED::QueueGetInfo> qfAndQueue) {
            this->currentState.previous = this->currentState.index;
            this->currentState.index = (++this->currentState.index) % this->sets.size();
            decltype(auto) fence = this->switchToReady(this->currentState.index, qfAndQueue); // still needs for await semaphores
            memcpy(this->currentState.images[1u], this->sets[this->currentState.previous].imageViewIndices.data(), std::min(uint32_t(this->sets[this->currentState.previous].imageViewIndices.size()), 6u) * 4u);
            memcpy(this->currentState.images[0u], this->sets[this->currentState.index].imageViewIndices.data(), std::min(uint32_t(this->sets[this->currentState.index].imageViewIndices.size()), 6u) * 4u);
            return this->currentState.index;
        };

        //
        virtual FenceType presentImage(cpp21::optional_ref<ANAMED::QueueGetInfo> qfAndQueue) {
            return this->switchToPresent(this->currentState.index, qfAndQueue);
        };

        //
        virtual FenceType clearImages(cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}, std::vector<glm::vec4> const& clearColors = {}) {
            decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info.value() } };

            // 
            //submission.submission.waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{ sets[*imageIndex].presentSemaphoreInfo };
            submission.commandInits.push_back([this, clearColors, queueFamilyIndex = info->queueFamilyIndex](vk::CommandBuffer const& cmdBuf) {
                this->writeClearImages(cmdBuf, queueFamilyIndex, clearColors);
                return cmdBuf;
            });

            //
            return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
        };

        //
        virtual FenceType clearImage(cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}, uint32_t const& imageIndex = 0u, glm::vec4 const& clearColors = {}) {
            decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info.value() } };

            // 
            //submission.submission.waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{ sets[*imageIndex].presentSemaphoreInfo };
            submission.commandInits.push_back([this, clearColors, imageIndex, queueFamilyIndex = info->queueFamilyIndex](vk::CommandBuffer const& cmdBuf) {
                this->writeClearImage(cmdBuf, queueFamilyIndex, imageIndex, clearColors);
                return cmdBuf;
            });

            //
            return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
        };

        //
        virtual void writeClearImage(vk::CommandBuffer const& cmdBuf, uint32_t const& queueFamilyIndex = 0u, uint32_t const& imageIndex = 0u, glm::vec4 const& clearColors = {}) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) image = this->sets[this->currentState.index].images[imageIndex];
            decltype(auto) imageObj = deviceObj->get<ResourceImageObj>(image);
            imageObj->writeClearCommand(ImageClearWriteInfo{
              .cmdBuf = cmdBuf,
              .clearColor = clearColors,
              .queueFamilyIndex = queueFamilyIndex
                });
        };

        //
        virtual void writeClearImages(vk::CommandBuffer const& cmdBuf, uint32_t const& queueFamilyIndex = 0u, std::vector<glm::vec4> const& clearColors = {}) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

            uint32_t setIndex = this->currentState.index;
            uint32_t J = 0u; for (auto& image : this->sets[setIndex].images) {
                uint32_t j = J++;
                decltype(auto) imageObj = deviceObj->get<ResourceImageObj>(image);
                imageObj->writeClearCommand(ImageClearWriteInfo{
                  .cmdBuf = cmdBuf,
                  .clearColor = clearColors[j],
                  .queueFamilyIndex = queueFamilyIndex
                    });
            };
        };

        //
        virtual void updateSwapchain() {
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) descriptorsObj = deviceObj->get<PipelineLayoutObj>(this->cInfo->layout);
            auto& device = this->base.as<vk::Device>();

            { //
                this->firstWait = true;
                decltype(auto) it = sets.begin();
                for (it = sets.begin(); it != sets.end();) {
                    it->imageViews = {};
                    it->imageViewIndices = {};

                    // 
                    deviceObj->get<SemaphoreObj>(it->readySemaphoreInfo->semaphore)->destroy(deviceObj.get());
                    deviceObj->get<SemaphoreObj>(it->presentSemaphoreInfo->semaphore)->destroy(deviceObj.get());
                    deviceObj->get<SemaphoreObj>(it->copySemaphoreInfo->semaphore)->destroy(deviceObj.get());

                    // 
                    decltype(auto) it2 = it->images.begin();
                    for (it2 = it->images.begin(); it2 != it->images.end();) {
                        deviceObj->get<ResourceImageObj>(*it2)->destroy(deviceObj.get());
                        it2 = it->images.erase(it2);
                    };

                    // 
                    it = sets.erase(it);
                };
            };

            //
            for (uint32_t i = 0; i < this->cInfo->minImageCount; i++) {
                this->sets.push_back(this->createSet(i));
            };

            // 
            uint32_t imageIndex = 0u;
            for (uint32_t i = 0; i < this->cInfo->minImageCount; i++) {
                uint32_t J = 0u; for (decltype(auto) image : this->cInfo->formats) {
                    uint32_t j = J++;
                    this->createImage(&this->sets[i], j, i, ImageType::eStorage); // 
                };
            };

            // 
            descriptorsObj->updateDescriptors();

            //
            this->currentState.index = this->sets.size() - 1u;
            if (this->currentState.previous != -1) {
                memcpy(this->currentState.images[1], this->sets[this->currentState.previous].imageViewIndices.data(), std::min(uint32_t(this->sets[this->currentState.previous].imageViewIndices.size()), 6u) * 4u);
            };
            memcpy(this->currentState.images[0], this->sets[this->currentState.index].imageViewIndices.data(), std::min(uint32_t(this->sets[this->currentState.index].imageViewIndices.size()), 6u) * 4u);

            //
            this->currentState.extent = glm::uvec2(this->cInfo->extent.width, this->cInfo->extent.height);
        };

    protected:

        //
        virtual void createSet(SwapchainSet* set, uint32_t index = 0u) {
            //
            vk::SemaphoreTypeCreateInfo timeline = {};
            timeline.semaphoreType = vk::SemaphoreType::eBinary;
            timeline.initialValue = 0ull;

            // incompatible with export
            decltype(auto) readySemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{});
            decltype(auto) presentSemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{});
            decltype(auto) copySemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{.signaled = true, .hasExport = false });

            //
            set->readySemaphoreInfo = readySemaphore->infoMap->get<vk::SemaphoreSubmitInfo>(vk::StructureType::eSemaphoreSubmitInfo);
            set->presentSemaphoreInfo = presentSemaphore->infoMap->get<vk::SemaphoreSubmitInfo>(vk::StructureType::eSemaphoreSubmitInfo);
            set->copySemaphoreInfo = copySemaphore->infoMap->get<vk::SemaphoreSubmitInfo>(vk::StructureType::eSemaphoreSubmitInfo);
        };

        //
        virtual SwapchainSet createSet(uint32_t index = 0u) {
            SwapchainSet set = {};
            this->createSet(&set, index);
            return set;
        };

        //
        virtual void createImage(SwapchainSet* set, uint32_t index, uint32_t setIndex, cpp21::optional_ref<ImageType> imageType = ImageType::eStorage) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) descriptorsObj = deviceObj->get<PipelineLayoutObj>(this->cInfo->layout);
            decltype(auto) imageLayout = vk::ImageLayout::eGeneral;

            //
            decltype(auto) extent3D = vk::Extent3D{ this->cInfo->extent.width * this->cInfo->split[index], this->cInfo->extent.height, 1u };
            decltype(auto) imageObj = ResourceImageObj::make(this->base, ImageCreateInfo{
                .descriptors = this->cInfo->layout,
                .format = this->cInfo->formats[index],
                .extent = extent3D,
                .layout = imageLayout,
                .info = this->cInfo->info ? this->cInfo->info.value() : QueueGetInfo{0u, 0u},
                .type = imageType
            });

            //
            decltype(auto) subresourceRange = imageObj->subresourceRange();
            decltype(auto) subresourceLayers =
                vk::ImageSubresourceLayers{
                  .aspectMask = subresourceRange.aspectMask,
                  .mipLevel = subresourceRange.baseMipLevel,
                  .baseArrayLayer = subresourceRange.baseArrayLayer,
                  .layerCount = subresourceRange.layerCount
            };

            // 
            decltype(auto) pair = imageObj->createImageView(ImageViewCreateInfo{
              .viewType = vk::ImageViewType::e2D,
              .subresourceRange = subresourceRange,
              .preference = ImageViewPreference::eStorage
                });

            //
            //intptr_t prevSetIndex = intptr_t(setIndex)-1;
            //if (prevSetIndex < 0) { prevSetIndex += this->sets.size(); };

            //
            decltype(auto) nextSetIndex = (setIndex + 1u) % uint32_t(this->sets.size());

            //
            set->images.push_back(imageObj.as<vk::Image>());
            set->imageViews.push_back(pair.imageView);
            set->imageViewIndices.push_back(pair.indice);

            // currently there is no any command
            set->switchToReadyFns.push_back([](vk::CommandBuffer const& cmdBuf) {
                });

            //
            std::vector<vk::ImageCopy2> copyRegions = { vk::ImageCopy2{
              .srcSubresource = subresourceLayers,
              .srcOffset = vk::Offset3D{0u,0u,0u},
              .dstSubresource = subresourceLayers,
              .dstOffset = vk::Offset3D{0u,0u,0u},
              .extent = extent3D
            } };

            // copy into next image in chain (i.e. use ping-pong alike Optifine)
            set->switchToPresentFns.push_back([this, device, subresourceRange, copyRegions, index, nextSetIndex, image = imageObj.as<vk::Image>()](vk::CommandBuffer const& cmdBuf) {
                // use next index of
                decltype(auto) nextImage = this->sets[nextSetIndex].images[index];
                if (nextImage != image) {
                    decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(device);
                    decltype(auto) imageObj = deviceObj->get<ResourceImageObj>(image);
                    decltype(auto) nextImageObj = deviceObj->get<ResourceImageObj>(nextImage);
                    decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
                    decltype(auto) accessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(imageObj->getImageUsage()));

                    // 
                    std::vector<vk::ImageMemoryBarrier2> imageBarriersBegin = {};
                    std::vector<vk::ImageMemoryBarrier2> imageBarriersEnd = {};

                    //
                    imageBarriersBegin.push_back(vk::ImageMemoryBarrier2{
                      .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                      .srcAccessMask = vk::AccessFlagBits2(accessMask),
                      .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
                      .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
                      .oldLayout = imageObj->cInfo->layout,
                      .newLayout = vk::ImageLayout::eTransferSrcOptimal,
                      .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                      .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                      .image = image,
                      .subresourceRange = subresourceRange
                        });
                    imageBarriersBegin.push_back(vk::ImageMemoryBarrier2{
                      .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                      .srcAccessMask = vk::AccessFlagBits2(accessMask),
                      .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                      .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
                      .oldLayout = nextImageObj->cInfo->layout,
                      .newLayout = vk::ImageLayout::eTransferDstOptimal,
                      .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                      .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                      .image = nextImage,
                      .subresourceRange = subresourceRange
                        });

                    //
                    imageBarriersEnd.push_back(vk::ImageMemoryBarrier2{
                      .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
                      .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
                      .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                      .dstAccessMask = vk::AccessFlagBits2(accessMask),
                      .oldLayout = vk::ImageLayout::eTransferSrcOptimal,
                      .newLayout = imageObj->cInfo->layout,
                      .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                      .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                      .image = image,
                      .subresourceRange = subresourceRange
                        });
                    imageBarriersEnd.push_back(vk::ImageMemoryBarrier2{
                      .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                      .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
                      .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(accessMask) | (accessMask & vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite) ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
                      .dstAccessMask = vk::AccessFlagBits2(accessMask),
                      .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                      .newLayout = nextImageObj->cInfo->layout,
                      .srcQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                      .dstQueueFamilyIndex = this->cInfo->info->queueFamilyIndex,
                      .image = nextImage,
                      .subresourceRange = subresourceRange
                        });

                    // 
                    decltype(auto) copyImageInfo = vk::CopyImageInfo2{
                      .srcImage = image,
                      .srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
                      .dstImage = nextImage,
                      .dstImageLayout = vk::ImageLayout::eTransferDstOptimal
                    };

                    // 
                    cmdBuf.pipelineBarrier2(depInfo.setImageMemoryBarriers(imageBarriersBegin));
                    cmdBuf.copyImage2(copyImageInfo.setRegions(copyRegions));
                    cmdBuf.pipelineBarrier2(depInfo.setImageMemoryBarriers(imageBarriersEnd));
                };
            });
        };

        // 
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<PingPongCreateInfo> cInfo = PingPongCreateInfo{}) {
            if (cInfo) { this->cInfo = cInfo; };
            this->handle = uintptr_t(this);
            this->updateSwapchain();
        };

    public:



    };

};
#endif
