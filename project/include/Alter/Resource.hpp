#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Device.hpp"
#include "./MemoryAllocator.hpp"
#include "./Descriptors.hpp"
#include "./Sampler.hpp"

// 
namespace ANAMED {

  // 
  class ResourceObj : public BaseObj {
  public: 
    using tType = WrapShared<ResourceObj>;
    using BaseObj::BaseObj;

  protected:
    //using BaseObj;
    friend DeviceObj;
    friend DescriptorsObj;
    friend UploaderObj;
    friend FramebufferObj;
    friend SwapchainObj;
    friend GeometryLevelObj;
    friend InstanceLevelObj;
    friend MemoryAllocatorObj;
    friend VirtualSwapchainObj;

    // 
    //vk::Buffer buffer = {};
    //vk::Image image = {};
    //vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
    void* mappedMemory = nullptr;
    uintptr_t deviceAddress = 0ull;

    // 
    std::optional<AllocatedMemory> allocated = AllocatedMemory{};
    std::optional<ResourceCreateInfo> cInfo = ResourceCreateInfo{};
    std::optional<MemoryRequirements> mReqs = {};
    //std::shared_ptr<MSS> infoMap = {};

    //
    //std::shared_ptr<DeviceObj> deviceObj = {};

    //
    std::vector<vk::ImageView> imageViews = {};
    std::vector<vk::BufferView> bufferViews = {};

    //
    vk::BufferUsageFlags bufferUsage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
    vk::ImageUsageFlags imageUsage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
    MemoryUsage memoryUsage = MemoryUsage::eGpuOnly;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };


  public:
    // 
    ResourceObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
      //this->construct(deviceObj, cInfo);
    };

    // 
    ResourceObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    //
    std::tuple<vk::ImageView, uint32_t> createImageView(cpp21::const_wrap_arg<ImageViewCreateInfo> info = {}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = this->cInfo->descriptors ? deviceObj->get<DescriptorsObj>(this->cInfo->descriptors) : WrapShared<DescriptorsObj>{};

      // 
      auto& imageInfo = this->cInfo->imageInfo;

      // 
      decltype(auto) aspectMask =
         imageInfo->type == ImageType::eDepthStencilAttachment ? (vk::ImageAspectFlagBits::eDepth) :
        (imageInfo->type == ImageType::eDepthAttachment ? vk::ImageAspectFlagBits::eDepth :
        (imageInfo->type == ImageType::eStencilAttachment ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor));
      decltype(auto) components = imageInfo->type == ImageType::eDepthStencilAttachment || imageInfo->type == ImageType::eDepthAttachment || imageInfo->type == ImageType::eStencilAttachment
        ? vk::ComponentMapping{ .r = vk::ComponentSwizzle::eR, .g = vk::ComponentSwizzle::eZero, .b = vk::ComponentSwizzle::eZero, .a = vk::ComponentSwizzle::eZero }
        : info->componentMapping;
      decltype(auto) imageLayout =
         imageInfo->type == ImageType::eDepthStencilAttachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal :
        (imageInfo->type == ImageType::eDepthAttachment ? vk::ImageLayout::eDepthAttachmentOptimal :
        (imageInfo->type == ImageType::eStencilAttachment ? vk::ImageLayout::eStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal));

      //
      decltype(auto) imageView = device.createImageView(vk::ImageViewCreateInfo{
        .image = this->handle.as<vk::Image>(),
        .viewType = info->viewType,
        .format = imageInfo->format,
        .components = components,
        .subresourceRange = vk::ImageSubresourceRange(info->subresourceRange).setAspectMask(aspectMask)
      });

      //
      this->imageViews.push_back(imageView);
      uint32_t descriptorId = 0xFFFFFFFFu;

      // 
      uint32_t imvType = 0u;
      if (descriptorsObj) {
        if ((this->imageUsage & vk::ImageUsageFlagBits::eStorage) && info->preference == ImageViewPreference::eStorage || !(this->imageUsage & vk::ImageUsageFlagBits::eSampled)) {
          descriptorId = descriptorsObj->images.add(vk::DescriptorImageInfo{ .imageView = imageView, .imageLayout = this->cInfo->imageInfo->layout }); imvType = 1u;
        } else
        if ((this->imageUsage & vk::ImageUsageFlagBits::eSampled) && info->preference == ImageViewPreference::eSampled || !(this->imageUsage & vk::ImageUsageFlagBits::eStorage)) {
          descriptorId = descriptorsObj->textures.add(vk::DescriptorImageInfo{ .imageView = imageView, .imageLayout = this->cInfo->imageInfo->layout }); imvType = 2u;
        };

        // 
        this->destructors.insert(this->destructors.begin(), 1, [device, descriptorId, images = (imvType == 1u ? descriptorsObj->getImageDescriptors() : descriptorsObj->getTextureDescriptors())](BaseObj const* baseObj) {
          const_cast<cpp21::bucket<vk::DescriptorImageInfo>&>(images).removeByIndex(descriptorId);
        });
      };

      //
      this->destructors.insert(this->destructors.begin(), 1, [device, imageView](BaseObj const* baseObj) {
        device.destroyImageView(imageView);
      });

      // 
      return std::tuple{ imageView, descriptorId }; // don't return reference, may broke vector
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
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
      auto shared = std::make_shared<ResourceObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual uintptr_t& getDeviceAddress() { return this->deviceAddress; };
    virtual uintptr_t const& getDeviceAddress() const { return this->deviceAddress; };

    //
    virtual vk::ImageLayout& getImageLayout() { return cInfo->imageInfo->layout; };
    virtual vk::ImageLayout const& getImageLayout() const { return cInfo->imageInfo->layout; };

    //
    virtual vk::ImageUsageFlags& getImageUsage() { return imageUsage; };
    virtual vk::ImageUsageFlags const& getImageUsage() const { return imageUsage; };

    //
    virtual vk::BufferUsageFlags& getBufferUsage() { return bufferUsage; };
    virtual vk::BufferUsageFlags const& getBufferUsage() const { return bufferUsage; };

  protected:

    //
    virtual std::optional<AllocatedMemory>& allocateMemory(cpp21::const_wrap_arg<MemoryRequirements> requirements) {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) memoryAllocatorObj = deviceObj->getExt<MemoryAllocatorObj>(this->cInfo->extUsed && this->cInfo->extUsed->find(ExtensionInfoName::eMemoryAllocator) != this->cInfo->extUsed->end() ? this->cInfo->extUsed->at(ExtensionInfoName::eMemoryAllocator) : ExtensionName::eMemoryAllocator);
      return memoryAllocatorObj->allocateMemory(requirements, this->allocated, this->extHandle, this->cInfo->extInfoMap, this->mappedMemory, this->destructors);
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
      if (cInfo) { this->cInfo = cInfo; };
      if (this->cInfo) {
        if (this->cInfo->imageInfo) { this->createImage(this->cInfo->imageInfo.value()); };
        if (this->cInfo->bufferInfo) { this->createBuffer(this->cInfo->bufferInfo.value()); };
      };
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
    virtual vk::BufferUsageFlags& handleBufferUsage(BufferType const& bufferType) {
      // 
      switch (bufferType) {
      case BufferType::eStorage:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |=
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
          vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT |
          vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
          vk::BufferUsageFlagBits::eShaderBindingTableKHR |
          vk::BufferUsageFlagBits::eStorageTexelBuffer;
        break;

      case BufferType::eVertex:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |=
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT |
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
          vk::BufferUsageFlagBits::eStorageTexelBuffer;
        break;

      case BufferType::eIndex:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |=
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT |
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
          vk::BufferUsageFlagBits::eStorageTexelBuffer;
        break;

      case BufferType::eHostMap:
        memoryUsage = MemoryUsage::eCpuToGpu;
        bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eUniformTexelBuffer;
        break;

      case BufferType::eUniform:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eUniformTexelBuffer;
        break;

      case BufferType::eUniversal:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |=
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
          vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT |
          vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT |
          vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
          vk::BufferUsageFlagBits::eShaderBindingTableKHR |
          vk::BufferUsageFlagBits::eStorageTexelBuffer
          ;
        break;

      default:;
      };

      return bufferUsage;
    };

    // 
    virtual FenceType createImage(cpp21::const_wrap_arg<ImageCreateInfo> cInfo = {}) {
      // 
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();

      //
      decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryImageCreateInfo, vk::ExternalMemoryImageCreateInfo{
        .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
      });

      // 
      decltype(auto) imageUsage = this->handleImageUsage(cInfo->type);
      decltype(auto) imageInfo = infoMap->set(vk::StructureType::eImageCreateInfo, vk::ImageCreateInfo{
        .pNext = memoryUsage == MemoryUsage::eGpuOnly ? externalInfo.get() : nullptr,
        .flags = cInfo->flags,
        .imageType = cInfo->imageType,
        .format = cInfo->format,
        .extent = cInfo->extent,
        .mipLevels = cInfo->mipLevelCount,
        .arrayLayers = cInfo->layerCount, // TODO: correct array layers
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = imageUsage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
      });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{
          
        }).get()
      });

      //
      device.getImageMemoryRequirements2(infoMap->set(vk::StructureType::eImageMemoryRequirementsInfo2, vk::ImageMemoryRequirementsInfo2{
        .image = (this->handle = this->cInfo->image ? this->cInfo->image.value() : device.createImage(imageInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices))).as<vk::Image>()
      }).get(), memReqInfo2.get());

      //
      destructors.push_back([device, image = this->handle.as<vk::Image>(), type=cInfo->type](BaseObj const*) {
        if (type!=ImageType::eSwapchain) {
          device.waitIdle();
          device.destroyImage(image);
        };
      });

      // 
      this->allocated = this->allocateMemory(this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .requirements = memReqInfo2->memoryRequirements,
        .dedicated = DedicatedMemory{.image = cInfo->type != ImageType::eSwapchain ? this->handle.as<vk::Image>() : vk::Image{} },
      });

      //
      std::vector<vk::BindImageMemoryInfo> bindInfos = { *infoMap->set(vk::StructureType::eBindImageMemoryInfo, vk::BindImageMemoryInfo{
        .pNext = cInfo->swapchain ? infoMap->set(vk::StructureType::eBindImageMemorySwapchainInfoKHR, vk::BindImageMemorySwapchainInfoKHR{
          .swapchain = cInfo->swapchain->swapchain,
          .imageIndex = cInfo->swapchain->index
         }).get() : nullptr,
        .image = this->handle.as<vk::Image>(), .memory = cInfo->swapchain ? vk::DeviceMemory{} : this->allocated->memory, .memoryOffset = cInfo->swapchain ? 0ull : this->allocated->offset
      }) };
      device.bindImageMemory2(bindInfos);

      // 
      if (cInfo->info) {
        // # another reason of internal error (mostly with std::optional)
        return this->executeSwitchLayoutOnce(ImageLayoutSwitchInfo{
          .info = cInfo->info,
          .switchInfo = ImageLayoutSwitchWriteInfo{
            .newImageLayout = cInfo->layout,
            .oldImageLayout = std::optional<vk::ImageLayout>(imageInfo->initialLayout),
          },
        });
      };

      return FenceType{};
    };

    // 
    virtual void createBuffer(cpp21::const_wrap_arg<BufferCreateInfo> cInfo = {}) {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();

      //
      decltype(auto) externalInfo = infoMap->set(vk::StructureType::eExternalMemoryBufferCreateInfo, vk::ExternalMemoryBufferCreateInfo{
        .handleTypes = memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{},
      });

      // 
      decltype(auto) bufferUsage = this->handleBufferUsage(cInfo->type);
      decltype(auto) bufferInfo = infoMap->set(vk::StructureType::eBufferCreateInfo, vk::BufferCreateInfo{
        .pNext = memoryUsage == MemoryUsage::eGpuOnly ? externalInfo.get() : nullptr,
        .size = cInfo->size,
        .usage = bufferUsage,
        .sharingMode = vk::SharingMode::eExclusive
      });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{}).get()
      });

      //
      device.getBufferMemoryRequirements2(infoMap->set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{
        .buffer = (this->handle = this->cInfo->buffer ? this->cInfo->buffer.value() : device.createBuffer(bufferInfo->setQueueFamilyIndices(deviceObj->getQueueFamilies().indices))).as<vk::Buffer>()
      }).get(), memReqInfo2.get());

      //
      destructors.push_back([device, buffer = this->handle.as<vk::Buffer>()](BaseObj const*) {
        device.waitIdle();
        device.destroyBuffer(buffer);
      });

      //
      this->allocated = this->allocateMemory(this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .requirements = memReqInfo2->memoryRequirements,
        .hasDeviceAddress = !!(bufferUsage & vk::BufferUsageFlagBits::eShaderDeviceAddress),
        .dedicated = DedicatedMemory{.buffer = this->handle.as<vk::Buffer>() }
      });

      //
      std::vector<vk::BindBufferMemoryInfo> bindInfos = { *infoMap->set(vk::StructureType::eBindBufferMemoryInfo, vk::BindBufferMemoryInfo{
        .buffer = this->handle.as<vk::Buffer>(), .memory = this->allocated->memory, .memoryOffset = this->allocated->offset
      }) };
      device.bindBufferMemory2(bindInfos);

      // 
      if (bufferUsage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
        this->deviceAddress = device.getBufferAddress(vk::BufferDeviceAddressInfo{
          .buffer = this->handle.as<vk::Buffer>()
          });
        deviceObj->getAddressSpace().insert({this->deviceAddress, this->deviceAddress + cInfo->size}, this->handle.as<vk::Buffer>());
      };
    };

  public:

    //
    virtual void writeClearCommand(cpp21::const_wrap_arg<ImageClearWriteInfo> clearInfo) {
      if (this->cInfo->imageInfo && this->handle.type == HandleType::eImage) {
        //decltype(auto) info = switchInfo.info ? switchInfo.info : this->cInfo->imageInfo->info;
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
        decltype(auto) imageLayout = this->cInfo->imageInfo->layout;
        decltype(auto) subresourceRange = clearInfo->subresourceRange ? clearInfo->subresourceRange.value() : vk::ImageSubresourceRange{
          .aspectMask =
            this->cInfo->imageInfo->type == ImageType::eDepthStencilAttachment ? (vk::ImageAspectFlagBits::eDepth) :
            (this->cInfo->imageInfo->type == ImageType::eDepthAttachment ? vk::ImageAspectFlagBits::eDepth :
            (this->cInfo->imageInfo->type == ImageType::eStencilAttachment ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor)),
          .baseMipLevel = 0u,
          .levelCount = imageInfo->mipLevels,
          .baseArrayLayer = 0u,
          .layerCount = imageInfo->arrayLayers
        };

        // 
        bool isValidLayout = imageLayout == vk::ImageLayout::eGeneral || imageLayout == vk::ImageLayout::eSharedPresentKHR || imageLayout == vk::ImageLayout::eTransferDstOptimal;
        auto clearColor = reinterpret_cast<vk::ClearColorValue const&>(clearInfo->clearColor);
        auto clearValue = vk::ClearValue{ .color = clearColor };
        if (!isValidLayout) {
          this->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{ .cmdBuf = clearInfo->cmdBuf, .newImageLayout = vk::ImageLayout::eTransferDstOptimal, .queueFamilyIndex = clearInfo->queueFamilyIndex });
        };
        clearInfo->cmdBuf.clearColorImage(this->handle.as<vk::Image>(), isValidLayout ? imageLayout : vk::ImageLayout::eTransferDstOptimal, clearColor, std::vector<vk::ImageSubresourceRange>{subresourceRange});
        if (!isValidLayout) {
          this->writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo{ .cmdBuf = clearInfo->cmdBuf, .newImageLayout = imageLayout, .queueFamilyIndex = clearInfo->queueFamilyIndex });
        };
      };
    };

    //
    virtual void writeSwitchLayoutCommand(cpp21::const_wrap_arg<ImageLayoutSwitchWriteInfo> switchInfo) {
      if (this->cInfo->imageInfo && this->handle.type == HandleType::eImage) {
        //decltype(auto) info = switchInfo.info ? switchInfo.info : this->cInfo->imageInfo->info;
        decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
        decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
        decltype(auto) oldImageLayout = switchInfo->oldImageLayout ? switchInfo->oldImageLayout.value() : this->cInfo->imageInfo->layout;
        //decltype(auto) submission = CommandOnceSubmission{ .info = switchInfo.info };
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
            .subresourceRange = switchInfo->subresourceRange ? switchInfo->subresourceRange.value() : vk::ImageSubresourceRange{
              .aspectMask =
                this->cInfo->imageInfo->type == ImageType::eDepthStencilAttachment ? (vk::ImageAspectFlagBits::eDepth) :
                (this->cInfo->imageInfo->type == ImageType::eDepthAttachment ? vk::ImageAspectFlagBits::eDepth :
                (this->cInfo->imageInfo->type == ImageType::eStencilAttachment ? vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eColor)),
              .baseMipLevel = 0u,
              .levelCount = imageInfo->mipLevels,
              .baseArrayLayer = 0u,
              .layerCount = imageInfo->arrayLayers
            }
          }
        };

        switchInfo->cmdBuf.pipelineBarrier2(depInfo.setImageMemoryBarriers(transferBarrier));
        this->cInfo->imageInfo->layout = switchInfo->newImageLayout;
      };

      //return SFT();
    };

    //
    virtual FenceType executeSwitchLayoutOnce(cpp21::const_wrap_arg<ImageLayoutSwitchInfo> execInfo = {}) {
      // 
      decltype(auto) switchInfo = execInfo->switchInfo.value();
      decltype(auto) info = execInfo->info ? execInfo->info : this->cInfo->imageInfo->info;
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
      decltype(auto) oldImageLayout = switchInfo.oldImageLayout ? switchInfo.oldImageLayout.value() : this->cInfo->imageInfo->layout;
      decltype(auto) submission = CommandOnceSubmission{ .submission = execInfo->submission };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) correctAccessMask = vku::getCorrectAccessMaskByImageLayout<vk::AccessFlagBits2>(switchInfo.newImageLayout);

      //
      if (this->cInfo->imageInfo && this->handle.type == HandleType::eImage) {
        submission.commandInits.push_back([this, switchInfo](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
          this->writeSwitchLayoutCommand(switchInfo.with(cmdBuf));
          return cmdBuf;
        });

        //
        //this->cInfo->imageInfo->layout = switchInfo.newImageLayout;
        return deviceObj->executeCommandOnce(submission);
      };

      // 
      return FenceType{};
    };

  };

  //
  inline void DescriptorsObj::createNullImages() {
    //
    decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

    //
    decltype(auto) textureObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
      .descriptors = this->handle.as<vk::PipelineLayout>(),
      .imageInfo = ANAMED::ImageCreateInfo{
        .format = vk::Format::eR8G8B8A8Unorm,
        .extent = vk::Extent3D{2u, 2u, 1u},
        .type = ANAMED::ImageType::eTexture
      }
    });

    //
    decltype(auto) imageObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
      .descriptors = this->handle.as<vk::PipelineLayout>(),
      .imageInfo = ANAMED::ImageCreateInfo{
        .format = vk::Format::eR8G8B8A8Unorm,
        .extent = vk::Extent3D{2u, 2u, 1u},
        .type = ANAMED::ImageType::eStorage
      }
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

  //
  inline vk::Buffer& DescriptorsObj::createUniformBuffer() {
    this->uniformBuffer = (this->uniformBufferObj = ResourceObj::make(this->base, ResourceCreateInfo{
      .bufferInfo = BufferCreateInfo{
        .size = uniformSize,
        .type = BufferType::eUniform
      }
    })).as<vk::Buffer>();
    this->uniformBufferDesc = vk::DescriptorBufferInfo{ this->uniformBuffer, 0ull, this->uniformSize };
    return this->uniformBuffer;
  };

  // 
  inline WrapShared<DeviceObj> DeviceObj::writeCopyBuffersCommand(cpp21::const_wrap_arg<CopyBufferWriteInfo> copyInfoRaw) {
    //decltype(auto) submission = CommandOnceSubmission{ .info = QueueGetInfo {.queueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex } };
    decltype(auto) device = this->base.as<vk::Device>();
    decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
    decltype(auto) size = std::min(copyInfoRaw->src->region.size, copyInfoRaw->dst->region.size);
    decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = copyInfoRaw->src->buffer, .dstBuffer = copyInfoRaw->dst->buffer };
    decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
    decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{.srcOffset = copyInfoRaw->src->region.offset, .dstOffset = copyInfoRaw->dst->region.offset, .size = size} };

    //
    decltype(auto) srcAccessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(deviceObj->get<ResourceObj>(copyInfoRaw->src->buffer)->getBufferUsage()));
    decltype(auto) dstAccessMask = vk::AccessFlagBits2(vku::getAccessMaskByImageUsage(deviceObj->get<ResourceObj>(copyInfoRaw->dst->buffer)->getBufferUsage()));

    //
    decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(srcAccessMask),
        .srcAccessMask = srcAccessMask,
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
        .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
        .srcQueueFamilyIndex = copyInfoRaw->src->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw->dst->queueFamilyIndex,
        .buffer = copyInfoRaw->src->buffer,
        .offset = copyInfoRaw->src->region.offset,
        .size = size
      },
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(dstAccessMask),
        .srcAccessMask = dstAccessMask,
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
        .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
        .srcQueueFamilyIndex = copyInfoRaw->src->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw->dst->queueFamilyIndex,
        .buffer = copyInfoRaw->dst->buffer,
        .offset = copyInfoRaw->dst->region.offset,
        .size = size
      }
    };

    //
    decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
        .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(srcAccessMask),
        .dstAccessMask = srcAccessMask,
        .srcQueueFamilyIndex = copyInfoRaw->dst->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw->src->queueFamilyIndex,
        .buffer = copyInfoRaw->src->buffer,
        .offset = copyInfoRaw->src->region.offset,
        .size = size
      },
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
        .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(dstAccessMask),
        .dstAccessMask = dstAccessMask,
        .srcQueueFamilyIndex = copyInfoRaw->dst->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw->src->queueFamilyIndex,
        .buffer = copyInfoRaw->dst->buffer,
        .offset = copyInfoRaw->dst->region.offset,
        .size = size
      }
    };

    // 
    //submission.commandInits.push_back([=](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
      auto _copyInfo = copyInfo;
      auto _depInfo = depInfo;
      copyInfoRaw->cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
      copyInfoRaw->cmdBuf.copyBuffer2(_copyInfo.setRegions(regions));
      copyInfoRaw->cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
    //});

    //
    //return this->executeCommandOnce(submission);
    return this->SFT();
  };

  
};
#endif
