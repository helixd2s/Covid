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

      // 
      submission.submission.signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{ sets[*imageIndex].readySemaphoreInfo };
      submission.commandInits.push_back([this, imageIndex](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        for (auto& switchFn : this->sets[*imageIndex].switchToPresentFns) { switchFn(cmdBuf); };
        return cmdBuf;
      });

      //
      return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };

    //
    virtual FenceType switchToReady(cpp21::const_wrap_arg<uint32_t> imageIndex, cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo { .info = info ? info.value() : this->cInfo->info } };

      // 
      submission.submission.waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{ sets[*imageIndex].presentSemaphoreInfo };
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
      memcpy(this->currentState.images, this->sets[this->currentState.index].imageViewIndices.data(), std::min(uint32_t(this->sets[this->currentState.index].imageViewIndices.size()), 4u)*4u);
      return this->currentState.index;
    };

    //
    virtual FenceType presentImage(cpp21::const_wrap_arg<ANAMED::QueueGetInfo> qfAndQueue) {
      return this->switchToPresent(this->currentState.index, qfAndQueue);
    };

  protected:

    //
    virtual void createSet(SwapchainSet* set, uint32_t index = 0u) {
      //
      vk::SemaphoreTypeCreateInfo timeline = {};
      timeline.semaphoreType = vk::SemaphoreType::eBinary;
      timeline.initialValue = 0ull;

      // incompatible with export
      decltype(auto) readySemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{ .hasExport = false });
      decltype(auto) presentSemaphore = SemaphoreObj::make(this->base, SemaphoreCreateInfo{ .hasExport = false });

      //
      set->readySemaphoreInfo = readySemaphore->infoMap->get<vk::SemaphoreSubmitInfo>(vk::StructureType::eSemaphoreSubmitInfo);
      set->presentSemaphoreInfo = presentSemaphore->infoMap->get<vk::SemaphoreSubmitInfo>(vk::StructureType::eSemaphoreSubmitInfo);
    }

    //
    virtual SwapchainSet createSet(uint32_t index = 0u) {
      SwapchainSet set = {};
      this->createSet(&set, index);
      return set;
    };

    //
    virtual void createImage(SwapchainSet* set, uint32_t index, cpp21::const_wrap_arg<ImageType> imageType = ImageType::eStorage) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) aspectMask = vk::ImageAspectFlagBits::eColor;
      decltype(auto) components = vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eG, .b = vk::ComponentSwizzle::eB, .a = vk::ComponentSwizzle::eA };
      decltype(auto) imageLayout = vk::ImageLayout::eGeneral;
      decltype(auto) format = this->cInfo->formats[index];
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
        .descriptors = this->cInfo->layout,
        .imageInfo = ImageCreateInfo{
          .format = format,
          .extent = vk::Extent3D{ this->cInfo->extent.width, this->cInfo->extent.height, 1u },
          .layout = imageLayout,
          .info = this->cInfo->info ? this->cInfo->info : QueueGetInfo{0u, 0u},
          .type = imageType
        }
      });

      // 
      decltype(auto) pair = imageObj->createImageView(ImageViewCreateInfo{
        .viewType = vk::ImageViewType::e2D,
        .subresourceRange = subresourceRange,
        .preference = ImageViewPreference::eStorage
      });

      //
      set->imageViews.push_back(std::get<0>(pair));
      set->imageViewIndices.push_back(std::get<1>(pair));

      // TODO: use pre-built command buffer
      set->switchToReadyFns.push_back([=](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf, cpp21::const_wrap_arg<SwapchainState> previousState = {}) {
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = imageLayout,
          .subresourceRange = subresourceRange,
        });
      });

      //
      set->switchToPresentFns.push_back([=](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf, cpp21::const_wrap_arg<SwapchainState> previousState = {}) {
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = vk::ImageLayout::ePresentSrcKHR,
          .subresourceRange = subresourceRange,
        });
      });
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<VirtualSwapchainCreateInfo> cInfo = VirtualSwapchainCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      //decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
      auto& device = this->base.as<vk::Device>();

      //
      this->handle = uintptr_t(this);

      //
      for (uint32_t i=0;i<this->cInfo->minImageCount;i++) {
        this->sets.push_back(this->createSet(i));
      };

      // 
      uint32_t imageIndex = 0u;
      for (uint32_t i=0;i<this->cInfo->minImageCount;i++) {
        uint32_t J = 0u; for (decltype(auto) image : this->cInfo->formats) { uint32_t j = J++;
          this->createImage(&this->sets[i], j, ImageType::eStorage); // 
        };
      };

      // 
      descriptorsObj->updateDescriptors();

      //
      this->currentState.index = this->sets.size()-1u;
      memcpy(this->currentState.images, this->sets[this->currentState.index].imageViewIndices.data(), std::min(uint32_t(this->sets[this->currentState.index].imageViewIndices.size()), 4u)*4u);
    };



  public:


    
  };

};
#endif
