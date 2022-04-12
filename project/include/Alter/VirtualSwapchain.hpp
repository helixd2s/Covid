#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"
#include "./Descriptors.hpp"
#include "./Semaphore.hpp"

// 
namespace ANAMED {
  
  //
  //struct SurfaceCapabilitiesInfo {
    //vk::SurfaceCapabilities2KHR capabilities2 = {};
    //std::vector<vk::SurfaceFormat2KHR> formats2 = {};
    //std::vector<vk::PresentModeKHR> presentModes = {};
    //vk::PhysicalDeviceSurfaceInfo2KHR info2 = {};
    //cpp21::const_wrap_arg<vk::SurfaceCapabilitiesKHR> capabilities = {};
    //cpp21::optional_ref<vk::SurfaceFormatKHR> formats = {};
  //};

  //
  struct SwapchainSet {
    std::vector<vk::Image> images = {};
    std::vector<vk::ImageView> imageViews = {};
    std::vector<uint32_t> imageViewIndices = {};
    std::vector<std::function<void(cpp21::const_wrap_arg<vk::CommandBuffer>)>> switchToPresentFns = {};
    std::vector<std::function<void(cpp21::const_wrap_arg<vk::CommandBuffer>)>> switchToReadyFns = {};

    // 
    cpp21::wrap_shared_ptr<vk::SemaphoreSubmitInfo> copySemaphoreInfo = {};
    cpp21::wrap_shared_ptr<vk::SemaphoreSubmitInfo> readySemaphoreInfo = {};
    cpp21::wrap_shared_ptr<vk::SemaphoreSubmitInfo> presentSemaphoreInfo = {};
  };

  // 
  class VirtualSwapchainObj : public BaseObj {
  public: 
    using tType = WrapShared<VirtualSwapchainObj>;
    using BaseObj::BaseObj;
    //using BaseObj;

  protected: 
    // 
    friend DeviceObj;
    friend PipelineObj;
    friend ResourceObj;
    bool firstWait = true;

    //
    std::optional<VirtualSwapchainCreateInfo> cInfo = VirtualSwapchainCreateInfo{};

    //
    std::vector<SwapchainSet> sets = {};

    //
    VirtualSwapchainStateInfo currentState = {};

    //
    vk::Rect2D renderArea = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:
    // 
    VirtualSwapchainObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<VirtualSwapchainCreateInfo> cInfo = VirtualSwapchainCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    VirtualSwapchainObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<VirtualSwapchainCreateInfo> cInfo = VirtualSwapchainCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    //
    virtual VirtualSwapchainStateInfo& getStateInfo() { return currentState; };
    virtual VirtualSwapchainStateInfo const& getStateInfo() const { return currentState; };
    virtual vk::Rect2D const& getRenderArea() const { return renderArea; };

    //
    virtual tType registerSelf() {
      ANAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<VirtualSwapchainCreateInfo> cInfo = VirtualSwapchainCreateInfo{}) {
      auto shared = std::make_shared<VirtualSwapchainObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual FenceType switchToPresent(cpp21::const_wrap_arg<uint32_t> imageIndex, cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = info ? info.value() : this->cInfo->info } };
      decltype(auto) nextIndex = ((*imageIndex) + 1u) % this->sets.size();

      // 
      //submission.submission.signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{ sets[*imageIndex].readySemaphoreInfo };
      submission.submission.signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{ sets[nextIndex].copySemaphoreInfo };
      submission.commandInits.push_back([this, imageIndex](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        for (auto& switchFn : this->sets[*imageIndex].switchToPresentFns) { switchFn(cmdBuf); };
        return cmdBuf;
      });

      //
      firstWait = false;

      //
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType switchToReady(cpp21::const_wrap_arg<uint32_t> imageIndex, cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = info ? info.value() : this->cInfo->info } };

      // 
      submission.submission.waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{};
      //submission.submission.waitSemaphores.push_back(sets[*imageIndex].presentSemaphoreInfo);

      // 
      if (!firstWait) {
        submission.submission.waitSemaphores->push_back(sets[*imageIndex].copySemaphoreInfo);
      };

      // 
      submission.commandInits.push_back([this, imageIndex](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        for (auto& switchFn : this->sets[*imageIndex].switchToReadyFns) { switchFn(cmdBuf); };
        return cmdBuf;
      });

      //
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual uint32_t& acquireImage(cpp21::const_wrap_arg<ANAMED::QueueGetInfo> qfAndQueue) {
      this->currentState.previous = this->currentState.index;
      this->currentState.index = (++this->currentState.index) % this->sets.size();
      decltype(auto) fence = this->switchToReady(this->currentState.index, qfAndQueue);
      memcpy(this->currentState.images, this->sets[this->currentState.index].imageViewIndices.data(), std::min(uint32_t(this->sets[this->currentState.index].imageViewIndices.size()), 4u)*4u);
      return this->currentState.index;
    };

    //
    virtual FenceType presentImage(cpp21::const_wrap_arg<ANAMED::QueueGetInfo> qfAndQueue) {
      return this->switchToPresent(this->currentState.index, qfAndQueue);
    };

    //
    virtual FenceType clearImages(cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}, std::vector<glm::vec4> const& clearColors = {}) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo {.info = info ? info.value() : this->cInfo->info } };

      // 
      //submission.submission.waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{ sets[*imageIndex].presentSemaphoreInfo };
      submission.commandInits.push_back([this, clearColors, queueFamilyIndex= info->queueFamilyIndex](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        //for (auto& switchFn : this->sets[*imageIndex].switchToReadyFns) { switchFn(cmdBuf); };
        this->writeClearImages(cmdBuf, clearColors, queueFamilyIndex);
        return cmdBuf;
      });

      //
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual void writeClearImages(cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf, std::vector<glm::vec4> const& clearColors, uint32_t const& queueFamilyIndex = 0u) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

      uint32_t setIndex = this->currentState.index;
      uint32_t J=0u; for (auto& image : this->sets[setIndex].images) { uint32_t j = J++;
        decltype(auto) imageObj = deviceObj->get<ResourceObj>(image);
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
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
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
            deviceObj->get<ResourceObj>(*it2)->destroy(deviceObj.get());
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
      memcpy(this->currentState.images, this->sets[this->currentState.index].imageViewIndices.data(), std::min(uint32_t(this->sets[this->currentState.index].imageViewIndices.size()), 4u) * 4u);
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
      decltype(auto) copySemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{ .signaled = true, .hasExport = false });

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
    virtual void createImage(SwapchainSet* set, uint32_t index, uint32_t setIndex, cpp21::const_wrap_arg<ImageType> imageType = ImageType::eStorage) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) imageLayout = vk::ImageLayout::eGeneral;

      //
      decltype(auto) extent3D = vk::Extent3D{ this->cInfo->extent.width, this->cInfo->extent.height, 1u };
      decltype(auto) imageObj = ResourceObj::make(this->base, ResourceCreateInfo{
        .descriptors = this->cInfo->layout,
        .imageInfo = ImageCreateInfo{
          .format = this->cInfo->formats[index],
          .extent = extent3D,
          .layout = imageLayout,
          .info = this->cInfo->info ? this->cInfo->info : QueueGetInfo{0u, 0u},
          .type = imageType
        }
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
      decltype(auto) nextSetIndex = (setIndex+1u) % uint32_t(this->sets.size());

      //
      set->imageViews.push_back(std::get<0>(pair));
      set->imageViewIndices.push_back(std::get<1>(pair));

      // TODO: use pre-built command buffer
      set->switchToReadyFns.push_back([](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
      });

      //
      std::vector<vk::ImageCopy2> copyRegions = {vk::ImageCopy2{
        .srcSubresource = subresourceLayers,
        .srcOffset = vk::Offset3D{0u,0u,0u},
        .dstSubresource = subresourceLayers,
        .dstOffset = vk::Offset3D{0u,0u,0u},
        .extent = extent3D
      }};

      // copy into next image in chain (i.e. use ping-pong alike Optifine)
      set->switchToPresentFns.push_back([this,device,subresourceRange,copyRegions,index,nextSetIndex,image=imageObj.as<vk::Image>()](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        // use next index of
        decltype(auto) nextImage = this->sets[nextSetIndex].images[index];
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(device);
        decltype(auto) imageObj = deviceObj->get<ResourceObj>(image);
        decltype(auto) nextImageObj = deviceObj->get<ResourceObj>(nextImage);
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
          .oldLayout = imageObj->cInfo->imageInfo->layout,
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
          .oldLayout = nextImageObj->cInfo->imageInfo->layout,
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
          .newLayout = imageObj->cInfo->imageInfo->layout,
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
          .newLayout = nextImageObj->cInfo->imageInfo->layout,
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
        cmdBuf->pipelineBarrier2(depInfo.setImageMemoryBarriers(imageBarriersBegin));
        cmdBuf->copyImage2(copyImageInfo.setRegions(copyRegions));
        cmdBuf->pipelineBarrier2(depInfo.setImageMemoryBarriers(imageBarriersEnd));
      });
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<VirtualSwapchainCreateInfo> cInfo = VirtualSwapchainCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      this->handle = uintptr_t(this);
      this->updateSwapchain();
    };

  public:


    
  };

};
#endif
