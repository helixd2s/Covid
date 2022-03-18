#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"
#include "./Descriptors.hpp"
#include "./Framebuffer.hpp"
#include "./Swapchain.hpp"

// 
namespace ZNAMED {

  // 
  class PipelineObj : public BaseObj {
  public: 
    using tType = WrapShared<PipelineObj>;
    using BaseObj::BaseObj;
    
  protected:
    friend DeviceObj;
    friend FramebufferObj;
    friend DescriptorsObj;

    // 
    //vk::Pipeline pipeline = {};
    std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{};
    std::vector<vk::PipelineShaderStageCreateInfo> pipelineStages = {};
    std::vector<vk::DynamicState> dynamicStates = {};
    
    // 
    std::vector<vk::Viewport> viewports = {};
    std::vector<vk::Rect2D> scissors = {};
    //std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:
    // 
    PipelineObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->getHandle();
      this->construct(deviceObj, cInfo);
    };

    // 
    PipelineObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : cInfo(cInfo) {
      this->construct(ZNAMED::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    virtual tType registerSelf() {
      ZNAMED::context->get<DeviceObj>(this->base)->registerObj(this->handle, shared_from_this());
      return SFT();
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
      auto shared = std::make_shared<PipelineObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  protected:
    //
    virtual void createCompute(cpp21::const_wrap_arg<ComputePipelineCreateInfo> compute = {}) {
      decltype(auto) descriptors = ZNAMED::context->get<DeviceObj>(this->base)->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) crInfo = makeComputePipelineStageInfo(device, *compute->code); this->pipelineStages.push_back(crInfo);
      decltype(auto) cmInfo = infoMap->set(vk::StructureType::eComputePipelineCreateInfo, vk::ComputePipelineCreateInfo{
        .flags = vk::PipelineCreateFlags{},
        .stage = this->pipelineStages.back(),
        .layout = this->cInfo->layout
      });

      
      this->handle = std::move<vk::Pipeline>(device.createComputePipeline(descriptors->cache, cmInfo.ref()));
      //
      //ZNAMED::context->get(this->base)->registerObj(this->handle, shared_from_this());
      //return this->SFT();
    };

    //
    virtual void createGraphics(cpp21::const_wrap_arg<GraphicsPipelineCreateInfo> graphics = {}) {
      //this->pipeline = makeComputePipelineStageInfo(this->deviceObj->device, compute->code);
      //
      //ZNAMED::context->get(this->base)->registerObj(this->handle, shared_from_this());
      //return this->SFT();
      decltype(auto) descriptors = ZNAMED::context->get<DeviceObj>(this->base)->get<DescriptorsObj>(this->cInfo->layout);
      decltype(auto) device = this->base.as<vk::Device>();

      //
      viewports = { vk::Viewport{.x = 0.f, .y = 0.f, .width = 1.f, .height = 1.f, .minDepth = 0.f, .maxDepth = 1.f} };
      scissors = { vk::Rect2D{{0,0},{1u,1u}} };

      //
      decltype(auto) pRendering = infoMap->set(vk::StructureType::ePipelineRenderingCreateInfo, vk::PipelineRenderingCreateInfo{
        .viewMask = 0x0u,
        .depthAttachmentFormat = descriptors->cInfo->attachments[std::to_underlying(graphics->framebufferType)].depthAttachmentFormat,
        .stencilAttachmentFormat = descriptors->cInfo->attachments[std::to_underlying(graphics->framebufferType)].stencilAttachmentFormat
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
        .conservativeRasterizationMode = vk::ConservativeRasterizationModeEXT::eOverestimate
      });

      //
      decltype(auto) pRasterization = infoMap->set(vk::StructureType::ePipelineRasterizationStateCreateInfo, vk::PipelineRasterizationStateCreateInfo{
        .pNext = pRasterizationConservative.get(),
        .depthClampEnable = true,
        .rasterizerDiscardEnable = false,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = false
      });

      //
      decltype(auto) pMultisample = infoMap->set(vk::StructureType::ePipelineMultisampleStateCreateInfo, vk::PipelineMultisampleStateCreateInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = false,
        .minSampleShading = 0.f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = false,
        .alphaToOneEnable = false
      });

      //
      decltype(auto) pDepthStencil = infoMap->set(vk::StructureType::ePipelineDepthStencilStateCreateInfo, vk::PipelineDepthStencilStateCreateInfo{
        .depthTestEnable = true,
        .depthWriteEnable = true,
        .depthCompareOp = vk::CompareOp::eLessOrEqual,
        .depthBoundsTestEnable = false,
        .stencilTestEnable = false,
        .front = vk::StencilOpState{.failOp = vk::StencilOp::eKeep, .passOp = vk::StencilOp::eReplace, .compareOp = vk::CompareOp::eAlways },
        .back = vk::StencilOpState{.failOp = vk::StencilOp::eKeep, .passOp = vk::StencilOp::eReplace, .compareOp = vk::CompareOp::eAlways },
        .minDepthBounds = 0.f,
        .maxDepthBounds = 1.f
      });

      //
      decltype(auto) pColorBlend = infoMap->set(vk::StructureType::ePipelineColorBlendStateCreateInfo, vk::PipelineColorBlendStateCreateInfo{
        .logicOpEnable = false,
        .logicOp = vk::LogicOp::eCopy
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
        vk::DynamicState::eScissorWithCount, 
        vk::DynamicState::eViewportWithCount
      });

      // 
      decltype(auto) pInfo = infoMap->set(vk::StructureType::eGraphicsPipelineCreateInfo, vk::GraphicsPipelineCreateInfo{
        .pNext = &pRendering->setColorAttachmentFormats(descriptors->cInfo->attachments[std::to_underlying(graphics->framebufferType)].colorAttachmentFormats),
        .flags = vk::PipelineCreateFlags{},
        .pVertexInputState = pVertexInput.get(),
        .pInputAssemblyState = pInputAssembly.get(),
        .pTessellationState = pTessellation.get(),
        .pViewportState = pViewport.get(),//->setViewports(viewports).setScissors(scissors),
        .pRasterizationState = pRasterization.get(),
        .pMultisampleState = pMultisample.get(),
        .pDepthStencilState = pDepthStencil.get(),
        .pColorBlendState = &pColorBlend->setAttachments(descriptors->cInfo->attachments[std::to_underlying(graphics->framebufferType)].blendStates),
        .pDynamicState = &pDynamic->setDynamicStates(this->dynamicStates),
        .layout = this->cInfo->layout
      })->setStages(pipelineStages);

      //
      this->handle = std::move<vk::Pipeline>(device.createGraphicsPipeline(descriptors->cache, pInfo));
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
      this->base = deviceObj->getHandle();
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());
      if (this->cInfo->compute) { this->createCompute(this->cInfo->compute); };
      if (this->cInfo->graphics) { this->createGraphics(this->cInfo->graphics); };
      //return this->SFT();
    };

  public:

    // TODO: using multiple-command
    virtual tType writeComputeCommand(cpp21::const_wrap_arg<WriteComputeInfo> exec = WriteComputeInfo{}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(exec->layout ? exec->layout : this->cInfo->layout);
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      //
      decltype(auto) memoryBarriersBegin = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eComputeShaderReadWrite) | vk::PipelineStageFlagBits2::eComputeShader,
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eComputeShaderReadWrite)
        }
      };

      //
      decltype(auto) memoryBarriersEnd = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eComputeShaderReadWrite) | vk::PipelineStageFlagBits2::eComputeShader,
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eComputeShaderReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite)
        }
      };

      // 
      std::vector<uint32_t> offsets = {};

      {
        exec->cmdBuf.pipelineBarrier2(depInfo.setMemoryBarriers(memoryBarriersBegin));
        exec->cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eCompute, descriptorsObj->handle.as<vk::PipelineLayout>(), 0u, descriptorsObj->sets, offsets);
        exec->cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, this->handle.as<vk::Pipeline>());
        if (exec->instanceAddressBlock) {
          exec->cmdBuf.pushConstants(descriptorsObj->handle.as<vk::PipelineLayout>(), vk::ShaderStageFlagBits::eAll, 0u, sizeof(InstanceAddressBlock), &exec->instanceAddressBlock.value());
        };
        if (exec->swapchain) {
          exec->cmdBuf.pushConstants(descriptorsObj->handle.as<vk::PipelineLayout>(), vk::ShaderStageFlagBits::eAll, sizeof(InstanceAddressBlock) + sizeof(PushConstantData), sizeof(SwapchainStateInfo), &deviceObj->get<SwapchainObj>(exec->swapchain)->getStateInfo());
        };
        exec->cmdBuf.dispatch(exec->dispatch.width, exec->dispatch.height, exec->dispatch.depth);
        exec->cmdBuf.pipelineBarrier2(depInfo.setMemoryBarriers(memoryBarriersEnd));
      };

      return SFT();
    };

    // TODO: using multiple-command
    virtual tType writeGraphicsCommand(cpp21::const_wrap_arg<WriteGraphicsInfo> exec = WriteGraphicsInfo{}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(exec->layout ? exec->layout : this->cInfo->layout);
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) framebuffer = deviceObj->get<FramebufferObj>(exec->framebuffer).shared();

      //
      decltype(auto) memoryBarriersBegin = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGraphicsShaderReadWrite) | vk::PipelineStageFlagBits2::eAllCommands,
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGraphicsShaderReadWrite)
        }
      };

      //
      decltype(auto) memoryBarriersEnd = std::vector<vk::MemoryBarrier2>{
        vk::MemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGraphicsShaderReadWrite) | vk::PipelineStageFlagBits2::eAllCommands,
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGraphicsShaderReadWrite),
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite)
        }
      };

      //
      decltype(auto) depthAttachment = framebuffer->getDepthAttachment();
      decltype(auto) stencilAttachment = framebuffer->getStencilAttachment();
      decltype(auto) colorAttachments = framebuffer->getColorAttachments();
      decltype(auto) renderArea = framebuffer->getRenderArea();

      // 
      const bool supportMultiDraw = deviceObj->getPhysicalDeviceInfoMap()->get<vk::PhysicalDeviceMultiDrawFeaturesEXT>(vk::StructureType::ePhysicalDeviceMultiDrawFeaturesEXT)->multiDraw;

      //
      viewports = { vk::Viewport{.x = 0.f, .y = 0.f, .width = float(renderArea.extent.width), .height = float(renderArea.extent.height), .minDepth = 0.f, .maxDepth = 1.f} };
      scissors = { renderArea };

      // 
      std::vector<uint32_t> offsets = {};

      // 
      auto _depInfo = depInfo;
      if (framebuffer) { framebuffer->writeSwitchToAttachment(exec->cmdBuf); };
      exec->cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersBegin));
      exec->cmdBuf.beginRendering(vk::RenderingInfoKHR{ .renderArea = renderArea, .layerCount = this->cInfo->graphics->framebufferType == FramebufferType::eCubemap ? 6u : 1u, .viewMask = 0x0u, .colorAttachmentCount = uint32_t(colorAttachments.size()), .pColorAttachments = colorAttachments.data(), .pDepthAttachment = &depthAttachment, .pStencilAttachment = &stencilAttachment });
      exec->cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, this->handle.as<vk::Pipeline>());
      exec->cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, descriptorsObj->handle.as<vk::PipelineLayout>(), 0u, descriptorsObj->sets, offsets);
      exec->cmdBuf.setViewportWithCount(viewports);
      exec->cmdBuf.setScissorWithCount(scissors);

      //
      using fnT = void(cpp21::shared_vector<vk::MultiDrawInfoEXT> const&);
      using fnTp = std::function<fnT>;

      //
      if (exec->instanceDraws.size() > 0) {
        for (decltype(auto) instInfo : exec->instanceDraws) {
          decltype(auto) multiDrawDirect = supportMultiDraw ?
            fnTp([cmdBuf = exec->cmdBuf, dispatch = deviceObj->getDispatch()](cpp21::shared_vector<vk::MultiDrawInfoEXT> const& multiDraw) {
              cmdBuf.drawMultiEXT(*multiDraw, 1u, 0u, sizeof(vk::MultiDrawInfoEXT), dispatch);
            }) :
            fnTp([cmdBuf = exec->cmdBuf](cpp21::shared_vector<vk::MultiDrawInfoEXT> const& multiDraw) {
              for (decltype(auto) drawInfo : *multiDraw) {
                cmdBuf.draw(drawInfo.vertexCount, 1u, drawInfo.firstVertex, 0u);
              };
            });

            // 
            if (exec->instanceAddressBlock) {
              exec->cmdBuf.pushConstants(descriptorsObj->handle.as<vk::PipelineLayout>(), vk::ShaderStageFlagBits::eAll, 0u, sizeof(InstanceAddressBlock), &exec->instanceAddressBlock.value());
            };
            if (instInfo.drawConst) {
              exec->cmdBuf.pushConstants(descriptorsObj->handle.as<vk::PipelineLayout>(), vk::ShaderStageFlagBits::eAll, sizeof(InstanceAddressBlock), sizeof(PushConstantData), &instInfo.drawConst.value());
            };
            if (exec->swapchain) {
              exec->cmdBuf.pushConstants(descriptorsObj->handle.as<vk::PipelineLayout>(), vk::ShaderStageFlagBits::eAll, sizeof(InstanceAddressBlock) + sizeof(PushConstantData), sizeof(SwapchainStateInfo), &deviceObj->get<SwapchainObj>(exec->swapchain)->getStateInfo());
            };
            multiDrawDirect(instInfo.drawInfos);
        };
      };

      // 
      exec->cmdBuf.endRendering();
      exec->cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersEnd));
      if (framebuffer) { framebuffer->writeSwitchToShaderRead(exec->cmdBuf); };

      // 
      return SFT();
    };

    // TODO: using multiple-command
    virtual FenceType executePipelineOnce(cpp21::const_wrap_arg<ExecutePipelineInfo> exec = ExecutePipelineInfo{}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      decltype(auto) submission = CommandOnceSubmission{ .submission = exec->submission };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      // 
      submission.commandInits.push_back([exec,this](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        if (exec->graphics) { this->writeGraphicsCommand(exec->graphics->with(cmdBuf)); };
        if (exec->compute) { this->writeComputeCommand(exec->compute->with(cmdBuf)); };
        return cmdBuf;
      });

      //
      return deviceObj->executeCommandOnce(submission);
    };

  };
  
};
