#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"
#include "./PipelineLayout.hpp"
#include "./Semaphore.hpp"

// 
namespace ANAMED {
  
  //
  struct SurfaceCapabilitiesInfo {
    vk::SurfaceCapabilities2KHR capabilities2 = {};
    std::vector<vk::SurfaceFormat2KHR> formats2 = {};
    std::vector<vk::PresentModeKHR> presentModes = {};
    //vk::PhysicalDeviceSurfaceInfo2KHR info2 = {};
    cpp21::carg<vk::SurfaceCapabilitiesKHR> capabilities = {};
    //cpp21::optional_ref<vk::SurfaceFormatKHR> formats = {};
  };

  // 
  class SwapchainObj : public BaseObj {
  public: 
    using tType = WrapShared<SwapchainObj>;
    using BaseObj::BaseObj;
    //using BaseObj;

  protected: 
    // 
    friend DeviceObj;
    friend PipelineObj;
    friend ResourceObj;

    //
    std::optional<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{};

    //
    std::vector<vk::Image> images = {};
    std::vector<vk::ImageView> imageViews = {};
    std::vector<uint32_t> imageViewIndices = {};

    //
    std::vector<vk::Semaphore> readySemaphores = {};
    std::vector<vk::Semaphore> presentSemaphores = {};

    //
    std::vector<vk::SemaphoreSubmitInfo> readySemaphoreInfos = {};
    std::vector<vk::SemaphoreSubmitInfo> presentSemaphoreInfos = {};

    //
    std::vector<std::function<void(cpp21::carg<vk::CommandBuffer>)>> switchToPresentFn = {};
    std::vector<std::function<void(cpp21::carg<vk::CommandBuffer>)>> switchToReadyFn = {};

    //
    vk::Rect2D renderArea = {};
    vk::SwapchainKHR swapchain = {};

    //
    SurfaceCapabilitiesInfo capInfo = {};
    SwapchainStateInfo currentState = {};
    //SwapchainState state = SwapchainState::eReady;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };


  public:
    // 
    SwapchainObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::carg<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    SwapchainObj(cpp21::carg<Handle> handle, cpp21::carg<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    //
    virtual SwapchainStateInfo& getStateInfo() { return currentState; };
    virtual SwapchainStateInfo const& getStateInfo() const { return currentState; };

    //
    virtual std::vector<uint32_t> const& getImageViewIndices() const { return imageViewIndices; };
    virtual std::vector<vk::Semaphore> const& getWaitSemaphores() const { return readySemaphores; };
    virtual std::vector<vk::Semaphore> const& getPresentSemaphores() const { return presentSemaphores; };
    virtual std::vector<vk::SemaphoreSubmitInfo> const& getReadySemaphoreInfos() const { return readySemaphoreInfos; };
    virtual std::vector<vk::SemaphoreSubmitInfo> const& getPresentSemaphoreInfos() const { return presentSemaphoreInfos; };
    virtual vk::Rect2D const& getRenderArea() const { return renderArea; };

    //
    virtual tType registerSelf() {
      ANAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(cpp21::carg<Handle> handle, cpp21::carg<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) {
      auto shared = std::make_shared<SwapchainObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual FenceType switchToPresent(cpp21::carg<uint32_t> imageIndex, cpp21::carg<QueueGetInfo> info = QueueGetInfo{}) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = info ? info.value() : this->cInfo->info.ref() }};

      // 
      submission.submission.signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{ readySemaphoreInfos[*imageIndex] };
      submission.commandInits.push_back([this, imageIndex](cpp21::carg<vk::CommandBuffer> cmdBuf) {
        this->switchToPresentFn[*imageIndex](cmdBuf);
        return cmdBuf;
      });

      //
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType switchToReady(cpp21::carg<uint32_t> imageIndex, cpp21::carg<QueueGetInfo> info = QueueGetInfo{}) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = info ? info.value() : this->cInfo->info.ref() } };

      // 
      submission.submission.waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{ presentSemaphoreInfos[*imageIndex] };
      submission.commandInits.push_back([this, imageIndex](cpp21::carg<vk::CommandBuffer> cmdBuf) {
        this->switchToReadyFn[*imageIndex](cmdBuf);
        return cmdBuf;
      });

      //
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual uint32_t& acquireImage(cpp21::carg<ANAMED::QueueGetInfo> qfAndQueue) {
      //this->currentState.previous = this->currentState.index;
      decltype(auto) semIndex = (this->currentState.index + 1u) % this->imageViewIndices.size();
      decltype(auto) acquired = (this->currentState.index = handleResult(this->base.as<vk::Device>().acquireNextImage2KHR(vk::AcquireNextImageInfoKHR{ .swapchain = this->swapchain, .timeout = 0, .semaphore = this->presentSemaphoreInfos[semIndex].semaphore, .deviceMask = 0x1u })));

      //
      this->switchToReady(acquired, qfAndQueue);
      this->currentState.image = this->imageViewIndices[acquired];
      return this->currentState.index;
    };

    //
    virtual std::tuple<FenceType, vk::Result> presentImage(cpp21::carg<ANAMED::QueueGetInfo> qfAndQueue) {
      decltype(auto) fence = this->switchToPresent(this->currentState.index, qfAndQueue);
      decltype(auto) result = ANAMED::context->get<DeviceObj>(this->base)->getQueue(qfAndQueue).presentKHR(vk::PresentInfoKHR{
        .waitSemaphoreCount = 1u,
        .pWaitSemaphores = &this->readySemaphoreInfos[this->currentState.index].semaphore,
        .swapchainCount = 1u,
        .pSwapchains = &this->swapchain,
        .pImageIndices = &this->currentState.index
        });
      return std::make_tuple(fence, result);
    };

    //
    virtual vk::SwapchainKHR& getTrueSwapchain() { return this->swapchain; };
    virtual vk::SwapchainKHR const& getTrueSwapchain() const { return this->swapchain; };

    //
    virtual vk::SwapchainKHR& updateSwapchain() {
      //decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<PipelineLayoutObj>(this->cInfo->layout);
      auto& device = this->base.as<vk::Device>();

      //
      auto& physicalDevice = deviceObj->getPhysicalDevice();
      auto PDInfoMap = deviceObj->getPhysicalDeviceInfoMap();
      capInfo.capabilities2 = physicalDevice.getSurfaceCapabilities2KHR(vk::PhysicalDeviceSurfaceInfo2KHR{ .surface = cInfo->surface });
      capInfo.capabilities = capInfo.capabilities2.surfaceCapabilities;
      capInfo.formats2 = physicalDevice.getSurfaceFormats2KHR(vk::PhysicalDeviceSurfaceInfo2KHR{ .surface = cInfo->surface });
      capInfo.presentModes = physicalDevice.getSurfacePresentModesKHR(cInfo->surface);

      // TODO: search needed surface format
      decltype(auto) surfaceFormat2 = capInfo.formats2.back();
      decltype(auto) presentMode = capInfo.presentModes.front();

      //
      this->imageViews = {};
      this->imageViewIndices = {};
      this->switchToReadyFn = {};
      this->switchToPresentFn = {};
      this->readySemaphoreInfos = {};
      this->presentSemaphoreInfos = {};
      this->renderArea = vk::Rect2D{ vk::Offset2D{0u, 0u}, capInfo.capabilities->currentExtent };

      { //
        decltype(auto) it = images.begin();
        for (it = images.begin(); it != images.end();) {
          deviceObj->get<ResourceObj>(*it)->destroy(deviceObj.get());
          it = images.erase(it);
        };
      };

      { //
        decltype(auto) it = readySemaphores.begin();
        for (it = readySemaphores.begin(); it != readySemaphores.end();) {
          deviceObj->get<SemaphoreObj>(*it)->destroy(deviceObj.get());
          it = readySemaphores.erase(it);
        };
      };

      { //
        decltype(auto) it = presentSemaphores.begin();
        for (it = presentSemaphores.begin(); it != presentSemaphores.end();) {
          deviceObj->get<SemaphoreObj>(*it)->destroy(deviceObj.get());
          it = presentSemaphores.erase(it);
        };
      };

      //
      images = device.getSwapchainImagesKHR(this->swapchain = device.createSwapchainKHR(infoMap->set(vk::StructureType::eSwapchainCreateInfoKHR, vk::SwapchainCreateInfoKHR{
        .surface = cInfo->surface,
        .minImageCount = std::max(capInfo.capabilities->minImageCount, capInfo.capabilities->maxImageCount),
        .imageFormat = surfaceFormat2.surfaceFormat.format,
        .imageColorSpace = surfaceFormat2.surfaceFormat.colorSpace,
        .imageExtent = capInfo.capabilities->currentExtent,
        .imageArrayLayers = std::min(capInfo.capabilities->maxImageArrayLayers, 1u),
        .imageUsage = capInfo.capabilities->supportedUsageFlags,
        .preTransform = capInfo.capabilities->currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = true
      })->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices)));

      // 
      uint32_t imageIndex = 0u;
      for (decltype(auto) image : images) {
        this->createImage(image, ImageType::eSwapchain, surfaceFormat2, ImageSwapchainInfo{
          .swapchain = this->swapchain,
          .index = imageIndex++
        }); // 
      };

      // 
      descriptorsObj->updateDescriptors();

      //
      this->currentState.extent = glm::uvec2(capInfo.capabilities->currentExtent.width, capInfo.capabilities->currentExtent.height);
      this->currentState.index = this->images.size() - 1u;
      this->currentState.image = this->imageViewIndices[this->currentState.index];
      return this->swapchain;
    };

  protected:

    //
    virtual void createImage(cpp21::carg<vk::Image> image, cpp21::carg<ImageType> imageType = ImageType::eSwapchain, cpp21::carg<vk::SurfaceFormat2KHR> surfaceFormat2 = {}, cpp21::carg<ImageSwapchainInfo> swapchainInfo = {}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<PipelineLayoutObj>(this->cInfo->layout);
      decltype(auto) imageLayout = vk::ImageLayout::eGeneral;
      decltype(auto) format = surfaceFormat2->surfaceFormat.format;

      //
      decltype(auto) imageObj = ResourceObj::make(this->base, ResourceCreateInfo{
        .image = image,
        .imageInfo = ImageCreateInfo{
          .format = format,
          .extent = vk::Extent3D{ capInfo.capabilities->currentExtent.width, capInfo.capabilities->currentExtent.height, 1u },
          .layout = imageLayout,
          // # first reason of internal error (if use implicit cast, mostly with std::optional)
          .swapchain = std::optional<ImageSwapchainInfo>(swapchainInfo),
          .info = this->cInfo->info ? this->cInfo->info.ref() : QueueGetInfo{0u, 0u},
          .type = imageType
        }
      });

      // 
      decltype(auto) subresourceRange = imageObj->subresourceRange();

      // 
      decltype(auto) pair = imageObj->createImageView(ImageViewCreateInfo{
        .viewType = vk::ImageViewType::e2D,
        .subresourceRange = subresourceRange,
        .preference = ImageViewPreference::eStorage
      });
      this->imageViews.push_back(pair.imageView);
      this->imageViewIndices.push_back(pair.indice);

      // TODO: use pre-built command buffer
      this->switchToReadyFn.push_back([device, imageLayout, subresourceRange, image=imageObj.as<vk::Image>()](cpp21::carg<vk::CommandBuffer> cmdBuf) {
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(device);
        decltype(auto) imageObj = deviceObj->get<ResourceObj>(image);
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = imageLayout,
          .subresourceRange = subresourceRange,
        });
      });

      //
      this->switchToPresentFn.push_back([device, imageLayout, subresourceRange, image=imageObj.as<vk::Image>()](cpp21::carg<vk::CommandBuffer> cmdBuf) {
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(device);
        decltype(auto) imageObj = deviceObj->get<ResourceObj>(image);
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = vk::ImageLayout::ePresentSrcKHR,
          .subresourceRange = subresourceRange,
        });
      });

      //
      vk::SemaphoreTypeCreateInfo timeline = {};
      timeline.semaphoreType = vk::SemaphoreType::eBinary;
      timeline.initialValue = 0ull;//this->readySemaphores.size();

      // incompatible with export
      decltype(auto) readySemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{ .hasExport = false });
      decltype(auto) presentSemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{ .hasExport = false });

      //
      this->readySemaphores.push_back(readySemaphore.as<vk::Semaphore>());
      this->presentSemaphores.push_back(presentSemaphore.as<vk::Semaphore>());

      //
      this->readySemaphoreInfos.push_back(readySemaphore->infoMap->get<vk::SemaphoreSubmitInfo>(vk::StructureType::eSemaphoreSubmitInfo));
      this->presentSemaphoreInfos.push_back(presentSemaphore->infoMap->get<vk::SemaphoreSubmitInfo>(vk::StructureType::eSemaphoreSubmitInfo));
      //ANAMED::context->get<DeviceObj>(this->base)
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::carg<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      //this->handle = Handle(uintptr_t(this), HandleType::eSwapchain);
      this->handle = uintptr_t(this);
      this->updateSwapchain();
    };

  public:


    
  };

};
#endif
