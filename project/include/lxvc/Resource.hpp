#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {

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

    // 
    //vk::Buffer buffer = {};
    //vk::Image image = {};
    //vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
    void* mappedMemory = nullptr;

    // 
    std::optional<AllocatedMemory> allocated = {};
    std::optional<ResourceCreateInfo> cInfo = {};
    std::optional<MemoryRequirements> mReqs = {};
    //std::shared_ptr<MSS> infoMap = {};

    //
    //std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

  public:
    // 
    ResourceObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    ResourceObj(Handle const& handle, std::optional<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : cInfo(cInfo) {
      this->construct(lxvc::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual tType registerSelf() {
      lxvc::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(Handle const& handle, std::optional<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
      auto shared = std::make_shared<ResourceObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  protected:

    //
    virtual AllocatedMemory& allocateMemory(cpp21::optional_ref<MemoryRequirements> requirements) {
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      auto& device = this->base.as<vk::Device>();
      auto& physicalDevice = deviceObj->physicalDevices[deviceObj->cInfo->physicalDeviceIndex];
      auto memTypeHeap = deviceObj->findMemoryTypeAndHeapIndex(physicalDevice, *requirements);
      auto& allocated = (this->allocated = AllocatedMemory{}).value();

      // 
      bool imageCondition = this->cInfo->imageInfo && this->cInfo->imageInfo->type != ImageType::eSwapchain;
      bool bufferCondition = true;

      // 
      allocated = AllocatedMemory{
        .memory = device.allocateMemory(infoMap->set(vk::StructureType::eMemoryAllocateInfo, vk::MemoryAllocateInfo{
          .pNext = infoMap->set(vk::StructureType::eMemoryAllocateFlagsInfo, vk::MemoryAllocateFlagsInfo{
            .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedAllocateInfo, vk::MemoryDedicatedAllocateInfo{
              .image = requirements->dedicated && this->handle.type == HandleType::eImage && imageCondition ? this->handle.as<vk::Image>() : vk::Image{},
              .buffer = requirements->dedicated && this->handle.type == HandleType::eBuffer && bufferCondition ? this->handle.as<vk::Buffer>() : vk::Buffer{}
            }),
            .flags = this->cInfo->bufferInfo->type == BufferType::eDevice ? vk::MemoryAllocateFlagBits::eDeviceAddress : vk::MemoryAllocateFlagBits{},
          }),
          .allocationSize = requirements->size,
          .memoryTypeIndex = std::get<0>(memTypeHeap)
        })),
        .offset = 0ull,
        .size = requirements->size
      };

      // 
      return allocated;
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) {
      try {
        this->base = deviceObj->handle;
        //this->deviceObj = deviceObj;
        this->cInfo = cInfo;
        this->infoMap = std::make_shared<MSS>();
      }
      catch (std::exception e) {
        std::cerr << "Unable to initialize ResourceObj itself" << std::endl; std::cerr << e.what() << std::endl;
      };

      // 
      if (this->cInfo) {
        if (this->cInfo->imageInfo) { try { this->createImage(this->cInfo->imageInfo.value()); } catch (std::exception e) { std::cerr << "Unable to initialize ResourceObj as Image" << std::endl; std::cerr << e.what() << std::endl; } };
        if (this->cInfo->bufferInfo) { try { this->createBuffer(this->cInfo->bufferInfo.value()); } catch (std::exception e) { std::cerr << "Unable to initialize ResourceObj as Buffer" << std::endl; std::cerr << e.what() << std::endl; } };
      };
    };

    // 
    virtual FenceType createImage(cpp21::optional_ref<ImageCreateInfo> cInfo = {}) {
      // 
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) imageUsage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
      decltype(auto) memoryUsage = MemoryUsage::eGpuOnly;

      // 
      switch (cInfo->type) {
      case ImageType::eSwapchain:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
        break;

      case ImageType::eStorage:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
        break;

      case ImageType::eTexture:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eSampled;
        break;

      case ImageType::eColorAttachment:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
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

      default:;
      };

      // 
      decltype(auto) imageInfo = infoMap->set(vk::StructureType::eImageCreateInfo, vk::ImageCreateInfo{
        .imageType = cInfo->imageType,
        .format = cInfo->format,
        .extent = cInfo->extent,
        .mipLevels = 1u,
        .arrayLayers = 1u, // TODO: correct array layers
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = imageUsage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
      });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{
          
        })
      });

      //
      device.getImageMemoryRequirements2(infoMap->set(vk::StructureType::eImageMemoryRequirementsInfo2, vk::ImageMemoryRequirementsInfo2{
        .image = (this->handle = this->cInfo->image ? this->cInfo->image.value() : device.createImage(imageInfo->setQueueFamilyIndices(deviceObj->queueFamilies.indices)))
      }).get(), memReqInfo2.get());

      //
      //lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());

      // 
      decltype(auto) memReqInfo = memReqInfo2->memoryRequirements;
      this->allocated = this->allocateMemory(this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .memoryTypeBits = memReqInfo.memoryTypeBits,
        .size = memReqInfo.size
      });

      //
      std::vector<vk::BindImageMemoryInfo> bindInfos = { *infoMap->set(vk::StructureType::eBindImageMemoryInfo, vk::BindImageMemoryInfo{
        .pNext = cInfo->swapchain ? infoMap->set(vk::StructureType::eBindImageMemorySwapchainInfoKHR, vk::BindImageMemorySwapchainInfoKHR{
          .swapchain = cInfo->swapchain->swapchain,
          .imageIndex = cInfo->swapchain->index
         }).get() : nullptr,
        .image = this->handle , .memory = cInfo->swapchain ? vk::DeviceMemory{} : this->allocated->memory, .memoryOffset = cInfo->swapchain ? 0ull : this->allocated->offset
      }) };
      device.bindImageMemory2(bindInfos);

      // 
      if (cInfo->info) {
        this->executeSwitchLayoutOnce(ImageLayoutSwitchInfo{
          .info = cInfo->info,
          .switchInfo = ImageLayoutSwitchWriteInfo{
            .newImageLayout = cInfo->layout,
            .oldImageLayout = imageInfo->initialLayout,
          },
        });
      };
    };

    //
    virtual tType writeSwitchLayoutCommand(ImageLayoutSwitchWriteInfo const switchInfo) {
      if (this->cInfo->imageInfo && this->handle.type == HandleType::eImage) {
        //decltype(auto) info = switchInfo.info ? switchInfo.info : this->cInfo->imageInfo->info;
        decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
        decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
        decltype(auto) oldImageLayout = switchInfo.oldImageLayout ? switchInfo.oldImageLayout.value() : this->cInfo->imageInfo->layout;
        //decltype(auto) submission = CommandOnceSubmission{ .info = switchInfo.info };
        decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
        decltype(auto) correctAccessMask = vku::getCorrectAccessMaskByImageLayout<vk::AccessFlagBits2>(switchInfo.newImageLayout);
        decltype(auto) transferBarrier = std::vector<vk::ImageMemoryBarrier2>{
          vk::ImageMemoryBarrier2{
            .srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
            .srcAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
            .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(correctAccessMask) | (correctAccessMask & vk::AccessFlagBits2::eShaderRead ? vk::PipelineStageFlagBits2::eAllCommands : vk::PipelineStageFlagBits2{}),
            .dstAccessMask = correctAccessMask,
            .oldLayout = oldImageLayout,
            .newLayout = switchInfo.newImageLayout,
            .srcQueueFamilyIndex = switchInfo.queueFamilyIndex,
            .dstQueueFamilyIndex = switchInfo.queueFamilyIndex,
            .image = this->handle.as<vk::Image>(),
            .subresourceRange = switchInfo.subresourceRange ? switchInfo.subresourceRange.value() : vk::ImageSubresourceRange{
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

        switchInfo.cmdBuf.pipelineBarrier2(depInfo.setImageMemoryBarriers(transferBarrier));
        this->cInfo->imageInfo->layout = switchInfo.newImageLayout;
      };
    };


    //
    virtual FenceType executeSwitchLayoutOnce(ImageLayoutSwitchInfo const& execInfo = {}) {
      // 
      decltype(auto) switchInfo = execInfo.switchInfo.value();
      decltype(auto) info = execInfo.info ? execInfo.info : this->cInfo->imageInfo->info;
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);
      decltype(auto) oldImageLayout = switchInfo.oldImageLayout ? switchInfo.oldImageLayout.value() : this->cInfo->imageInfo->layout;
      decltype(auto) submission = CommandOnceSubmission{ .submission = execInfo.submission };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) correctAccessMask = vku::getCorrectAccessMaskByImageLayout<vk::AccessFlagBits2>(switchInfo.newImageLayout);

      //
      if (this->cInfo->imageInfo && this->handle.type == HandleType::eImage) {
        submission.commandInits.push_back([=, this](vk::CommandBuffer const& cmdBuf) {
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

    // 
    virtual void createBuffer(cpp21::optional_ref<BufferCreateInfo> cInfo = {}) {
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) bufferUsage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
      decltype(auto) memoryUsage = MemoryUsage::eGpuOnly;

      // 
      switch (cInfo->type) {
      case BufferType::eDevice:
        memoryUsage = MemoryUsage::eGpuOnly;
        bufferUsage |=
          vk::BufferUsageFlagBits::eShaderDeviceAddress |
          vk::BufferUsageFlagBits::eStorageBuffer |
          vk::BufferUsageFlagBits::eIndexBuffer |
          vk::BufferUsageFlagBits::eVertexBuffer |
          vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT |
          vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT |
          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
          vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
          vk::BufferUsageFlagBits::eShaderBindingTableKHR |
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

      default:;
      };

      // 
      decltype(auto) bufferInfo = infoMap->set(vk::StructureType::eBufferCreateInfo, vk::BufferCreateInfo{
        .size = cInfo->size,
        .usage = bufferUsage,
        .sharingMode = vk::SharingMode::eExclusive
      });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{})
      });

      //
      device.getBufferMemoryRequirements2(infoMap->set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{
        .buffer = (this->handle = this->cInfo->buffer ? this->cInfo->buffer.value() : device.createBuffer(bufferInfo->setQueueFamilyIndices(deviceObj->queueFamilies.indices)))
      }).get(), memReqInfo2.get());

      //
      //lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());

      // 
      decltype(auto) memReqInfo = memReqInfo2->memoryRequirements;
      this->allocated = this->allocateMemory(this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .memoryTypeBits = memReqInfo.memoryTypeBits,
        .size = memReqInfo.size
      });

      //
      std::vector<vk::BindBufferMemoryInfo> bindInfos = { *infoMap->set(vk::StructureType::eBindBufferMemoryInfo, vk::BindBufferMemoryInfo{
        .buffer = this->handle, .memory = this->allocated->memory, .memoryOffset = this->allocated->offset
      }) };
      device.bindBufferMemory2(bindInfos);

      //
      if (cInfo->type == BufferType::eHostMap) {
        this->mappedMemory = device.mapMemory(this->allocated->memory, this->allocated->offset, memReqInfo.size);
      };

      // 
      //return this->SFT();
    };

  };

  // 
  inline WrapShared<DeviceObj> DeviceObj::writeCopyBuffersCommand(CopyBufferWriteInfo const& copyInfoRaw) {
    //decltype(auto) submission = CommandOnceSubmission{ .info = QueueGetInfo {.queueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex } };
    decltype(auto) device = this->base.as<vk::Device>();
    decltype(auto) size = std::min(copyInfoRaw.src->region.size, copyInfoRaw.dst->region.size);
    decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = copyInfoRaw.src->buffer, .dstBuffer = copyInfoRaw.dst->buffer };
    decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
    decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{.srcOffset = copyInfoRaw.src->region.offset, .dstOffset = copyInfoRaw.dst->region.offset, .size = size} };

    //
    decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
        .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
        .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
        .srcQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
        .buffer = copyInfoRaw.src->buffer,
        .offset = copyInfoRaw.src->region.offset,
        .size = size
      },
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralRead),
        .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralRead),
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
        .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
        .srcQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
        .buffer = copyInfoRaw.dst->buffer,
        .offset = copyInfoRaw.dst->region.offset,
        .size = size
      }
    };

    //
    decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
        .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
        .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
        .srcQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
        .buffer = copyInfoRaw.src->buffer,
        .offset = copyInfoRaw.src->region.offset,
        .size = size
      },
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
        .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralRead),
        .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralRead),
        .srcQueueFamilyIndex = copyInfoRaw.dst->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw.src->queueFamilyIndex,
        .buffer = copyInfoRaw.dst->buffer,
        .offset = copyInfoRaw.dst->region.offset,
        .size = size
      }
    };

    // 
    //submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
      auto _copyInfo = copyInfo;
      auto _depInfo = depInfo;
      copyInfoRaw.cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
      copyInfoRaw.cmdBuf.copyBuffer2(_copyInfo.setRegions(regions));
      copyInfoRaw.cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
    //});

    //
    //return this->executeCommandOnce(submission);
    return this->SFT();
  };

  
};
