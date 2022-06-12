#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"
#include "./PipelineLayout.hpp"

// 
namespace ANAMED {
  
  //
  struct FbHistory {
    //
    FramebufferState state = FramebufferState::eShaderRead;

    std::vector<vk::Image> images = {};
    std::vector<vk::ImageView> imageViews = {};
    std::vector<uint32_t> imageViewIndices = {};
    std::vector<vk::RenderingAttachmentInfo> colorAttachments = {};

    //
    vk::RenderingAttachmentInfo depthAttachment = {};
    vk::RenderingAttachmentInfo stencilAttachment = {};

    //
    std::vector<std::function<void(cpp21::optional_ref<vk::CommandBuffer>, cpp21::optional_ref<FramebufferState>)>> switchToShaderReadFn = {};
    std::vector<std::function<void(cpp21::optional_ref<vk::CommandBuffer>, cpp21::optional_ref<FramebufferState>)>> switchToAttachmentFn = {};
    std::vector<std::function<void(cpp21::optional_ref<vk::CommandBuffer>, cpp21::optional_ref<FramebufferState>)>> clearAttachmentFn = {};


  };

  // 
  class FramebufferObj : public BaseObj {
  public: 
    using tType = WrapShared<FramebufferObj>;
    using BaseObj::BaseObj;
    //using BaseObj;

  protected: 
    // 
    friend DeviceObj;
    friend PipelineObj;
    friend ResourceObj;
    friend PipelineObj;
    friend PipelineLayoutObj;

    //
    std::optional<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{};
    std::vector<FbHistory> fbHistory = {};

    //
    uint32_t currentIndex = 0u, previousIndex = 0u;
    vk::Rect2D renderArea = {};

    //
    FramebufferStateInfo currentState = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };


  public:
    //
    virtual FramebufferStateInfo& getStateInfo() { return currentState; };
    virtual FramebufferStateInfo const& getStateInfo() const { return currentState; };

    // 
    FramebufferObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    FramebufferObj(cpp21::optional_ref<Handle> handle, cpp21::optional_ref<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    //
    virtual std::vector<uint32_t> const& getImageViewIndices(FbHistory const& history) const { return history.imageViewIndices; };

    //
    virtual vk::RenderingAttachmentInfo const& getDepthAttachment(FbHistory const& history) const {
      return history.depthAttachment;
    };

    //
    virtual vk::RenderingAttachmentInfo const& getStencilAttachment(FbHistory const& history) const {
      return history.stencilAttachment;
    };

    //
    virtual std::vector<vk::RenderingAttachmentInfo> const& getColorAttachments(FbHistory const& history) const {
      return history.colorAttachments;
    };

    //
    virtual std::vector<uint32_t> const& getImageViewIndices() const { return getImageViewIndices(this->fbHistory[this->currentIndex]); };
    virtual std::vector<uint32_t> const& getPrevImageViewIndices() const { return getImageViewIndices(this->fbHistory[this->previousIndex]); };

    //
    virtual vk::RenderingAttachmentInfo const& getDepthAttachment() const {
      return getDepthAttachment(this->fbHistory[this->currentIndex]);
    };

    //
    virtual vk::RenderingAttachmentInfo const& getStencilAttachment() const {
      return getStencilAttachment(this->fbHistory[this->currentIndex]);
    };

    //
    virtual std::vector<vk::RenderingAttachmentInfo> const& getColorAttachments() const {
      return getColorAttachments(this->fbHistory[this->currentIndex]);
    };

    //
    virtual uint32_t& acquireImage(cpp21::optional_ref<ANAMED::QueueGetInfo> qfAndQueue) {
      this->previousIndex = this->currentIndex;
      this->currentIndex = (++this->currentIndex) % this->fbHistory.size();

      // 
      decltype(auto) framebufferAttachments = this->getImageViewIndices();
      memcpy(currentState.attachments[0], framebufferAttachments.data(), std::min(framebufferAttachments.size(), 8ull) * sizeof(uint32_t));

      // 
      decltype(auto) previousFramebufferAttachments = this->getPrevImageViewIndices();
      memcpy(currentState.attachments[1], previousFramebufferAttachments.data(), std::min(previousFramebufferAttachments.size(), 8ull) * sizeof(uint32_t));

      //
      return this->currentIndex;
    };

    //
    virtual vk::Rect2D const& getRenderArea() const { return renderArea; };

    //
    virtual tType registerSelf() {
      ANAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(cpp21::optional_ref<Handle> handle, cpp21::optional_ref<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) {
      auto shared = std::make_shared<FramebufferObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual tType writeSwitchToShaderRead(FbHistory& history, cpp21::optional_ref<vk::CommandBuffer> cmdBuf) {
      if (history.state != FramebufferState::eShaderRead) {
        for (decltype(auto) fn : history.switchToShaderReadFn) { fn(cmdBuf, history.state); };
        history.state = FramebufferState::eShaderRead;
      };
      return SFT();
    };

    //
    virtual tType writeSwitchToAttachment(FbHistory& history, cpp21::optional_ref<vk::CommandBuffer> cmdBuf) {
      if (history.state != FramebufferState::eAttachment) {
        for (decltype(auto) fn : history.switchToAttachmentFn) { fn(cmdBuf, history.state); };
        history.state = FramebufferState::eAttachment;
      };
      return SFT();
    };


    //
    virtual tType writeSwitchToShaderRead(cpp21::optional_ref<vk::CommandBuffer> cmdBuf) {
      return writeSwitchToShaderRead(this->fbHistory[this->currentIndex], cmdBuf);
    };

    //
    virtual tType writeSwitchToAttachment(cpp21::optional_ref<vk::CommandBuffer> cmdBuf) {
      return writeSwitchToAttachment(this->fbHistory[this->currentIndex], cmdBuf);
    };



    //
    virtual tType writeClearAttachments(FbHistory& history, cpp21::optional_ref<vk::CommandBuffer> cmdBuf) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<PipelineLayoutObj>(this->cInfo->layout);
      decltype(auto) attachment = this->cInfo->attachmentLayout;

      //
      std::vector<vk::ClearRect> clearRects = {};
      std::vector<vk::ClearAttachment> clearAttachments = {};

      // 
      clearRects.push_back(vk::ClearRect{ .rect = this->renderArea, .baseArrayLayer = 0u, .layerCount = this->cInfo->attachmentLayout->type == FramebufferType::eCubemap ? 6u : 1u });
      uint32_t i = 0u; for (decltype(auto) color : history.colorAttachments) {
        uint32_t t = i++;
        clearAttachments.push_back(vk::ClearAttachment{ .aspectMask = vk::ImageAspectFlagBits::eColor, .colorAttachment = t, .clearValue = attachment->colorClearValues[t] });
      };

      //
      clearAttachments.push_back(vk::ClearAttachment{ .aspectMask = vk::ImageAspectFlagBits::eDepth, .clearValue = attachment->depthClearValue });
      clearAttachments.push_back(vk::ClearAttachment{ .aspectMask = vk::ImageAspectFlagBits::eStencil, .clearValue = attachment->stencilClearValue });

      // 
      cmdBuf->beginRendering(vk::RenderingInfoKHR{
        .renderArea = renderArea,
        .layerCount = this->cInfo->attachmentLayout->type == FramebufferType::eCubemap ? 6u : 1u,
        .viewMask = 0x0u,
        .colorAttachmentCount = uint32_t(history.colorAttachments.size()),
        .pColorAttachments = history.colorAttachments.data(),
        .pDepthAttachment = &history.depthAttachment,
        .pStencilAttachment = &history.stencilAttachment
      });
      cmdBuf->clearAttachments(clearAttachments, clearRects);
      cmdBuf->endRendering();

      //
      return SFT();
    };

    //
    virtual FenceType clearAttachments(FbHistory& history, cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}) {
      // 
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo{.info = info.value() } };
      submission.commandInits.push_back([this, &history](cpp21::optional_ref<vk::CommandBuffer> cmdBuf) {
        this->writeClearAttachments(history, cmdBuf);
        return cmdBuf;
      });
      ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);

      //
      return FenceType{};
    };

    //
    virtual FenceType switchToShaderRead(FbHistory& history, cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}) {
      // 
      if (history.state != FramebufferState::eShaderRead) {
        decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo{.info = info.value() }};
        submission.commandInits.push_back([this, &history](cpp21::optional_ref<vk::CommandBuffer> cmdBuf) {
          this->writeSwitchToShaderRead(history, cmdBuf);
          return cmdBuf;
        });
        history.state = FramebufferState::eShaderRead;
        ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
      };

      //
      return FenceType{};
    };

    //
    virtual FenceType switchToAttachment(FbHistory& history, cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}) {
      // 
      if (history.state != FramebufferState::eAttachment) {
        decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo{.info = info.value() } };
        submission.commandInits.push_back([this, &history](cpp21::optional_ref<vk::CommandBuffer> cmdBuf) {
          this->writeSwitchToAttachment(history, cmdBuf);
          return cmdBuf;
        });
        history.state = FramebufferState::eAttachment;
        ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
      };

      //
      return FenceType{};
    };

    //
    virtual FenceType clearAttachments(cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}) {
      return clearAttachments(this->fbHistory[this->currentIndex], info);
    };

    //
    virtual FenceType switchToShaderRead(cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}) {
      return switchToShaderRead(this->fbHistory[this->currentIndex], info);
    };

    //
    virtual FenceType switchToAttachment(cpp21::optional_ref<QueueGetInfo> info = QueueGetInfo{}) {
      return switchToAttachment(this->fbHistory[this->currentIndex], info);
    };


  protected:

    //
    virtual void createImage(FbHistory& history, cpp21::optional_ref<ImageType> imageType = ImageType::eColorAttachment) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<PipelineLayoutObj>(this->cInfo->layout);
      decltype(auto) attachment = this->cInfo->attachmentLayout;
      decltype(auto) lastDepthFormat = attachment->depthAttachmentFormat;
      decltype(auto) lastStencilFormat = attachment->stencilAttachmentFormat;

      // 
      decltype(auto) format = (*imageType) == ImageType::eDepthStencilAttachment ? lastDepthFormat : ((*imageType) == ImageType::eDepthAttachment ? lastDepthFormat : ((*imageType) == ImageType::eStencilAttachment ? lastStencilFormat : attachment->colorAttachmentFormats[history.colorAttachments.size()]));
      decltype(auto) imageLayout = 
        (*imageType) == ImageType::eDepthStencilAttachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal :
        ((*imageType) == ImageType::eDepthAttachment ? vk::ImageLayout::eDepthAttachmentOptimal :
        ((*imageType) == ImageType::eStencilAttachment ? vk::ImageLayout::eStencilAttachmentOptimal : ((*imageType) == ImageType::eColorAttachment ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eGeneral)));

      //
      auto imageObj = ResourceObj::make(this->base, ResourceCreateInfo{
        .descriptors = this->cInfo->layout,
        .imageInfo = ImageCreateInfo{
          .flags = this->cInfo->attachmentLayout->type == FramebufferType::eCubemap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits{},
          .imageType = vk::ImageType::e2D,//this->cInfo->type == FramebufferType::eCubemap ? vk::ImageType::e3D : vk::ImageType::e2D,
          .format = format,
          .extent = vk::Extent3D{ cInfo->extent.width, cInfo->extent.height, /*this->cInfo->type == FramebufferType::eCubemap ? 6u : 1u*/ 1u},
          .layerCount = this->cInfo->attachmentLayout->type == FramebufferType::eCubemap ? 6u : 1u,
          .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
          .type = imageType,
        }
      });

      //
      decltype(auto) subresourceRange = imageObj->subresourceRange(0u, this->cInfo->attachmentLayout->type == FramebufferType::eCubemap ? 6u : 1u, 0u, 1u);
      decltype(auto) pair = imageObj->createImageView(ImageViewCreateInfo{
        .viewType = this->cInfo->attachmentLayout->type == FramebufferType::eCubemap ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
        .subresourceRange = subresourceRange,
        .preference = ImageViewPreference::eSampled
      });

      //
      history.images.push_back(imageObj.as<vk::Image>());
      history.imageViews.push_back(pair.imageView);
      history.imageViewIndices.push_back(pair.indice);

      //
      decltype(auto) imageView = pair.imageView;

      // TODO: use pre-built command buffer
      history.switchToAttachmentFn.push_back([this, device, imageLayout, subresourceRange, image=imageObj.as<vk::Image>(), &history](cpp21::optional_ref<vk::CommandBuffer> cmdBuf, cpp21::optional_ref<FramebufferState> previousState = {}) {
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(device);
        decltype(auto) imageObj = deviceObj->get<ResourceObj>(image);
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = imageLayout,
          .subresourceRange = subresourceRange,
        });
      });

      //
      history.switchToShaderReadFn.push_back([this, device, subresourceRange, image = imageObj.as<vk::Image>(), &history](cpp21::optional_ref<vk::CommandBuffer> cmdBuf, cpp21::optional_ref<FramebufferState> previousState = {}) {
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(device);
        decltype(auto) imageObj = deviceObj->get<ResourceObj>(image);
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
          .subresourceRange = subresourceRange,
        });
      });

      //
      if ((*imageType) == ImageType::eDepthStencilAttachment) {
        history.stencilAttachment = history.depthAttachment = vk::RenderingAttachmentInfo{ .imageView = imageView, .imageLayout = imageLayout, .resolveMode = vk::ResolveModeFlagBits::eNone, .loadOp = vk::AttachmentLoadOp::eLoad, .storeOp = vk::AttachmentStoreOp::eStore, .clearValue = attachment->depthClearValue };
      }
      else
      if ((*imageType) == ImageType::eDepthAttachment) {
        history.depthAttachment = vk::RenderingAttachmentInfo{ .imageView = imageView, .imageLayout = imageLayout, .resolveMode = vk::ResolveModeFlagBits::eNone, .loadOp = vk::AttachmentLoadOp::eLoad, .storeOp = vk::AttachmentStoreOp::eStore, .clearValue = attachment->depthClearValue };
      }
      else 
      if ((*imageType) == ImageType::eStencilAttachment) {
        history.stencilAttachment = vk::RenderingAttachmentInfo{ .imageView = imageView, .imageLayout = imageLayout, .resolveMode = vk::ResolveModeFlagBits::eNone, .loadOp = vk::AttachmentLoadOp::eLoad, .storeOp = vk::AttachmentStoreOp::eStore, .clearValue = attachment->depthClearValue };
      }
      else {
        uintptr_t last = history.colorAttachments.size();
        history.colorAttachments.push_back(vk::RenderingAttachmentInfo{ .imageView = imageView, .imageLayout = imageLayout, .resolveMode = vk::ResolveModeFlagBits::eNone, .loadOp = vk::AttachmentLoadOp::eLoad, .storeOp = vk::AttachmentStoreOp::eStore, .clearValue = attachment->colorClearValues[last] });
      };
    };

    //
    virtual void updateFramebuffer() {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<PipelineLayoutObj>(this->cInfo->layout);

      // 
      for (auto& history : this->fbHistory) { //
        decltype(auto) it = history.images.begin();
        for (it = history.images.begin(); it != history.images.end();) {
          deviceObj->get<ResourceObj>(*it)->destroy(deviceObj.get());
          it = history.images.erase(it);
        };
      };

      //
      this->renderArea = vk::Rect2D{ vk::Offset2D{0u, 0u}, cInfo->extent };

      // avoid indirection
      this->fbHistory = {};
      for (uint32_t i = 0; i < this->cInfo->minImageCount; i++) {
        this->fbHistory.push_back(FbHistory{});
      };

      // re-loop 
      for (auto& history : this->fbHistory) {

        //
        for (auto& format : this->cInfo->attachmentLayout->colorAttachmentFormats) {
          this->createImage(history, ImageType::eUniversal);
        };

        // 
        if (this->cInfo->attachmentLayout->depthAttachmentFormat == this->cInfo->attachmentLayout->stencilAttachmentFormat) {
          this->createImage(history, ImageType::eDepthStencilAttachment);
        }
        else {
          this->createImage(history, ImageType::eDepthAttachment);
          this->createImage(history, ImageType::eStencilAttachment);
        };
      };

      // 
      decltype(auto) framebufferAttachments = this->getImageViewIndices();
      memcpy(currentState.attachments[0], framebufferAttachments.data(), std::min(framebufferAttachments.size(), 8ull) * sizeof(uint32_t));

      // 
      decltype(auto) previousFramebufferAttachments = this->getPrevImageViewIndices();
      memcpy(currentState.attachments[1], previousFramebufferAttachments.data(), std::min(previousFramebufferAttachments.size(), 8ull) * sizeof(uint32_t));

      //
      currentState.extent = glm::uvec2(cInfo->extent.width, cInfo->extent.height);

      // 
      descriptorsObj->updateDescriptors();
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      this->handle = uintptr_t(this);
      this->updateFramebuffer();
    };



  public:


    
  };

};
#endif
