#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"

// 
namespace lxvc {

  // 
  class ResourceObj : public BaseObj {
  public: 
    using tType = std::shared_ptr<ResourceObj>;
    using BaseObj::BaseObj;

  protected:
    //using BaseObj;
    friend DeviceObj;
    friend DescriptorsObj;
    friend UploaderObj;

    // 
    //vk::Buffer buffer = {};
    //vk::Image image = {};
    vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;
    void* mappedMemory = nullptr;

    // 
    std::optional<AllocatedMemory> allocated = {};
    std::optional<ResourceCreateInfo> cInfo = {};
    std::optional<MemoryRequirements> mReqs = {};
    //std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return std::dynamic_pointer_cast<std::decay_t<decltype(*this)>>(shared_from_this()); };
    inline decltype(auto) SFT() const { return std::dynamic_pointer_cast<const std::decay_t<decltype(*this)>>(shared_from_this()); };

  public:
    // 
    ResourceObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<ResourceCreateInfo> cInfo = ResourceCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
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

  protected:

    //
    virtual std::optional<AllocatedMemory>& allocateMemory(cpp21::optional_ref<MemoryRequirements> requirements) {
      decltype(auto) physicalDevice = this->deviceObj->physicalDevices[this->deviceObj->cInfo->physicalDeviceIndex];
      decltype(auto) memTypeHeap = this->deviceObj->findMemoryTypeAndHeapIndex(physicalDevice, *requirements);
      decltype(auto) allocated = this->allocated;
      decltype(auto) device = this->base.as<vk::Device>();

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
    virtual void createImage(cpp21::optional_ref<ImageCreateInfo> cInfo = {}) {
      // 
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
        .usage = imageUsage
      });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{})
      });

      //
      device.getImageMemoryRequirements2(infoMap->set(vk::StructureType::eImageMemoryRequirementsInfo2, vk::ImageMemoryRequirementsInfo2{
        .image = (this->handle = device.createImage(imageInfo->setQueueFamilyIndices(this->deviceObj->queueFamilies.indices)))
      }).get(), memReqInfo2.get());

      //
      lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());

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
      //return this->SFT();
    };

    // 
    virtual void createBuffer(cpp21::optional_ref<BufferCreateInfo> cInfo = {}) {
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
        .usage = bufferUsage
      });

      // 
      decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
        .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{})
      });

      //
      device.getBufferMemoryRequirements2(infoMap->set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{
        .buffer = (this->handle = device.createBuffer(bufferInfo->setQueueFamilyIndices(this->deviceObj->queueFamilies.indices)))
      }).get(), memReqInfo2.get());

      //
      lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());

      // 
      decltype(auto) memReqInfo = memReqInfo2->memoryRequirements;
      this->allocated = this->allocateMemory(opt_ref((this->mReqs = MemoryRequirements{
        .memoryUsage = memoryUsage,
        .memoryTypeBits = memReqInfo.memoryTypeBits,
        .size = memReqInfo.size
      }).value()));

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
    decltype(auto) submission = CommandOnceSubmission{ .info = copyInfoRaw->info };
    decltype(auto) device = this->base.as<vk::Device>();
    decltype(auto) size = std::min(copyInfoRaw->src->region.size, copyInfoRaw->dst->region.size);
    decltype(auto) copyInfo = vk::CopyBufferInfo2{ .srcBuffer = copyInfoRaw->src->buffer, .dstBuffer = copyInfoRaw->dst->buffer };
    decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
    decltype(auto) regions = std::vector<vk::BufferCopy2>{ vk::BufferCopy2{.srcOffset = copyInfoRaw->src->region.offset, .dstOffset = copyInfoRaw->dst->region.offset, .size = size} };

    //
    decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
      vk::BufferMemoryBarrier2{
        .srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
        .srcAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eShaderWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
        .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead,
        .buffer = copyInfoRaw->src->buffer,
        .offset = copyInfoRaw->src->region.offset,
        .size = size
      },
      vk::BufferMemoryBarrier2{
        .srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
        .srcAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eShaderRead,
        .dstStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
        .dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
        .buffer = copyInfoRaw->dst->buffer,
        .offset = copyInfoRaw->dst->region.offset,
        .size = size
      }
    };

    //
    decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
      vk::BufferMemoryBarrier2{
        .srcStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
        .srcAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead,
        .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
        .dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eShaderWrite,
        .buffer = copyInfoRaw->src->buffer,
        .offset = copyInfoRaw->src->region.offset,
        .size = size
      },
      vk::BufferMemoryBarrier2{
        .srcStageMask = vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eTransfer,
        .srcAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eTransferWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
        .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eTransferRead | vk::AccessFlagBits2::eShaderRead,
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
