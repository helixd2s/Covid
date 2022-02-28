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
      return std::make_shared<ResourceObj>(handle, cInfo)->registerSelf();
    };

  protected:

    //
    virtual AllocatedMemory& allocateMemory(cpp21::optional_ref<MemoryRequirements> requirements) {
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) physicalDevice = deviceObj->physicalDevices[deviceObj->cInfo->physicalDeviceIndex];
      decltype(auto) memTypeHeap = deviceObj->findMemoryTypeAndHeapIndex(physicalDevice, *requirements);
      decltype(auto) allocated = cpp21::opt_ref<AllocatedMemory>((this->allocated = AllocatedMemory{}).value());

      // 
      allocated = AllocatedMemory{
        .memory = device.allocateMemory(infoMap->set(vk::StructureType::eMemoryAllocateInfo, vk::MemoryAllocateInfo{
          .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedAllocateInfo, vk::MemoryDedicatedAllocateInfo{ 
            .image = requirements->dedicated && this->handle.type == HandleType::eImage ? this->handle.as<vk::Image>() : vk::Image{},
            .buffer = requirements->dedicated && this->handle.type == HandleType::eBuffer ? this->handle.as<vk::Buffer>() : vk::Buffer{}
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
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();
      if (this->cInfo->imageInfo) { this->createImage(this->cInfo->imageInfo.value()); };
      if (this->cInfo->bufferInfo) { this->createBuffer(this->cInfo->bufferInfo.value()); };
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

      case ImageType::eStencilAttachment:
        memoryUsage = MemoryUsage::eGpuOnly;
        imageUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
        break;

      default:;
      };

      // 
      decltype(auto) imageInfo = infoMap->set(vk::StructureType::eImageCreateInfo, vk::ImageCreateInfo{
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
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{})
      });

      //
      device.getImageMemoryRequirements2(infoMap->set(vk::StructureType::eImageMemoryRequirementsInfo2, vk::ImageMemoryRequirementsInfo2{
        .image = (this->handle = device.createImage(imageInfo->setQueueFamilyIndices(deviceObj->queueFamilies.indices)))
      }).get(), memReqInfo2.get());

      //
      //lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());

      // 
      decltype(auto) memReqInfo = memReqInfo2->memoryRequirements;
      this->allocated = this->allocateMemory(opt_ref((this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .memoryTypeBits = memReqInfo.memoryTypeBits,
        .size = memReqInfo.size
      }).value()));

      //
      std::vector<vk::BindImageMemoryInfo> bindInfos = { *infoMap->set(vk::StructureType::eBindImageMemoryInfo, vk::BindImageMemoryInfo{
        .image = this->handle , .memory = this->allocated->memory, .memoryOffset = this->allocated->offset
      }) };
      device.bindImageMemory2(bindInfos);

      // 
      decltype(auto) submission = CommandOnceSubmission{ .info = cInfo->info };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) correctAccessMask = vku::getCorrectAccessMaskByImageLayout<vk::AccessFlagBits2>(cInfo->layout);
      decltype(auto) transferBarrier = std::vector<vk::ImageMemoryBarrier2>{
        vk::ImageMemoryBarrier2{
          .srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
          .srcAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(correctAccessMask),
          .dstAccessMask = correctAccessMask,
          .oldLayout = imageInfo->initialLayout,
          .newLayout = cInfo->layout,
          .image = this->handle.as<vk::Image>(),
          .subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0u,
            .levelCount = imageInfo->mipLevels,
            .baseArrayLayer = 0u,
            .layerCount = imageInfo->arrayLayers
          }
        }
      };

      // 
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _depInfo = depInfo;
        cmdBuf.pipelineBarrier2(_depInfo.setImageMemoryBarriers(transferBarrier));
        return cmdBuf;
      });

      //
      return deviceObj->executeCommandOnce(submission);
      // 
      //return this->SFT();
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
        .buffer = (this->handle = device.createBuffer(bufferInfo->setQueueFamilyIndices(deviceObj->queueFamilies.indices)))
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
  inline FenceType DeviceObj::copyBuffers(cpp21::optional_ref<CopyBufferInfo> copyInfoRaw) {
    decltype(auto) submission = CommandOnceSubmission{ .info = QueueGetInfo {.queueFamilyIndex = copyInfoRaw->dst->queueFamilyIndex } };
    decltype(auto) device = this->base.as<vk::Device>();
    decltype(auto) size = std::min(copyInfoRaw->src->region.size, copyInfoRaw->dst->region.size);
    decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = copyInfoRaw->src->buffer, .dstBuffer = copyInfoRaw->dst->buffer };
    decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
    decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{.srcOffset = copyInfoRaw->src->region.offset, .dstOffset = copyInfoRaw->dst->region.offset, .size = size} };

    //
    decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
        .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferRead),
        .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferRead),
        .srcQueueFamilyIndex = copyInfoRaw->src->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw->dst->queueFamilyIndex,
        .buffer = copyInfoRaw->src->buffer,
        .offset = copyInfoRaw->src->region.offset,
        .size = size
      },
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralRead),
        .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralRead),
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
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
        .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
        .srcQueueFamilyIndex = copyInfoRaw->dst->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw->src->queueFamilyIndex,
        .buffer = copyInfoRaw->src->buffer,
        .offset = copyInfoRaw->src->region.offset,
        .size = size
      },
      vk::BufferMemoryBarrier2{
        .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
        .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
        .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralRead),
        .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralRead),
        .srcQueueFamilyIndex = copyInfoRaw->dst->queueFamilyIndex,
        .dstQueueFamilyIndex = copyInfoRaw->src->queueFamilyIndex,
        .buffer = copyInfoRaw->dst->buffer,
        .offset = copyInfoRaw->dst->region.offset,
        .size = size
      }
    };

    // 
    submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
      auto _copyInfo = copyInfo;
      auto _depInfo = depInfo;
      cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
      cmdBuf.copyBuffer2(_copyInfo.setRegions(regions));
      cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
      return cmdBuf;
    });

    //
    return this->executeCommandOnce(submission);
  };

  
};
