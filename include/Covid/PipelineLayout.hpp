#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
//#include "./Resource.hpp"

//
struct DescriptorBindings {
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {};
    std::vector<vk::DescriptorBindingFlags> bindingFlags = {};

};

// 
namespace ANAMED {

    // 
    class PipelineLayoutObj : public BaseObj {
    public:
        using tType = WrapShared<PipelineLayoutObj>;
        using BaseObj::BaseObj;
        //using BaseObj;

    protected:
        // 
        friend DeviceObj;
        friend PipelineObj;
        friend ResourceImageObj;
        friend ResourceBufferObj;
        friend FramebufferObj;
        friend SwapchainObj;

        //
        //std::vector<ResourceObj> bindedResources = {};

        //
        vk::PipelineCache cache = {};
        //vk::PipelineLayout layout = {};
        vk::DescriptorPool pool = {};
        std::vector<vk::DescriptorSet> sets = {};
        std::vector<vk::DescriptorSetLayout> layouts = {};
        std::vector<vk::PushConstantRange> pushConstantRanges = {};

        // 
        cpp21::vector_of_shared<MSS> layoutInfoMaps = {};
        cpp21::vector_of_shared<DescriptorBindings> layoutBindings = {};
        std::vector<uint32_t> descriptorCounts = {};

        //
        cpp21::bucket<vk::DescriptorImageInfo> textures = std::vector<vk::DescriptorImageInfo>{};
        cpp21::bucket<vk::DescriptorImageInfo> samplers = std::vector<vk::DescriptorImageInfo>{};
        cpp21::bucket<vk::DescriptorImageInfo> images = std::vector<vk::DescriptorImageInfo>{};
        std::optional<vk::DescriptorBufferInfo> uniformBufferDesc = {};
        std::vector<vk::DescriptorBufferInfo> cacheBufferDescs = {};

        // 
        std::vector<vk::DescriptorPoolSize> DPC = {};
        std::optional<PipelineLayoutCreateInfo> cInfo = PipelineLayoutCreateInfo{};

        //
        //std::shared_ptr<DeviceObj> deviceObj = {};
        //std::shared_ptr<ResourceObj> uniformBuffer = {};
        vk::Buffer uniformBuffer = {};
        vk::Buffer cacheBuffer = {};

        //
        WrapShared<ResourceBufferObj> uniformBufferObj = {};
        WrapShared<ResourceBufferObj> cacheBufferObj = {};

        //
        std::vector<char8_t> initialData = {};

        //
        size_t cachePages = 16u;
        size_t cachePageSize = 65536ull;
        size_t uniformSize = 65536ull;

        //
        vk::Image nullTexture = {};
        vk::Image nullImage = {};
        vk::Sampler nullSampler = {};

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };


    public:
        PipelineLayoutCreateInfo& getCInfo() { return this->cInfo.value(); };
        PipelineLayoutCreateInfo const& getCInfo() const { return this->cInfo.value(); };

        // 
        PipelineLayoutObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<PipelineLayoutCreateInfo> cInfo = PipelineLayoutCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            //this->construct(deviceObj, cInfo);
        };

        // 
        PipelineLayoutObj(Handle const& handle, cpp21::optional_ref<PipelineLayoutCreateInfo> cInfo = PipelineLayoutCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
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
        inline virtual std::vector<vk::DescriptorSet>& getDescriptorSets() { return this->sets; };
        inline virtual std::vector<vk::DescriptorSet> const& getDescriptorSets() const { return this->sets; };

        //
        inline virtual vk::PipelineCache& getPipelineCache() { return this->cache; };
        inline virtual vk::PipelineCache const& getPipelineCache() const { return this->cache; };

        //
        inline virtual vk::PipelineLayout& getPipelineLayout() { return this->handle.as<vk::PipelineLayout>(); };
        inline virtual vk::PipelineLayout const& getPipelineLayout() const { return this->handle.as<vk::PipelineLayout>(); };

        //
        inline virtual cpp21::bucket<vk::DescriptorImageInfo>& getTextureDescriptors() { return textures; };
        inline virtual cpp21::bucket<vk::DescriptorImageInfo> const& getTextureDescriptors() const { return textures; };

        //
        inline virtual cpp21::bucket<vk::DescriptorImageInfo>& getSamplerDescriptors() { return samplers; };
        inline virtual cpp21::bucket<vk::DescriptorImageInfo> const& getSamplerDescriptors() const { return samplers; };

        //
        inline virtual cpp21::bucket<vk::DescriptorImageInfo>& getImageDescriptors() { return images; };
        inline virtual cpp21::bucket<vk::DescriptorImageInfo> const& getImageDescriptors() const { return images; };

        //
        inline static tType make(Handle const& handle, cpp21::optional_ref<PipelineLayoutCreateInfo> cInfo = PipelineLayoutCreateInfo{}) {
            auto shared = std::make_shared<PipelineLayoutObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            wrap->createNullImages();
            return wrap;
        };

    protected:
        //
        virtual void createDescriptorLayout(cpp21::optional_ref<vk::DescriptorType> type, uint32_t const& count = 1u) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) last = this->layouts.size();
            this->layoutInfoMaps->push_back(std::make_shared<MSS>(MSS()));
            this->layoutBindings->push_back(std::make_shared<DescriptorBindings>());
            decltype(auto) layoutInfoMap = this->layoutInfoMaps[last];
            decltype(auto) layoutBindingStack = this->layoutBindings[last];
            layoutBindingStack->bindings.push_back(vk::DescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = type, .descriptorCount = count, .stageFlags = vk::ShaderStageFlagBits::eAll });
            layoutBindingStack->bindingFlags.push_back(vk::DescriptorBindingFlags{ vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind });
            decltype(auto) layoutInfo = layoutInfoMap->set(vk::StructureType::eDescriptorSetLayoutCreateInfo, vk::DescriptorSetLayoutCreateInfo{
              .pNext = &(layoutInfoMap->set(vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo, vk::DescriptorSetLayoutBindingFlagsCreateInfo{

              })->setBindingFlags(layoutBindingStack->bindingFlags)),
                .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool /*| vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR*/
                })->setBindings(layoutBindingStack->bindings);
            this->layouts.push_back(handleResult(device.createDescriptorSetLayout(layoutInfo)));
            this->descriptorCounts.push_back(count);
        };

        // 
        virtual void createDescriptorLayoutUniformStorage(uint32_t const& maxPageCount = 1u) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) last = this->layouts.size();
            this->layoutInfoMaps->push_back(std::make_shared<MSS>(MSS()));
            this->layoutBindings->push_back(std::make_shared<DescriptorBindings>());
            decltype(auto) layoutInfoMap = this->layoutInfoMaps[last];
            decltype(auto) layoutBindingStack = this->layoutBindings[last];
            layoutBindingStack->bindings.push_back(vk::DescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1u, .stageFlags = vk::ShaderStageFlagBits::eAll });
            layoutBindingStack->bindings.push_back(vk::DescriptorSetLayoutBinding{ .binding = 1u, .descriptorType = vk::DescriptorType::eStorageBuffer, .descriptorCount = maxPageCount, .stageFlags = vk::ShaderStageFlagBits::eAll });
            layoutBindingStack->bindingFlags.push_back(vk::DescriptorBindingFlags{ vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind });
            layoutBindingStack->bindingFlags.push_back(vk::DescriptorBindingFlags{ vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::eVariableDescriptorCount });
            decltype(auto) layoutInfo = layoutInfoMap->set(vk::StructureType::eDescriptorSetLayoutCreateInfo, vk::DescriptorSetLayoutCreateInfo{
              .pNext = &(layoutInfoMap->set(vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo, vk::DescriptorSetLayoutBindingFlagsCreateInfo{

              })->setBindingFlags(layoutBindingStack->bindingFlags)),
                .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool /*| vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR*/
                })->setBindings(layoutBindingStack->bindings);
            this->layouts.push_back(handleResult(device.createDescriptorSetLayout(layoutInfo)));
            this->descriptorCounts.push_back(maxPageCount);
        };

        // 
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<PipelineLayoutCreateInfo> cInfo = PipelineLayoutCreateInfo{}) {
            //this->deviceObj = deviceObj;
            if (cInfo) { this->cInfo = cInfo; };

            //
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) DPI = infoMap->set(vk::StructureType::eDescriptorPoolCreateInfo, vk::DescriptorPoolCreateInfo{
              .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
              .maxSets = 6u,
                })->setPoolSizes(this->DPC = std::vector<vk::DescriptorPoolSize>{
                  vk::DescriptorPoolSize{ vk::DescriptorType::eSampler, 64u },
                  vk::DescriptorPoolSize{ vk::DescriptorType::eSampledImage, 1024u },
                  vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBuffer, 2u },
                  vk::DescriptorPoolSize{ vk::DescriptorType::eStorageImage, 256u },
                  vk::DescriptorPoolSize{ vk::DescriptorType::eStorageBuffer, 2u },
                    });

            // 
            this->layoutBindings = cpp21::vector_of_shared<DescriptorBindings>();
            this->layouts = std::vector<vk::DescriptorSetLayout>{};
            this->sets = std::vector<vk::DescriptorSet>{};
            this->layoutInfoMaps = cpp21::vector_of_shared<MSS>();
            this->descriptorCounts = std::vector<uint32_t>{};
            //this->createDescriptorLayout(vk::DescriptorType::eUniformBuffer, 1u);
            //this->createDescriptorLayout(vk::DescriptorType::eStorageBuffer, 1u);
            this->createDescriptorLayoutUniformStorage(16u);
            this->createDescriptorLayout(vk::DescriptorType::eSampledImage, 1024u);
            this->createDescriptorLayout(vk::DescriptorType::eSampler, 64u);
            this->createDescriptorLayout(vk::DescriptorType::eStorageImage, 256u);

            // reserve for ping-pong
            this->pushConstantRanges.push_back(vk::PushConstantRange{ vk::ShaderStageFlagBits::eAll, 0ull, sizeof(InstanceDrawInfo) + sizeof(InstanceAddressBlock) });

            //
            this->sets = handleResult(device.allocateDescriptorSets(this->infoMap->set(vk::StructureType::eDescriptorSetAllocateInfo, vk::DescriptorSetAllocateInfo{
             .pNext = &this->infoMap->set(vk::StructureType::eDescriptorSetVariableDescriptorCountAllocateInfo, vk::DescriptorSetVariableDescriptorCountAllocateInfo{

              })->setDescriptorCounts(this->descriptorCounts),
              .descriptorPool = (this->pool = handleResult(device.createDescriptorPool(DPI)))
                })->setSetLayouts(this->layouts)));

            //
            this->handle = handleResult(device.createPipelineLayout(infoMap->set(vk::StructureType::ePipelineLayoutCreateInfo, vk::PipelineLayoutCreateInfo{

                })->setSetLayouts(this->layouts).setPushConstantRanges(this->pushConstantRanges)));

            //
            this->cache = handleResult(device.createPipelineCache(infoMap->set(vk::StructureType::ePipelineCacheCreateInfo, vk::PipelineCacheCreateInfo{

                })->setInitialData<char8_t>(this->initialData)));

            //
            //ANAMED::context->get(this->base)->registerObj(this->handle, shared_from_this());

            //
            this->createUniformBuffer();
            this->createCacheBuffer();
            this->updateDescriptors();

            // 
            //return this->SFT();
        };

        //
        virtual vk::Buffer& createUniformBuffer();
        virtual vk::Buffer& createCacheBuffer();
        virtual void createNullImages();

        // 
    public:

        //
        virtual void writePushDescriptor(vk::PipelineBindPoint const& bindPoint, vk::CommandBuffer const& cmdBuf) {
            /*
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            decltype(auto) temp = vk::WriteDescriptorSet{ .dstBinding = 0u, .dstArrayElement = 0u, .descriptorType = vk::DescriptorType::eUniformBuffer };
            decltype(auto) PL = this->handle.as<vk::PipelineLayout>();

            {
              std::vector<vk::WriteDescriptorSet> writes = {};
              if (this->uniformBufferDesc) {
                writes.push_back(vk::WriteDescriptorSet(temp).setDstBinding(0u).setDescriptorType(vk::DescriptorType::eUniformBuffer).setPBufferInfo(&this->uniformBufferDesc.value()).setDescriptorCount(1u));
              };
              if (this->cacheBufferDescs.size() > 0) {
                writes.push_back(vk::WriteDescriptorSet(temp).setDstBinding(1u).setDescriptorType(vk::DescriptorType::eStorageBuffer).setBufferInfo(this->cacheBufferDescs));
              };
              cmdBuf->pushDescriptorSetKHR(bindPoint, PL, 0u, writes, deviceObj->getDispatch());
            };

            if (this->textures->size() > 0ull) {
              std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(temp).setPImageInfo(this->textures.data()).setDescriptorCount(uint32_t(this->textures.size())).setDescriptorType(vk::DescriptorType::eSampledImage)
              };
              cmdBuf->pushDescriptorSetKHR(bindPoint, PL, 1u, writes, deviceObj->getDispatch());
            };

            if (this->samplers->size() > 0ull) {
              std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(temp).setPImageInfo(this->samplers.data()).setDescriptorCount(uint32_t(this->samplers.size())).setDescriptorType(vk::DescriptorType::eSampler)
              };
              cmdBuf->pushDescriptorSetKHR(bindPoint, PL, 2u, writes, deviceObj->getDispatch());
            };

            if (this->images->size() > 0ull) {
              std::vector<vk::WriteDescriptorSet> writes = {
                vk::WriteDescriptorSet(temp).setPImageInfo(this->images.data()).setDescriptorCount(uint32_t(this->images.size())).setDescriptorType(vk::DescriptorType::eStorageImage)
              };
              cmdBuf->pushDescriptorSetKHR(bindPoint, PL, 3u, writes, deviceObj->getDispatch());
            };*/
        };

        //
        virtual void updateDescriptors() {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) temp = vk::WriteDescriptorSet{ .dstSet = this->sets[0u], .dstBinding = 0u, .dstArrayElement = 0u, .descriptorType = vk::DescriptorType::eUniformBuffer };
            std::vector<vk::WriteDescriptorSet> writes = {};
            if (this->uniformBufferDesc) {
                writes.push_back(vk::WriteDescriptorSet(temp).setDstSet(this->sets[0u]).setDstBinding(0u).setDescriptorType(vk::DescriptorType::eUniformBuffer).setPBufferInfo(&this->uniformBufferDesc.value()).setDescriptorCount(1u));
            };
            if (this->cacheBufferDescs.size() > 0) {
                writes.push_back(vk::WriteDescriptorSet(temp).setDstSet(this->sets[0u]).setDstBinding(1u).setDescriptorType(vk::DescriptorType::eStorageBuffer).setBufferInfo(this->cacheBufferDescs));
            };
            if (this->textures->size() > 0ull) { writes.push_back(vk::WriteDescriptorSet(temp).setDstSet(this->sets[1u]).setPImageInfo(this->textures.data()).setDescriptorCount(uint32_t(this->textures.size())).setDescriptorType(vk::DescriptorType::eSampledImage)); };
            if (this->samplers->size() > 0ull) { writes.push_back(vk::WriteDescriptorSet(temp).setDstSet(this->sets[2u]).setPImageInfo(this->samplers.data()).setDescriptorCount(uint32_t(this->samplers.size())).setDescriptorType(vk::DescriptorType::eSampler)); };
            if (this->images->size() > 0ull) { writes.push_back(vk::WriteDescriptorSet(temp).setDstSet(this->sets[3u]).setPImageInfo(this->images.data()).setDescriptorCount(uint32_t(this->images.size())).setDescriptorType(vk::DescriptorType::eStorageImage)); };

            //
            device.updateDescriptorSets(writes, {});
        };

        //
        virtual tType writeUniformUpdateCommand(UniformDataWriteSet const& cInfo) {
            size_t size = std::min(cInfo.data.size(), cInfo.region->size);
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

            //
            decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
              vk::BufferMemoryBarrier2{
                .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralRead),
                .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralRead) | vk::AccessFlagBits2::eUniformRead,
                .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
                .srcQueueFamilyIndex = cInfo.info->queueFamilyIndex,
                .dstQueueFamilyIndex = cInfo.info->queueFamilyIndex,
                .buffer = this->uniformBuffer,
                .offset = cInfo.region->offset,
                .size = size
              }
            };

            //
            decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
              vk::BufferMemoryBarrier2{
                .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
                .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralRead),
                .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralRead) | vk::AccessFlagBits2::eUniformRead,
                .srcQueueFamilyIndex = cInfo.info->queueFamilyIndex,
                .dstQueueFamilyIndex = cInfo.info->queueFamilyIndex,
                .buffer = this->uniformBuffer,
                .offset = cInfo.region->offset,
                .size = size
              }
            };

            // 
            cInfo.cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
            cInfo.cmdBuf.updateBuffer(this->uniformBuffer, cInfo.region->offset, size, cInfo.data.data());
            cInfo.cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd));

            // 
            return SFT();
        };

        virtual tType writeCacheUpdateCommand(CacheDataWriteSet const& cInfo) {
            size_t size = std::min(cInfo.data.size(), cInfo.region->size);
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

            //
            decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
              vk::BufferMemoryBarrier2{
                .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
                .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite) | vk::AccessFlagBits2::eShaderStorageWrite | vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eTransformFeedbackCounterReadEXT | vk::AccessFlagBits2::eTransformFeedbackCounterWriteEXT,
                .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
                .srcQueueFamilyIndex = cInfo.info->queueFamilyIndex,
                .dstQueueFamilyIndex = cInfo.info->queueFamilyIndex,
                .buffer = this->cacheBuffer,
                .offset = this->cachePageSize * cInfo.page + cInfo.region->offset,
                .size = size
              }
            };

            //
            decltype(auto) bufferBarriersEnd = std::vector<vk::BufferMemoryBarrier2>{
              vk::BufferMemoryBarrier2{
                .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
                .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite) | vk::AccessFlagBits2::eShaderStorageWrite | vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eTransformFeedbackCounterReadEXT | vk::AccessFlagBits2::eTransformFeedbackCounterWriteEXT,
                .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralReadWrite),
                .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralReadWrite),
                .srcQueueFamilyIndex = cInfo.info->queueFamilyIndex,
                .dstQueueFamilyIndex = cInfo.info->queueFamilyIndex,
                .buffer = this->cacheBuffer,
                .offset = this->cachePageSize * cInfo.page + cInfo.region->offset,
                .size = size
              }
            };

            // 
            cInfo.cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
            cInfo.cmdBuf.updateBuffer(this->cacheBuffer, this->cachePageSize * cInfo.page + cInfo.region->offset, size, cInfo.data.data());
            cInfo.cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd));

            // 
            return SFT();
        };

        //
        virtual FenceType executeUniformUpdateOnce(UniformDataSet const& cInfo) {
            decltype(auto) submission = CommandOnceSubmission{ .submission = cInfo.submission };

            // 
            submission.commandInits.push_back([this, cInfo](vk::CommandBuffer const& cmdBuf) {
                this->writeUniformUpdateCommand(cInfo.writeInfo->with(cmdBuf));
                return cmdBuf;
                });

            //
            return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
        };

        //
        virtual FenceType executeCacheUpdateOnce(CacheDataSet const& cInfo) {
            decltype(auto) submission = CommandOnceSubmission{ .submission = cInfo.submission };

            // 
            submission.commandInits.push_back([this, cInfo](vk::CommandBuffer const& cmdBuf) {
                this->writeCacheUpdateCommand(cInfo.writeInfo->with(cmdBuf));
                return cmdBuf;
                });

            //
            return ANAMED::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
        };

    };

};

#endif
