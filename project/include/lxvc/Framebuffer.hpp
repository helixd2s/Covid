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

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

    //
    std::optional<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{};

    //
    std::vector<vk::Image> images = {};
    std::vector<vk::ImageView> imageViews = {};
    std::vector<uint32_t> imageViewIndices = {};

    //
    std::vector<std::function<FenceType(std::optional<QueueGetInfo> const&, FramebufferState const&)>> switchToShaderReadFn = {};
    std::vector<std::function<FenceType(std::optional<QueueGetInfo> const&, FramebufferState const&)>> switchToAttachmentFn = {};

    //
    FramebufferState state = FramebufferState::eAttachment;

  public:
    // 
    FramebufferObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    FramebufferObj(Handle const& handle, std::optional<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) : cInfo(cInfo) {
      this->construct(lxvc::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    //
    virtual tType registerSelf() {
      lxvc::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(Handle const& handle, std::optional<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) {
      return std::make_shared<FramebufferObj>(handle, cInfo)->registerSelf();
    };

    //
    virtual std::vector<FenceType> switchToShaderRead(std::optional<QueueGetInfo> const& info = QueueGetInfo{}) {
      std::vector<FenceType> fences = {};
      if (this->state != FramebufferState::eShaderRead) {
        for (decltype(auto) fn : switchToShaderReadFn) { fences.push_back(fn(info, this->state)); };
        this->state = FramebufferState::eShaderRead;
      };
      return fences;
    };

    //
    virtual std::vector<FenceType> switchToAttachment(std::optional<QueueGetInfo> const& info = QueueGetInfo{}) {
      std::vector<FenceType> fences = {};
      if (this->state != FramebufferState::eAttachment) {
        for (decltype(auto) fn : switchToAttachmentFn) { fences.push_back(fn(info, this->state)); };
        this->state = FramebufferState::eAttachment;
      };
      return fences;
    };

  protected:

    //
    virtual void createImage(ImageType const& imageType = ImageType::eColorAttachment, vk::Format const& format_ = vk::Format::eR32G32B32A32Sfloat) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) format = imageType == ImageType::eDepthAttachment ? vk::Format::eD32Sfloat : (imageType == ImageType::eStencilAttachment ? vk::Format::eS8Uint : format_);
      decltype(auto) aspectMask =
        imageType == ImageType::eDepthAttachment ? vk::ImageAspectFlagBits::eDepth :
        (imageType == ImageType::eStencilAttachment ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor);
      decltype(auto) components = imageType == ImageType::eDepthAttachment || imageType == ImageType::eStencilAttachment
        ? vk::ComponentMapping{ .r = vk::ComponentSwizzle::eZero, .g = vk::ComponentSwizzle::eZero, .b = vk::ComponentSwizzle::eZero, .a = vk::ComponentSwizzle::eZero }
        : vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eG, .b = vk::ComponentSwizzle::eB, .a = vk::ComponentSwizzle::eA };
      decltype(auto) imageLayout = 
          imageType == ImageType::eDepthAttachment ? vk::ImageLayout::eDepthAttachmentOptimal :
        (imageType == ImageType::eStencilAttachment ? vk::ImageLayout::eStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal);
      decltype(auto) subresourceRange =
        vk::ImageSubresourceRange{
          .aspectMask = aspectMask,
          .baseMipLevel = 0u,
          .levelCount = 1u,
          .baseArrayLayer = 0u,
          .layerCount = 1u
        };

      //
      this->images.push_back(ResourceObj::make(this->base, ResourceCreateInfo{
        .imageInfo = ImageCreateInfo{
          .type = imageType,
          .extent = { cInfo->extent.width, cInfo->extent.height, 1u },
          .format = format,
          .layout = imageLayout
        }
      }).as<vk::Image>());

      //
      decltype(auto) image = this->images.back();

      //
      this->imageViews.push_back(this->base.as<vk::Device>().createImageView(vk::ImageViewCreateInfo{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .components = components,
        .subresourceRange = subresourceRange
      }));

      // 
      this->imageViewIndices.push_back(descriptorsObj->textures.add(vk::DescriptorImageInfo{ .imageView = this->imageViews.back(), .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal }));

      // TODO: use pre-built command buffer
      this->switchToAttachmentFn.push_back([=](std::optional<QueueGetInfo> const& info = QueueGetInfo{}, FramebufferState const& previousState = {}) {
        return deviceObj->get<ResourceObj>(image)->switchLayout(ImageLayoutSwitchInfo{
          .newImageLayout = imageLayout,
          .subresourceRange = subresourceRange,
          .info = info
        });
      });

      //
      this->switchToShaderReadFn.push_back([=](std::optional<QueueGetInfo> const& info = QueueGetInfo{}, FramebufferState const& previousState = {}) {
        return deviceObj->get<ResourceObj>(image)->switchLayout(ImageLayoutSwitchInfo{
          .newImageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
          .subresourceRange = subresourceRange,
          .info = info
        });
      });

      //
      this->handle = uintptr_t(this);

      //lxvc::context->get<DeviceObj>(this->base)
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<FramebufferCreateInfo> cInfo = FramebufferCreateInfo{}) {
      this->cInfo = cInfo;
      //decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(this->cInfo->layout);

      // 
      this->createImage(ImageType::eColorAttachment); // for albedo
      this->createImage(ImageType::eColorAttachment); // for triangle data
      this->createImage(ImageType::eDepthAttachment);
      this->createImage(ImageType::eStencilAttachment);

      // 
      descriptorsObj->updateDescriptors();
    };



  public:


    
  };

};
