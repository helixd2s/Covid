#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"
#include "./Descriptors.hpp"

// 
namespace lxvc {

  //
  struct AttachmentsInfo {
    vk::Format depthAttachmentFormat = vk::Format::eD32Sfloat;
    vk::Format stencilAttachmentFormat = vk::Format::eS8Uint;
    std::vector<vk::Format> colorAttachmentFormats = {};
    std::vector<vk::PipelineColorBlendAttachmentState> blendStates = {};
  };

  // 
  class PipelineObj : public BaseObj {
  public: 
    using tType = WrapShared<PipelineObj>;
    using BaseObj::BaseObj;
    
  protected:
    friend DeviceObj;

    // 
    //vk::Pipeline pipeline = {};
    std::optional<PipelineCreateInfo> cInfo = {};
    std::vector<vk::PipelineShaderStageCreateInfo> pipelineStages = {};
    std::vector<vk::DynamicState> dynamicStates = {};
    
    //std::vector<vk::Viewport> viewports = {};
    //std::vector<vk::Rect2D> scissors = {};
    //std::shared_ptr<DeviceObj> deviceObj = {};

    AttachmentsInfo attachments = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

  public:
    // 
    PipelineObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    PipelineObj(Handle const& handle, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(Handle const& handle, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
      return std::make_shared<PipelineObj>(handle, cInfo)->registerSelf();
    };

  protected:
    //
    virtual void createCompute(cpp21::optional_ref<ComputePipelineCreateInfo> compute = {}) {
      decltype(auto) descriptors = lxvc::context->get<DeviceObj>(this->base)->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) device = this->base.as<vk::Device>();
      this->pipelineStages.push_back(makeComputePipelineStageInfo(device, *compute->code));
      this->handle = std::move<vk::Pipeline>(device.createComputePipeline(descriptors->cache, infoMap->set(vk::StructureType::eComputePipelineCreateInfo, vk::ComputePipelineCreateInfo{
        .flags = vk::PipelineCreateFlags{},
        .stage = this->pipelineStages.back(),
        .layout = this->cInfo->layout
      }).ref()));
      //
      //lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());
      //return this->SFT();
    };

    //
    virtual void createGraphics(cpp21::optional_ref<GraphicsPipelineCreateInfo> graphics = {}) {
      //this->pipeline = makeComputePipelineStageInfo(this->deviceObj->device, compute->code);
      //
      //lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());
      //return this->SFT();
      decltype(auto) descriptors = lxvc::context->get<DeviceObj>(this->base)->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) device = this->base.as<vk::Device>();

      //
      decltype(auto) pRendering = infoMap->set(vk::StructureType::ePipelineRenderingCreateInfo, vk::PipelineRenderingCreateInfo{
        .depthAttachmentFormat = vk::Format::eD32Sfloat,
        .stencilAttachmentFormat = vk::Format::eS8Uint
      });

      //
      decltype(auto) pVertexInput = infoMap->set(vk::StructureType::ePipelineVertexInputStateCreateInfo, vk::PipelineVertexInputStateCreateInfo{
        
      });

      //
      decltype(auto) pInputAssembly = infoMap->set(vk::StructureType::ePipelineInputAssemblyStateCreateInfo, vk::PipelineInputAssemblyStateCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList
      });

      //
      decltype(auto) pTessellation = infoMap->set(vk::StructureType::ePipelineTessellationStateCreateInfo, vk::PipelineTessellationStateCreateInfo{
        .patchControlPoints = 1u
      });

      //
      decltype(auto) pViewport = infoMap->set(vk::StructureType::ePipelineViewportStateCreateInfo, vk::PipelineViewportStateCreateInfo{

      });

      //
      decltype(auto) pRasterizationConservative = infoMap->set(vk::StructureType::ePipelineRasterizationConservativeStateCreateInfoEXT, vk::PipelineRasterizationConservativeStateCreateInfoEXT{

      });

      //
      decltype(auto) pRasterization = infoMap->set(vk::StructureType::ePipelineRasterizationStateCreateInfo, vk::PipelineRasterizationStateCreateInfo{
        .pNext = pRasterizationConservative.get(),
        .depthClampEnable = true,
        .rasterizerDiscardEnable = true,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eNone,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = false
      });

      //
      decltype(auto) pMultisample = infoMap->set(vk::StructureType::ePipelineMultisampleStateCreateInfo, vk::PipelineMultisampleStateCreateInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = false
      });

      //
      decltype(auto) pDepthStencil = infoMap->set(vk::StructureType::ePipelineDepthStencilStateCreateInfo, vk::PipelineDepthStencilStateCreateInfo{
        .depthTestEnable = true,
        .depthWriteEnable = true,
        .depthCompareOp = vk::CompareOp::eLessOrEqual,
        .depthBoundsTestEnable = true,
        .stencilTestEnable = false,
        .minDepthBounds = 0.f,
        .maxDepthBounds = 1.f
      });

      //
      decltype(auto) pColorBlend = infoMap->set(vk::StructureType::ePipelineColorBlendStateCreateInfo, vk::PipelineColorBlendStateCreateInfo{
        .logicOpEnable = false
      });

      //
      decltype(auto) pDynamic = infoMap->set(vk::StructureType::ePipelineDynamicStateCreateInfo, vk::PipelineDynamicStateCreateInfo{

      });

      //
      for (decltype(auto) pair : graphics->stageCodes) {
        pipelineStages.push_back(makePipelineStageInfo(device, *pair.second, pair.first, "main"));
      };

      //
      this->dynamicStates.insert(dynamicStates.end(), {
        vk::DynamicState::eViewport, 
        vk::DynamicState::eScissor, 
        vk::DynamicState::eScissorWithCount, 
        vk::DynamicState::eViewportWithCount, 
        vk::DynamicState::eVertexInputBindingStride,
        vk::DynamicState::eVertexInputEXT,
        vk::DynamicState::eCullMode,
        vk::DynamicState::eBlendConstants,
        vk::DynamicState::ePrimitiveTopology
      });

      // 
      this->attachments.colorAttachmentFormats.insert(attachments.colorAttachmentFormats.end(), {vk::Format::eR32G32B32A32Sfloat, vk::Format::eR32G32B32A32Sfloat });

      // for 1st image of framebuffer
      this->attachments.blendStates.push_back(vk::PipelineColorBlendAttachmentState{
        .blendEnable = false,
        .srcColorBlendFactor = vk::BlendFactor::eOneMinusDstAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eDstAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eOne,
        .alphaBlendOp = vk::BlendOp::eMax
      });

      // for 2st image of framebuffer
      this->attachments.blendStates.push_back(vk::PipelineColorBlendAttachmentState{
        .blendEnable = false,
        .srcColorBlendFactor = vk::BlendFactor::eOneMinusDstAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eDstAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eOne,
        .alphaBlendOp = vk::BlendOp::eMax
      });

      // 
      decltype(auto) pInfo = infoMap->set(vk::StructureType::eGraphicsPipelineCreateInfo, vk::GraphicsPipelineCreateInfo{
        .pNext = &pRendering->setColorAttachmentFormats(this->attachments.colorAttachmentFormats),
        .flags = vk::PipelineCreateFlags{},
        .pVertexInputState = pVertexInput.get(),
        .pInputAssemblyState = pInputAssembly.get(),
        .pTessellationState = pTessellation.get(),
        .pViewportState = pViewport.get(),
        .pRasterizationState = pRasterization.get(),
        .pMultisampleState = pMultisample.get(),
        .pDepthStencilState = pDepthStencil.get(),
        .pColorBlendState = &pColorBlend->setAttachments(this->attachments.blendStates),
        .pDynamicState = &pDynamic->setDynamicStates(this->dynamicStates),
        .layout = this->cInfo->layout
      });

      //
      this->handle = std::move<vk::Pipeline>(device.createGraphicsPipeline(descriptors->cache, pInfo->setStages(pipelineStages)));
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();
      if (this->cInfo->compute) { this->createCompute(this->cInfo->compute); };
      if (this->cInfo->graphics) { this->createGraphics(this->cInfo->graphics); };
      //return this->SFT();
    };

  public:
    // TODO: using multiple-command
    virtual FenceType executeComputeOnce(std::optional<ExecuteComputeInfo> exec = ExecuteComputeInfo{}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(exec->layout ? exec->layout : this->cInfo->layout);
      decltype(auto) submission = CommandOnceSubmission{ .info = exec->info };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      //
      decltype(auto) memoryBarriersBegin = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eShaderReadWrite) | vk::PipelineStageFlagBits2::eComputeShader,
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite)
        }
      };

      //
      decltype(auto) memoryBarriersEnd = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eShaderReadWrite) | vk::PipelineStageFlagBits2::eComputeShader,
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eShaderReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite)
        }
      };

      // 
      std::vector<uint32_t> offsets = {};
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _depInfo = depInfo;
        cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersBegin));
        cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eCompute, descriptorsObj->handle.as<vk::PipelineLayout>(), 0u, descriptorsObj->sets, offsets);
        cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, this->handle.as<vk::Pipeline>());
        cmdBuf.dispatch(exec->dispatch.width, exec->dispatch.height, exec->dispatch.depth);
        cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersEnd));
        return cmdBuf;
      });

      //
      return deviceObj->executeCommandOnce(submission);
    };

  };
  
};
