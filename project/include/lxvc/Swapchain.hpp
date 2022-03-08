#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"
#include "./Descriptors.hpp"
#include "./Semaphore.hpp"

// 
namespace lxvc {
  
  //
  struct SurfaceCapabilitiesInfo {
    vk::SurfaceCapabilities2KHR capabilities2 = {};
    std::vector<vk::SurfaceFormat2KHR> formats2 = {};
    std::vector<vk::PresentModeKHR> presentModes = {};
    //vk::PhysicalDeviceSurfaceInfo2KHR info2 = {};
    cpp21::const_wrap_arg<vk::SurfaceCapabilitiesKHR> capabilities = {};
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
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

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
    std::vector<std::function<void(cpp21::const_wrap_arg<vk::CommandBuffer>, cpp21::const_wrap_arg<SwapchainState>)>> switchToPresentFn = {};
    std::vector<std::function<void(cpp21::const_wrap_arg<vk::CommandBuffer>, cpp21::const_wrap_arg<SwapchainState>)>> switchToReadyFn = {};

    //
    SwapchainState state = SwapchainState::eReady;
    SurfaceCapabilitiesInfo capInfo = {};

    //
    vk::Rect2D renderArea = {};

  public:
    // 
    SwapchainObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    SwapchainObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) : cInfo(cInfo) {
      this->construct(lxvc::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    //
    virtual std::vector<uint32_t> const& getImageViewIndices() const { return imageViewIndices; };
    virtual std::vector<vk::Semaphore> const& getWaitSemaphores() const { return readySemaphores; };
    virtual std::vector<vk::Semaphore> const& getPresentSemaphores() const { return presentSemaphores; };
    virtual std::vector<vk::SemaphoreSubmitInfo> const& getReadySemaphoreInfos() const { return readySemaphoreInfos; };
    virtual std::vector<vk::SemaphoreSubmitInfo> const& getPresentSemaphoreInfos() const { return presentSemaphoreInfos; };
    virtual vk::Rect2D const& getRenderArea() const { return renderArea; };

    //
    virtual tType registerSelf() {
      lxvc::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) {
      auto shared = std::make_shared<SwapchainObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual FenceType switchToPresent(cpp21::const_wrap_arg<uint32_t> imageIndex, cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = info ? info.value() : this->cInfo->info } };

      // 
      submission.commandInits.push_back([=, this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        switchToPresentFn[*imageIndex](cmdBuf, this->state);
        return cmdBuf;
      });

      //
      return lxvc::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType switchToReady(cpp21::const_wrap_arg<uint32_t> imageIndex, cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = info ? info.value() : this->cInfo->info } };

      // 
      submission.commandInits.push_back([=, this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        switchToReadyFn[*imageIndex](cmdBuf, this->state);
        return cmdBuf;
      });

      //
      return lxvc::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

  protected:

    //
    virtual void createImage(cpp21::const_wrap_arg<vk::Image> image, cpp21::const_wrap_arg<ImageType> imageType = ImageType::eSwapchain, cpp21::const_wrap_arg<vk::SurfaceFormat2KHR> surfaceFormat2 = {}, cpp21::const_wrap_arg<ImageSwapchainInfo> swapchainInfo = {}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) aspectMask = vk::ImageAspectFlagBits::eColor;
      decltype(auto) components = vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eG, .b = vk::ComponentSwizzle::eB, .a = vk::ComponentSwizzle::eA };
      decltype(auto) imageLayout = vk::ImageLayout::eGeneral;
      decltype(auto) format = surfaceFormat2->surfaceFormat.format;
      decltype(auto) subresourceRange =
        vk::ImageSubresourceRange{
          .aspectMask = aspectMask,
          .baseMipLevel = 0u,
          .levelCount = 1u,
          .baseArrayLayer = 0u,
          .layerCount = 1u
        };

      //
      decltype(auto) imageObj = ResourceObj::make(this->base, ResourceCreateInfo{
        .image = image,
        .imageInfo = ImageCreateInfo{
          .format = format,
          .extent = { capInfo.capabilities->currentExtent.width, capInfo.capabilities->currentExtent.height, 1u },
          .layout = imageLayout,
          .swapchain = swapchainInfo,
          .info = this->cInfo->info ? this->cInfo->info : QueueGetInfo{0u, 0u},
          .type = imageType
        }
      });

      // 
      this->renderArea = vk::Rect2D{ vk::Offset2D{0u, 0u}, capInfo.capabilities->currentExtent };
      this->imageViews.push_back(imageObj->createImageView(ImageViewCreateInfo{
        .viewType = vk::ImageViewType::e2D,
        .subresourceRange = subresourceRange
      }));

      // 
      this->imageViewIndices.push_back(descriptorsObj->images.add(vk::DescriptorImageInfo{ .imageView = this->imageViews.back(), .imageLayout = vk::ImageLayout::eGeneral }));

      // TODO: use pre-built command buffer
      this->switchToReadyFn.push_back([=](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf, cpp21::const_wrap_arg<SwapchainState> previousState = {}) {
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = imageLayout,
          .subresourceRange = subresourceRange,
        });
      });

      //
      this->switchToPresentFn.push_back([=](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf, cpp21::const_wrap_arg<SwapchainState> previousState = {}) {
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = vk::ImageLayout::ePresentSrcKHR,
          .subresourceRange = subresourceRange,
        });
      });

      // 
      ;

      //
      vk::SemaphoreTypeCreateInfo timeline = {};
      timeline.semaphoreType = vk::SemaphoreType::eBinary;
      timeline.initialValue = 0ull;//this->readySemaphores.size();

      //
      decltype(auto) readySemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{  });
      decltype(auto) presentSemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{  });

      //
      this->readySemaphores.push_back(readySemaphore.as<vk::Semaphore>());
      this->presentSemaphores.push_back(presentSemaphore.as<vk::Semaphore>());

      //
      this->readySemaphoreInfos.push_back(readySemaphore->infoMap->get<vk::SemaphoreSubmitInfo>(vk::StructureType::eSemaphoreSubmitInfo));
      this->presentSemaphoreInfos.push_back(presentSemaphore->infoMap->get<vk::SemaphoreSubmitInfo>(vk::StructureType::eSemaphoreSubmitInfo));
      //lxvc::context->get<DeviceObj>(this->base)
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      //decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
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
      this->handle = device.createSwapchainKHR(infoMap->set(vk::StructureType::eSwapchainCreateInfoKHR, vk::SwapchainCreateInfoKHR{
        .surface = cInfo->surface,
        .minImageCount = capInfo.capabilities->minImageCount,
        .imageFormat = surfaceFormat2.surfaceFormat.format,
        .imageColorSpace = surfaceFormat2.surfaceFormat.colorSpace,
        .imageExtent = capInfo.capabilities->currentExtent,
        .imageArrayLayers = std::min(capInfo.capabilities->maxImageArrayLayers, 1u),
        .imageUsage = capInfo.capabilities->supportedUsageFlags,
        .preTransform = capInfo.capabilities->currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = true
      })->setQueueFamilyIndices(deviceObj->queueFamilies.indices));

      //
      images = device.getSwapchainImagesKHR(this->handle.as<vk::SwapchainKHR>());

      // 
      uint32_t imageIndex = 0u;
      for (decltype(auto) image : images) {
        this->createImage(image, ImageType::eSwapchain, surfaceFormat2, ImageSwapchainInfo{
          .swapchain = this->handle.as<vk::SwapchainKHR>(),
          .index = imageIndex++
        }); // 
      };

      // 
      descriptorsObj->updateDescriptors();

      //
      //this->handle = uintptr_t(this);
    };



  public:


    
  };

};
