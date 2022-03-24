#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"
#include "./Descriptors.hpp"

// 
namespace ZNAMED {
  
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
    friend DescriptorsObj;

    //
    std::optional<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{};

    //
    std::vector<vk::Image> images = {};
    std::vector<vk::ImageView> imageViews = {};
    std::vector<uint32_t> imageViewIndices = {};

    //
    std::vector<vk::RenderingAttachmentInfo> colorAttachments = {};

    //
    vk::RenderingAttachmentInfo depthAttachment = {};
    vk::RenderingAttachmentInfo stencilAttachment = {};

    //
    std::vector<std::function<void(cpp21::const_wrap_arg<vk::CommandBuffer>, cpp21::const_wrap_arg<FramebufferState>)>> switchToShaderReadFn = {};
    std::vector<std::function<void(cpp21::const_wrap_arg<vk::CommandBuffer>, cpp21::const_wrap_arg<FramebufferState>)>> switchToAttachmentFn = {};
    std::vector<std::function<void(cpp21::const_wrap_arg<vk::CommandBuffer>, cpp21::const_wrap_arg<FramebufferState>)>> clearAttachmentFn = {};

    //
    FramebufferState state = FramebufferState::eShaderRead;

    //
    vk::Rect2D renderArea = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };


  public:
    // 
    FramebufferObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) : BaseObj(deviceObj), cInfo(cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    FramebufferObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      //this->construct(ZNAMED::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    //
    virtual std::vector<uint32_t> const& getImageViewIndices() const { return imageViewIndices; };

    //
    virtual vk::RenderingAttachmentInfo const& getDepthAttachment() const {
      return depthAttachment;
    };

    //
    virtual vk::RenderingAttachmentInfo const& getStencilAttachment() const {
      return stencilAttachment;
    };

    //
    virtual std::vector<vk::RenderingAttachmentInfo> const& getColorAttachments() {
      return colorAttachments;
    };

    //
    virtual vk::Rect2D const& getRenderArea() const { return renderArea; };

    //
    virtual tType registerSelf() {
      ZNAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) {
      auto shared = std::make_shared<FramebufferObj>(handle, cInfo);
      shared->construct(ZNAMED::context->get<DeviceObj>(handle), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual tType writeSwitchToShaderRead(cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
      if (this->state != FramebufferState::eShaderRead) {
        for (decltype(auto) fn : switchToShaderReadFn) { fn(cmdBuf, this->state); };
        this->state = FramebufferState::eShaderRead;
      };
      return SFT();
    };

    //
    virtual tType writeSwitchToAttachment(cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
      if (this->state != FramebufferState::eAttachment) {
        for (decltype(auto) fn : switchToAttachmentFn) { fn(cmdBuf, this->state); };
        this->state = FramebufferState::eAttachment;
      };
      return SFT();
    };

    //
    virtual tType writeClearAttachments(cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) attachment = descriptorsObj->cInfo->attachments[std::to_underlying(this->cInfo->type)];

      //
      std::vector<vk::ClearRect> clearRects = {};
      std::vector<vk::ClearAttachment> clearAttachments = {};

      // 
      clearRects.push_back(vk::ClearRect{ .rect = this->renderArea, .baseArrayLayer = 0u, .layerCount = this->cInfo->type == FramebufferType::eCubemap ? 6u : 1u });
      uint32_t i = 0u; for (decltype(auto) color : this->colorAttachments) {
        uint32_t t = i++;
        clearAttachments.push_back(vk::ClearAttachment{ .aspectMask = vk::ImageAspectFlagBits::eColor, .colorAttachment = t, .clearValue = attachment.colorClearValues[t] });
      };

      //
      clearAttachments.push_back(vk::ClearAttachment{ .aspectMask = vk::ImageAspectFlagBits::eDepth, .clearValue = attachment.depthClearValue });
      clearAttachments.push_back(vk::ClearAttachment{ .aspectMask = vk::ImageAspectFlagBits::eStencil, .clearValue = attachment.stencilClearValue });

      // 
      cmdBuf->beginRendering(vk::RenderingInfoKHR{
        .renderArea = renderArea,
        .layerCount = this->cInfo->type == FramebufferType::eCubemap ? 6u : 1u,
        .viewMask = 0x0u,
        .colorAttachmentCount = uint32_t(colorAttachments.size()),
        .pColorAttachments = colorAttachments.data(),
        .pDepthAttachment = &depthAttachment,
        .pStencilAttachment = &stencilAttachment
      });
      cmdBuf->clearAttachments(clearAttachments, clearRects);
      cmdBuf->endRendering();

      //
      return SFT();
    };

    //
    virtual FenceType clearAttachments(cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      // 
      decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo{.info = info } };
      submission.commandInits.push_back([this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        this->writeClearAttachments(cmdBuf);
        return cmdBuf;
      });
      ZNAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);

      //
      return FenceType{};
    };

    //
    virtual FenceType switchToShaderRead(cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      // 
      if (this->state != FramebufferState::eShaderRead) {
        decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo{.info = info } };
        submission.commandInits.push_back([this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
          this->writeSwitchToShaderRead(cmdBuf);
          return cmdBuf;
        });
        this->state = FramebufferState::eShaderRead;
        ZNAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
      };

      //
      return FenceType{};
    };

    //
    virtual FenceType switchToAttachment(cpp21::const_wrap_arg<QueueGetInfo> info = QueueGetInfo{}) {
      // 
      if (this->state != FramebufferState::eAttachment) {
        decltype(auto) submission = CommandOnceSubmission{ .submission = SubmissionInfo{.info = info } };
        submission.commandInits.push_back([this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
          this->writeSwitchToAttachment(cmdBuf);
          return cmdBuf;
        });
        this->state = FramebufferState::eAttachment;
        ZNAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
      };

      //
      return FenceType{};
    };

  protected:

    //
    virtual void createImage(cpp21::const_wrap_arg<ImageType> imageType = ImageType::eColorAttachment) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) attachment = descriptorsObj->cInfo->attachments[std::to_underlying(this->cInfo->type)];
      decltype(auto) lastDepthFormat = attachment.depthAttachmentFormat;
      decltype(auto) lastStencilFormat = attachment.stencilAttachmentFormat;

      decltype(auto) format = (*imageType) == ImageType::eDepthStencilAttachment ? lastDepthFormat : ((*imageType) == ImageType::eDepthAttachment ? lastDepthFormat : ((*imageType) == ImageType::eStencilAttachment ? lastStencilFormat : attachment.colorAttachmentFormats[colorAttachments.size()]));
      decltype(auto) aspectMask =
        (*imageType) == ImageType::eDepthStencilAttachment ? (vk::ImageAspectFlagBits::eDepth) :
        ((*imageType) == ImageType::eDepthAttachment ? vk::ImageAspectFlagBits::eDepth :
        ((*imageType) == ImageType::eStencilAttachment ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor));
      decltype(auto) components = (*imageType) == ImageType::eDepthStencilAttachment || (*imageType) == ImageType::eDepthAttachment || (*imageType) == ImageType::eStencilAttachment
        ? vk::ComponentMapping{ .r = vk::ComponentSwizzle::eZero, .g = vk::ComponentSwizzle::eZero, .b = vk::ComponentSwizzle::eZero, .a = vk::ComponentSwizzle::eZero }
        : vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eG, .b = vk::ComponentSwizzle::eB, .a = vk::ComponentSwizzle::eA };
      decltype(auto) imageLayout = 
        (*imageType) == ImageType::eDepthStencilAttachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal :
        ((*imageType) == ImageType::eDepthAttachment ? vk::ImageLayout::eDepthAttachmentOptimal :
        ((*imageType) == ImageType::eStencilAttachment ? vk::ImageLayout::eStencilAttachmentOptimal : ((*imageType) == ImageType::eColorAttachment ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eGeneral)));
      decltype(auto) subresourceRange =
        vk::ImageSubresourceRange{
          .aspectMask = aspectMask,
          .baseMipLevel = 0u,
          .levelCount = 1u,
          .baseArrayLayer = 0u,
          .layerCount = this->cInfo->type == FramebufferType::eCubemap ? 6u : 1u
        };

      //
      auto imageObj = ResourceObj::make(this->base, ResourceCreateInfo{
        .imageInfo = ImageCreateInfo{
          .flags = this->cInfo->type == FramebufferType::eCubemap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits{},
          .imageType = vk::ImageType::e2D,//this->cInfo->type == FramebufferType::eCubemap ? vk::ImageType::e3D : vk::ImageType::e2D,
          .format = format,
          .extent = vk::Extent3D{ cInfo->extent.width, cInfo->extent.height, /*this->cInfo->type == FramebufferType::eCubemap ? 6u : 1u*/ 1u},
          .layerCount = this->cInfo->type == FramebufferType::eCubemap ? 6u : 1u,
          .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
          .type = imageType,
        }
      });

      //
      this->images.push_back(imageObj.as<vk::Image>());
      this->imageViews.push_back(std::get<0>(imageObj->createImageView(ImageViewCreateInfo{
        .viewType = this->cInfo->type == FramebufferType::eCubemap ? vk::ImageViewType::eCube : vk::ImageViewType::e2D,
        .subresourceRange = subresourceRange,
        .preference = ImageViewPreference::eSampled
      })));

      //
      decltype(auto) imageView = this->imageViews.back();

      // 
      this->imageViewIndices.push_back(descriptorsObj->textures.add(vk::DescriptorImageInfo{ .imageView = imageView,.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal }));

      // TODO: use pre-built command buffer
      this->switchToAttachmentFn.push_back([imageLayout, subresourceRange, imageObj](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf, cpp21::const_wrap_arg<FramebufferState> previousState = {}) {
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = imageLayout,
          .subresourceRange = subresourceRange,
        });
      });

      //
      this->switchToShaderReadFn.push_back([subresourceRange, imageObj](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf, cpp21::const_wrap_arg<FramebufferState> previousState = {}) {
        imageObj->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{
          .cmdBuf = cmdBuf,
          .newImageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
          .subresourceRange = subresourceRange,
        });
      });

      //
      if ((*imageType) == ImageType::eDepthStencilAttachment) {
        stencilAttachment = depthAttachment = vk::RenderingAttachmentInfo{ .imageView = imageView, .imageLayout = imageLayout, .resolveMode = vk::ResolveModeFlagBits::eNone, .loadOp = vk::AttachmentLoadOp::eClear, .storeOp = vk::AttachmentStoreOp::eStore, .clearValue = attachment.depthClearValue };
      }
      else
      if ((*imageType) == ImageType::eDepthAttachment) {
        depthAttachment = vk::RenderingAttachmentInfo{ .imageView = imageView, .imageLayout = imageLayout, .resolveMode = vk::ResolveModeFlagBits::eNone, .loadOp = vk::AttachmentLoadOp::eClear, .storeOp = vk::AttachmentStoreOp::eStore, .clearValue = attachment.depthClearValue };
      }
      else 
      if ((*imageType) == ImageType::eStencilAttachment) {
        stencilAttachment = vk::RenderingAttachmentInfo{ .imageView = imageView, .imageLayout = imageLayout, .resolveMode = vk::ResolveModeFlagBits::eNone, .loadOp = vk::AttachmentLoadOp::eClear, .storeOp = vk::AttachmentStoreOp::eStore, .clearValue = attachment.depthClearValue };
      }
      else {
        uintptr_t last = colorAttachments.size();
        colorAttachments.push_back(vk::RenderingAttachmentInfo{ .imageView = imageView, .imageLayout = imageLayout, .resolveMode = vk::ResolveModeFlagBits::eNone, .loadOp = vk::AttachmentLoadOp::eClear, .storeOp = vk::AttachmentStoreOp::eStore, .clearValue = attachment.colorClearValues[last] });
      };
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      //decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);

      //
      this->renderArea = vk::Rect2D{ vk::Offset2D{0u, 0u}, cInfo->extent };

      // 
      glm::vec4 color = glm::vec4(0.f,0.f,0.f,0.f);
      for (auto& format : descriptorsObj->cInfo->attachments[std::to_underlying(this->cInfo->type)].colorAttachmentFormats) {
        this->createImage(ImageType::eUniversal);
      };

      // 
      if (descriptorsObj->cInfo->attachments[std::to_underlying(this->cInfo->type)].depthAttachmentFormat == descriptorsObj->cInfo->attachments[std::to_underlying(this->cInfo->type)].stencilAttachmentFormat) {
        this->createImage(ImageType::eDepthStencilAttachment);
      } else {
        this->createImage(ImageType::eDepthAttachment);
        this->createImage(ImageType::eStencilAttachment);
      };

      // 
      descriptorsObj->updateDescriptors();
      this->handle = uintptr_t(this);
    };



  public:


    
  };

};
