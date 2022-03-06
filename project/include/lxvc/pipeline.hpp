#pragma once

// 
#include "./Core.hpp"
#include "./Device.hpp"
#include "./Descriptors.hpp"
#include "./Framebuffer.hpp"

// 
namespace lxvc {

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
    std::optional<PipelineCreateInfo> cInfo = {};
    std::vector<vk::PipelineShaderStageCreateInfo> pipelineStages = {};
    std::vector<vk::DynamicState> dynamicStates = {};
    
    // 
    std::vector<vk::Viewport> viewports = {};
    std::vector<vk::Rect2D> scissors = {};
    //std::shared_ptr<DeviceObj> deviceObj = {};

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
      auto shared = std::make_shared<PipelineObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
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
      viewports = { vk::Viewport{.x = 0.f, .y = 0.f, .width = 1.f, .height = 1.f, .minDepth = 0.f, .maxDepth = 1.f} };
      scissors = { vk::Rect2D{{0,0},{1u,1u}} };

      //
      decltype(auto) pRendering = infoMap->set(vk::StructureType::ePipelineRenderingCreateInfo, vk::PipelineRenderingCreateInfo{
        .viewMask = 0x0u,
        .depthAttachmentFormat = descriptors->cInfo->attachments.depthAttachmentFormat,
        .stencilAttachmentFormat = descriptors->cInfo->attachments.stencilAttachmentFormat
      });

      //
      decltype(auto) pVertexInput = infoMap->set(vk::StructureType::ePipelineVertexInputStateCreateInfo, vk::PipelineVertexInputStateCreateInfo{
        
      });

      //
      decltype(auto) pInputAssembly = infoMap->set(vk::StructureType::ePipelineInputAssemblyStateCreateInfo, vk::PipelineInputAssemblyStateCreateInfo{
        .topology = vk::PrimitiveTopology::eTriangleList
      });

      //
      //decltype(auto) pTessellation = infoMap->set(vk::StructureType::ePipelineTessellationStateCreateInfo, vk::PipelineTessellationStateCreateInfo{
        //.patchControlPoints = 1u
      //});

      //
      decltype(auto) pViewport = infoMap->set(vk::StructureType::ePipelineViewportStateCreateInfo, vk::PipelineViewportStateCreateInfo{

      });

      //
      //decltype(auto) pRasterizationConservative = infoMap->set(vk::StructureType::ePipelineRasterizationConservativeStateCreateInfoEXT, vk::PipelineRasterizationConservativeStateCreateInfoEXT{
        //.conservativeRasterizationMode = vk::ConservativeRasterizationModeEXT::eOverestimate
      //});

      //
      decltype(auto) pRasterization = infoMap->set(vk::StructureType::ePipelineRasterizationStateCreateInfo, vk::PipelineRasterizationStateCreateInfo{
        //.pNext = pRasterizationConservative.get(),
        .depthClampEnable = false,
        .rasterizerDiscardEnable = false,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eNone,
        .frontFace = vk::FrontFace::eCounterClockwise,
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
        .depthTestEnable = false,
        .depthWriteEnable = true,
        .depthCompareOp = vk::CompareOp::eAlways,
        .depthBoundsTestEnable = false,
        .stencilTestEnable = false,
        .front = vk::StencilOpState{.failOp = vk::StencilOp::eKeep, .passOp = vk::StencilOp::eReplace, .compareOp = vk::CompareOp::eAlways },
        .back = vk::StencilOpState{.failOp = vk::StencilOp::eKeep, .passOp = vk::StencilOp::eReplace, .compareOp = vk::CompareOp::eAlways },
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
        vk::DynamicState::eScissorWithCount, 
        vk::DynamicState::eViewportWithCount
      });

      // 
      decltype(auto) pInfo = infoMap->set(vk::StructureType::eGraphicsPipelineCreateInfo, vk::GraphicsPipelineCreateInfo{
        .pNext = &pRendering->setColorAttachmentFormats(descriptors->cInfo->attachments.colorAttachmentFormats),
        .flags = vk::PipelineCreateFlags{},
        .pVertexInputState = pVertexInput.get(),
        .pInputAssemblyState = pInputAssembly.get(),
        //.pTessellationState = pTessellation.get(),
        .pViewportState = pViewport.get(),//->setViewports(viewports).setScissors(scissors),
        .pRasterizationState = pRasterization.get(),
        .pMultisampleState = pMultisample.get(),
        .pDepthStencilState = pDepthStencil.get(),
        .pColorBlendState = &pColorBlend->setAttachments(descriptors->cInfo->attachments.blendStates),
        .pDynamicState = &pDynamic->setDynamicStates(this->dynamicStates),
        .layout = this->cInfo->layout
      })->setStages(pipelineStages);

      //
      this->handle = std::move<vk::Pipeline>(device.createGraphicsPipeline(descriptors->cache, pInfo));
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
      decltype(auto) submission = CommandOnceSubmission{ .info = exec->info, .waitSemaphores = exec->waitSemaphores, .signalSemaphores = exec->signalSemaphores };
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

    // TODO: using multiple-command
    virtual FenceType executeGraphicsOnce(std::optional<ExecuteGraphicsInfo> exec = ExecuteGraphicsInfo{}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = lxvc::context->get<DeviceObj>(this->base);
      decltype(auto) descriptorsObj = deviceObj->get<DescriptorsObj>(exec->layout ? exec->layout : this->cInfo->layout);
      decltype(auto) submission = CommandOnceSubmission{ .info = exec->info, .waitSemaphores = exec->waitSemaphores, .signalSemaphores = exec->signalSemaphores };
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
      decltype(auto) framebuffers = deviceObj->get<FramebufferObj>(exec->framebuffer);

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
      decltype(auto) depthAttachment = framebuffers->getDepthAttachment();
      decltype(auto) stencilAttachment = framebuffers->getStencilAttachment();
      decltype(auto) colorAttachments = framebuffers->getColorAttachments();
      decltype(auto) renderArea = framebuffers->getRenderArea();

      //
      viewports = { vk::Viewport{ .x = 0.f, .y = 0.f, .width = float(renderArea.extent.width), .height = float(renderArea.extent.height), .minDepth = 0.f, .maxDepth = 1.f} };
      scissors = { renderArea };

      // 
      std::vector<uint32_t> offsets = {};
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _depInfo = depInfo;
        cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersBegin));
        cmdBuf.beginRendering(vk::RenderingInfoKHR{ .renderArea = renderArea, .layerCount = 1u, .viewMask = 0x0u, .colorAttachmentCount = uint32_t(colorAttachments.size()), .pColorAttachments = colorAttachments.data(), .pDepthAttachment = &depthAttachment, .pStencilAttachment = &stencilAttachment });
        cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, this->handle.as<vk::Pipeline>());
        cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, descriptorsObj->handle.as<vk::PipelineLayout>(), 0u, descriptorsObj->sets, offsets);
        cmdBuf.setViewportWithCount(viewports);
        cmdBuf.setScissorWithCount(scissors);
        decltype(auto) catchFn = [&, this](std::optional<std::exception> e = {}) {
          //std::cerr << "Failed to MultiDraw or not supported, trying to reform command..." << std::endl;
          if (e) {
            std::cerr << e->what() << std::endl;
          };
          for (auto drawInfo : exec->multiDrawInfo) {
            cmdBuf.draw(drawInfo.vertexCount, 1u, drawInfo.firstVertex, 0u);
          };
        };

        if (deviceObj->dispatch.vkCmdDrawMultiEXT) {
          try {
            cmdBuf.drawMultiEXT(exec->multiDrawInfo, 1u, 0u, sizeof(vk::MultiDrawInfoEXT), deviceObj->dispatch);
          }
          catch (std::exception e) {
            catchFn(e);
          };
        }
        else { catchFn(); };
        
        cmdBuf.endRendering();
        cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersEnd));
        return cmdBuf;
      });

      //
      decltype(auto) switchToAttachmentFence = framebuffers->switchToAttachment();
      decltype(auto) graphicsFence = deviceObj->executeCommandOnce(submission);
      decltype(auto) switchToShaderReadFence = framebuffers->switchToShaderRead();

      //
      return graphicsFence;
    };

  };
  
};
