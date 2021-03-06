#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Device.hpp"
#include "./PipelineLayout.hpp"
#include "./Framebuffer.hpp"
#include "./Swapchain.hpp"
#include "./PingPong.hpp"

// 
namespace ANAMED {

    // 
    class PipelineObj : public BaseObj {
    public:
        using tType = WrapShared<PipelineObj>;
        using BaseObj::BaseObj;

    protected:
        friend DeviceObj;
        friend FramebufferObj;
        friend PipelineLayoutObj;
        friend DenoiserObj;

        //
        std::vector<vk::Pipeline> secondaryPipelines = {};

        // 
        //vk::Pipeline pipeline = {};
        std::optional<PipelineCreateInfo> cInfo = PipelineCreateInfo{};
        std::vector<vk::PipelineShaderStageCreateInfo> pipelineStages = {};
        std::vector<vk::DynamicState> dynamicStates = {};

        // 
        std::vector<vk::Viewport> viewports = {};
        std::vector<vk::Rect2D> scissors = {};
        vk::Pipeline recreatedPipeline = {};
        //std::shared_ptr<DeviceObj> deviceObj = {};

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    public:
        // 
        PipelineObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            //this->construct(deviceObj, cInfo);
        };

        // 
        PipelineObj(Handle const& handle, cpp21::optional_ref<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
            //this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
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
        inline static tType make(Handle const& handle, cpp21::optional_ref<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
            auto shared = std::make_shared<PipelineObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

        // 
        PipelineCreateInfo& getCInfo() { return this->cInfo.value(); };
        PipelineCreateInfo const& getCInfo() const { return this->cInfo.value(); };

    protected:

        //
        virtual void createCompute(cpp21::optional_ref<ComputePipelineCreateInfo> compute = {}) {
            decltype(auto) descriptors = ANAMED::context->get<DeviceObj>(this->base)->get<PipelineLayoutObj>(this->cInfo->layout);
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) crInfo = makeComputePipelineStageInfo(device, *compute->code); this->pipelineStages.push_back(crInfo);
            decltype(auto) cmInfo = infoMap->set(vk::StructureType::eComputePipelineCreateInfo, vk::ComputePipelineCreateInfo{
              .flags = vk::PipelineCreateFlags{},
              .stage = this->pipelineStages.back(),
              .layout = this->cInfo->layout
                });


            this->handle = (this->recreatedPipeline = std::move<vk::Pipeline>(handleResult(device.createComputePipeline(descriptors->cache, cmInfo.value()))));
            //
            //ANAMED::context->get(this->base)->registerObj(this->handle, shared_from_this());
            //return this->SFT();
        };

        //
        virtual void createGraphics(cpp21::optional_ref<GraphicsPipelineCreateInfo> graphics = {}) {
            //this->pipeline = makeComputePipelineStageInfo(this->deviceObj->device, compute->code);
            //
            //ANAMED::context->get(this->base)->registerObj(this->handle, shared_from_this());
            //return this->SFT();
            decltype(auto) device = this->base.as<vk::Device>();

            //
            viewports = { vk::Viewport{.x = 0.f, .y = 0.f, .width = 1.f, .height = 1.f, .minDepth = 0.f, .maxDepth = 1.f} };
            scissors = { vk::Rect2D{{0,0},{1u,1u}} };

            // TODO: learn this feature...
            decltype(auto) pFragmentRate = infoMap->set(vk::StructureType::ePipelineFragmentShadingRateStateCreateInfoKHR, vk::PipelineFragmentShadingRateStateCreateInfoKHR{

                });

            //
            decltype(auto) pLibrary = infoMap->set(vk::StructureType::eGraphicsPipelineLibraryCreateInfoEXT, vk::GraphicsPipelineLibraryCreateInfoEXT{
              .flags =
                vk::GraphicsPipelineLibraryFlagBitsEXT::eVertexInputInterface |
                vk::GraphicsPipelineLibraryFlagBitsEXT::ePreRasterizationShaders |
                vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentShader |
                vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentOutputInterface
                });

            //
            decltype(auto) pRendering = infoMap->set(vk::StructureType::ePipelineRenderingCreateInfo, vk::PipelineRenderingCreateInfo{
              .pNext = pLibrary.get(),
              .viewMask = 0x0u,
              .depthAttachmentFormat = graphics->attachmentLayout->depthAttachmentFormat,
              .stencilAttachmentFormat = graphics->attachmentLayout->stencilAttachmentFormat
                });

            //
            decltype(auto) pVertexInput = infoMap->set(vk::StructureType::ePipelineVertexInputStateCreateInfo, vk::PipelineVertexInputStateCreateInfo{

                });

            //
            decltype(auto) pInputAssembly = infoMap->set(vk::StructureType::ePipelineInputAssemblyStateCreateInfo, vk::PipelineInputAssemblyStateCreateInfo{
              .topology = vk::PrimitiveTopology::ePointList
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
              .conservativeRasterizationMode = graphics->hasConservativeRaster ? (graphics->underestimated ? vk::ConservativeRasterizationModeEXT::eUnderestimate : vk::ConservativeRasterizationModeEXT::eOverestimate) : vk::ConservativeRasterizationModeEXT::eDisabled
                });

            //
            decltype(auto) pRasterization = infoMap->set(vk::StructureType::ePipelineRasterizationStateCreateInfo, vk::PipelineRasterizationStateCreateInfo{
              .pNext = pRasterizationConservative.get(),
              .depthClampEnable = true,
              .rasterizerDiscardEnable = false,
              .polygonMode = vk::PolygonMode::eFill,
              .cullMode = vk::CullModeFlagBits::eNone,
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
              .depthTestEnable = graphics->dynamicState->hasDepthTest,
              .depthWriteEnable = graphics->dynamicState->hasDepthWrite,
              .depthCompareOp = graphics->dynamicState->reversalDepth ? vk::CompareOp::eGreaterOrEqual : vk::CompareOp::eLessOrEqual,
              .depthBoundsTestEnable = false,
              .stencilTestEnable = false,
              .front = vk::StencilOpState{.failOp = vk::StencilOp::eKeep, .passOp = vk::StencilOp::eReplace, .compareOp = vk::CompareOp::eAlways },
              .back = vk::StencilOpState{.failOp = vk::StencilOp::eKeep, .passOp = vk::StencilOp::eReplace, .compareOp = vk::CompareOp::eAlways },
              .minDepthBounds = 0.f,
              .maxDepthBounds = 1.f
                });

            //
            decltype(auto) pColorBlend = infoMap->set(vk::StructureType::ePipelineColorBlendStateCreateInfo, vk::PipelineColorBlendStateCreateInfo{
              .logicOpEnable = true,
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
              vk::DynamicState::eViewportWithCount,
              vk::DynamicState::eDepthCompareOp,
              vk::DynamicState::eDepthTestEnable,
              vk::DynamicState::eDepthWriteEnable
                });

            // 
            decltype(auto) pInfo = infoMap->set(vk::StructureType::eGraphicsPipelineCreateInfo, vk::GraphicsPipelineCreateInfo{
              .pNext = &pRendering->setColorAttachmentFormats(graphics->attachmentLayout->colorAttachmentFormats),
              .flags = vk::PipelineCreateFlags{},
              .pVertexInputState = pVertexInput.get(),
              .pInputAssemblyState = pInputAssembly.get(),
              .pTessellationState = pTessellation.get(),
              .pViewportState = pViewport.get(),//->setViewports(viewports).setScissors(scissors),
              .pRasterizationState = pRasterization.get(),
              .pMultisampleState = pMultisample.get(),
              .pDepthStencilState = pDepthStencil.get(),
              .pColorBlendState = &pColorBlend->setAttachments(graphics->attachmentLayout->blendStates),
              .pDynamicState = &pDynamic->setDynamicStates(this->dynamicStates),
              .layout = this->cInfo->layout
                })->setStages(pipelineStages);

            //
            decltype(auto) descriptors = ANAMED::context->get<DeviceObj>(this->base)->get<PipelineLayoutObj>(this->cInfo->layout);
            this->handle = (this->recreatedPipeline = std::move<vk::Pipeline>(handleResult(device.createGraphicsPipeline(descriptors->cache, infoMap->get<vk::GraphicsPipelineCreateInfo>(vk::StructureType::eGraphicsPipelineCreateInfo)))));
        };

        // 
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
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
        virtual tType writeComputeCommand(WriteComputeInfo const& exec = WriteComputeInfo{}) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) descriptorsObj = deviceObj->get<PipelineLayoutObj>(exec.layout ? exec.layout : this->cInfo->layout);
            decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

            //
            this->recreatedPipeline = std::move<vk::Pipeline>(handleResult(device.createComputePipeline(descriptorsObj->cache, infoMap->get<vk::ComputePipelineCreateInfo>(vk::StructureType::eComputePipelineCreateInfo))));

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
            decltype(auto) sets = descriptorsObj->sets;

            {
                exec.cmdBuf.pipelineBarrier2(depInfo.setMemoryBarriers(memoryBarriersBegin));
                if (sets.size() > 0) { exec.cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eCompute, descriptorsObj->handle.as<vk::PipelineLayout>(), 0u, sets, offsets); };
                descriptorsObj->writePushDescriptor(vk::PipelineBindPoint::eCompute, exec.cmdBuf);
                exec.cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, exec.pipelineIndex == 0u ? this->recreatedPipeline : this->secondaryPipelines[exec.pipelineIndex - 1u]);
                if (exec.instanceAddressBlock) {
                    exec.cmdBuf.pushConstants(descriptorsObj->handle.as<vk::PipelineLayout>(), vk::ShaderStageFlagBits::eAll, 0u, sizeof(InstanceAddressBlock), &exec.instanceAddressBlock.value());
                };
                exec.cmdBuf.dispatch(exec.dispatch.width, exec.dispatch.height, exec.dispatch.depth);
                exec.cmdBuf.pipelineBarrier2(depInfo.setMemoryBarriers(memoryBarriersEnd));
            };

            return SFT();
        };

        // TODO: using multiple-command
        virtual tType writeGraphicsCommand(WriteGraphicsInfo const& exec = WriteGraphicsInfo{}) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) pipelineLayout = exec.layout ? exec.layout : this->cInfo->layout;
            decltype(auto) descriptorsObj = deviceObj->get<PipelineLayoutObj>(pipelineLayout);
            decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };
            decltype(auto) framebuffer = deviceObj->get<FramebufferObj>(exec.framebuffer).shared();
            decltype(auto) dynamicState = this->cInfo->graphics->dynamicState;

            // TODO: pipeline destruction system
            //device.destroyPipeline(this->recreatedPipeline);
            //if (!this->recreatedPipeline) {
                this->recreatedPipeline = std::move<vk::Pipeline>(handleResult(device.createGraphicsPipeline(descriptorsObj->cache, infoMap->get<vk::GraphicsPipelineCreateInfo>(vk::StructureType::eGraphicsPipelineCreateInfo))));
            //};

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
            decltype(auto) sets = descriptorsObj->sets;

            // 
            auto _depInfo = depInfo;
            if (framebuffer) { framebuffer->writeSwitchToAttachment(exec.cmdBuf); };
            exec.cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersBegin));
            exec.cmdBuf.beginRendering(vk::RenderingInfoKHR{ .renderArea = renderArea, .layerCount = this->cInfo->graphics->attachmentLayout->type == FramebufferType::eCubemap ? 6u : 1u, .viewMask = 0x0u, .colorAttachmentCount = uint32_t(colorAttachments.size()), .pColorAttachments = colorAttachments.data(), .pDepthAttachment = &depthAttachment, .pStencilAttachment = &stencilAttachment });
            exec.cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, exec.pipelineIndex == 0u ? this->recreatedPipeline : this->secondaryPipelines[exec.pipelineIndex - 1u]);
            if (sets.size() > 0) { exec.cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, descriptorsObj->handle.as<vk::PipelineLayout>(), 0u, sets, offsets); };
            descriptorsObj->writePushDescriptor(vk::PipelineBindPoint::eGraphics, exec.cmdBuf);
            exec.cmdBuf.setViewportWithCount(viewports);
            exec.cmdBuf.setScissorWithCount(scissors);
            exec.cmdBuf.setDepthTestEnable(dynamicState->hasDepthTest);
            exec.cmdBuf.setDepthWriteEnable(dynamicState->hasDepthWrite);
            exec.cmdBuf.setDepthCompareOp(dynamicState->reversalDepth ? vk::CompareOp::eGreaterOrEqual : vk::CompareOp::eLessOrEqual);

            //
            using fnT = void(cpp21::shared_vector<vk::MultiDrawInfoEXT> const&, uint32_t const&);
            using fnTp = std::function<fnT>;

            //
            if (exec.instanceDraws.size() > 0) {
                for (decltype(auto) instInfo : exec.instanceDraws) {
                    decltype(auto) multiDrawDirect = supportMultiDraw ?
                        fnTp([cmdBuf = exec.cmdBuf, dispatch = deviceObj->getDispatch()](cpp21::shared_vector<vk::MultiDrawInfoEXT> const& multiDraw, uint32_t const& instanceCount = 1u) {
                        cmdBuf.drawMultiEXT(*multiDraw, instanceCount, 0u, sizeof(vk::MultiDrawInfoEXT), dispatch);
                    }) :
                        fnTp([cmdBuf = exec.cmdBuf, pipelineLayout, instInfo](cpp21::shared_vector<vk::MultiDrawInfoEXT> const& multiDraw, uint32_t const& instanceCount = 1u) {
                        uint32_t index = 0u;
                        for (decltype(auto) drawInfo : *multiDraw) {
                            decltype(auto) pushed = instInfo.drawConst->with(index++);
                            cmdBuf.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(InstanceAddressBlock), sizeof(InstanceDrawInfo), &pushed);
                            cmdBuf.draw(drawInfo.vertexCount, instanceCount, drawInfo.firstVertex, 0u);
                        };
                    });

                    // 
                    if (exec.instanceAddressBlock) {
                        exec.cmdBuf.pushConstants(descriptorsObj->handle.as<vk::PipelineLayout>(), vk::ShaderStageFlagBits::eAll, 0u, sizeof(InstanceAddressBlock), &exec.instanceAddressBlock.value());
                    };
                    if (instInfo.drawConst) {
                        exec.cmdBuf.pushConstants(descriptorsObj->handle.as<vk::PipelineLayout>(), vk::ShaderStageFlagBits::eAll, sizeof(InstanceAddressBlock), sizeof(InstanceDrawInfo), &instInfo.drawConst.value());
                    };
                    multiDrawDirect(instInfo.drawInfos, instInfo.drawConst ? instInfo.drawConst->instanceCount : 1u);
                };
            };

            // 
            exec.cmdBuf.endRendering();
            exec.cmdBuf.pipelineBarrier2(_depInfo.setMemoryBarriers(memoryBarriersEnd));
            if (framebuffer) { framebuffer->writeSwitchToShaderRead(exec.cmdBuf); };

            // 
            return SFT();
        };

        // TODO: using multiple-command
        virtual FenceType executePipelineOnce(ExecutePipelineInfo const& exec = ExecutePipelineInfo{}) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) submission = CommandOnceSubmission{ .submission = exec.submission };
            decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

            // 
            submission.commandInits.push_back([exec, this](vk::CommandBuffer const& cmdBuf) {
                if (exec.graphics) { this->writeGraphicsCommand(exec.graphics->with(cmdBuf)); };
                if (exec.compute) { this->writeComputeCommand(exec.compute->with(cmdBuf)); };
                return cmdBuf;
            });

            //
            submission.submission.onDone.push_back([device, pipeline = this->recreatedPipeline, graphics=exec.graphics, compute = exec.compute](vk::Result result) {
                if (graphics || compute) { device.destroyPipeline(pipeline); };
            });

            //
            return deviceObj->executeCommandOnce(submission);
        };

    };

};
#endif
