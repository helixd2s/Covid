#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"
#include "./Descriptors.hpp"

// 
namespace lxvc {
  
  //
  struct SurfaceCapabilitiesInfo {
    vk::SurfaceCapabilities2KHR capabilities2 = {};
    std::vector<vk::SurfaceFormat2KHR> formats2 = {};
    std::vector<vk::PresentModeKHR> presentModes = {};
    //vk::PhysicalDeviceSurfaceInfo2KHR info2 = {};
    cpp21::optional_ref<vk::SurfaceCapabilitiesKHR> capabilities = {};
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
    std::vector<std::function<FenceType(std::optional<QueueGetInfo> const&, SwapchainState const&)>> switchToPresentFn = {};
    std::vector<std::function<FenceType(std::optional<QueueGetInfo> const&, SwapchainState const&)>> switchToReadyFn = {};

    //
    SwapchainState state = SwapchainState::eReady;
    SurfaceCapabilitiesInfo capInfo = {};

    //
    vk::Rect2D renderArea = {};

  public:
    // 
    SwapchainObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    SwapchainObj(Handle const& handle, std::optional<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(Handle const& handle, std::optional<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) {
      auto shared = std::make_shared<SwapchainObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual FenceType switchToPresent(uint32_t const& imageIndex, std::optional<QueueGetInfo> const& info = QueueGetInfo{}) {
      //std::vector<FenceType> fences = {};
      //if (this->state != SwapchainState::ePresent) {
        //for (decltype(auto) fn : switchToPresentFn) { fences.push_back(fn(info, this->state)); };
        //this->state = SwapchainState::ePresent;
      //};
      //return fences;
      return switchToPresentFn[imageIndex](info, this->state);
    };

    //
    virtual FenceType switchToReady(uint32_t const& imageIndex, std::optional<QueueGetInfo> const& info = QueueGetInfo{}) {
      //std::vector<FenceType> fences = {};
      //if (this->state != SwapchainState::eReady) {
        //for (decltype(auto) fn : switchToReadyFn) { fences.push_back(fn(info, this->state)); };
        //this->state = SwapchainState::eReady;
      //};
      //return fences;
      return switchToReadyFn[imageIndex](info, this->state);
    };

  protected:

    //
    virtual void createImage(vk::Image const& image, ImageType const& imageType = ImageType::eSwapchain, vk::SurfaceFormat2KHR const& surfaceFormat2 = {}, ImageSwapchainInfo const& swapchainInfo = {}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) aspectMask = vk::ImageAspectFlagBits::eColor;
      decltype(auto) components = vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eG, .b = vk::ComponentSwizzle::eB, .a = vk::ComponentSwizzle::eA };
      decltype(auto) imageLayout = vk::ImageLayout::eGeneral;
      decltype(auto) format = surfaceFormat2.surfaceFormat.format;
      decltype(auto) subresourceRange =
        vk::ImageSubresourceRange{
          .aspectMask = aspectMask,
          .baseMipLevel = 0u,
          .levelCount = 1u,
          .baseArrayLayer = 0u,
          .layerCount = 1u
        };

      //
      ResourceObj::make(this->base, ResourceCreateInfo{
        .image = image,
        .imageInfo = ImageCreateInfo{
          .swapchain = swapchainInfo,
          .info = this->cInfo->info ? this->cInfo->info : QueueGetInfo{0u, 0u},
          .type = imageType,
          .extent = { capInfo.capabilities->currentExtent.width, capInfo.capabilities->currentExtent.height, 1u },
          .format = format,
          .layout = imageLayout
        }
      });

      // 
      renderArea = vk::Rect2D{ vk::Offset2D{0u, 0u}, capInfo.capabilities->currentExtent };

      //
      //decltype(auto) image = this->images.back();

      //
      this->imageViews.push_back(this->base.as<vk::Device>().createImageView(vk::ImageViewCreateInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .components = components,
        .subresourceRange = subresourceRange
      }));

      // 
      this->imageViewIndices.push_back(descriptorsObj->images.add(vk::DescriptorImageInfo{ .imageView = this->imageViews.back(), .imageLayout = vk::ImageLayout::eGeneral }));

      // TODO: use pre-built command buffer
      this->switchToReadyFn.push_back([=](std::optional<QueueGetInfo> const& info = QueueGetInfo{}, SwapchainState const& previousState = {}) {
        return deviceObj->get<ResourceObj>(image)->switchLayout(ImageLayoutSwitchInfo{
          .newImageLayout = imageLayout,
          .subresourceRange = subresourceRange,
          .info = info
        });
      });

      //
      this->switchToPresentFn.push_back([=](std::optional<QueueGetInfo> const& info = QueueGetInfo{}, SwapchainState const& previousState = {}) {
        return deviceObj->get<ResourceObj>(image)->switchLayout(ImageLayoutSwitchInfo{
          .newImageLayout = vk::ImageLayout::ePresentSrcKHR,
          .subresourceRange = subresourceRange,
          .info = info
        });
      });

      //
      vk::SemaphoreTypeCreateInfo timeline = {};
      timeline.semaphoreType = vk::SemaphoreType::eBinary;
      timeline.initialValue = 0ull;//this->readySemaphores.size();

      //
      this->readySemaphores.push_back(device.createSemaphore(vk::SemaphoreCreateInfo{ .pNext = &timeline, .flags = {} }));
      this->presentSemaphores.push_back(device.createSemaphore(vk::SemaphoreCreateInfo{ .pNext = &timeline, .flags = {} }));

      //
      this->readySemaphoreInfos.push_back(vk::SemaphoreSubmitInfo{ .semaphore = this->readySemaphores.back(), .value = timeline.initialValue, .stageMask = vk::PipelineStageFlagBits2::eAllCommands });
      this->presentSemaphoreInfos.push_back(vk::SemaphoreSubmitInfo{ .semaphore = this->presentSemaphores.back(), .value = timeline.initialValue, .stageMask = vk::PipelineStageFlagBits2::eAllCommands });
      //lxvc::context->get<DeviceObj>(this->base)
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<SwapchainCreateInfo> cInfo = SwapchainCreateInfo{}) {
      this->cInfo = cInfo;
      //decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) device = this->base.as<vk::Device>();

      //
      decltype(auto) physicalDevice = deviceObj->physicalDevices[deviceObj->cInfo->physicalDeviceIndex];
      capInfo.capabilities2 = physicalDevice.getSurfaceCapabilities2KHR(vk::PhysicalDeviceSurfaceInfo2KHR{ .surface = cInfo->surface });
      capInfo.capabilities = cpp21::opt_ref(capInfo.capabilities2.surfaceCapabilities);
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
