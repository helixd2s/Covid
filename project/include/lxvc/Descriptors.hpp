#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"
#include "./Resource.hpp"

//
struct DescriptorBindings {
  std::vector<vk::DescriptorSetLayoutBinding> bindings = {};
  std::vector<vk::DescriptorBindingFlags> bindingFlags = {};
  
};

// 
namespace lxvc {
  
  // 
  class DescriptorsObj : public BaseObj {
  public: 
    using tType = WrapShared<DescriptorsObj>;
    using BaseObj::BaseObj;
    //using BaseObj;

  protected: 
    // 
    friend DeviceObj;
    friend PipelineObj;
    friend ResourceObj;
    friend FramebufferObj;
    friend SwapchainObj;

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

    // 
    std::vector<vk::DescriptorPoolSize> DPC = {};
    std::optional<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{};

    //
    //std::shared_ptr<DeviceObj> deviceObj = {};
    //std::shared_ptr<ResourceObj> uniformBuffer = {};
    vk::Buffer uniformBuffer = {};

    //
    std::vector<char8_t> initialData = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

  public:
    //
    virtual cpp21::bucket<vk::DescriptorImageInfo>& getTextureDescriptors() { return textures; };
    virtual cpp21::bucket<vk::DescriptorImageInfo> const& getTextureDescriptors() const { return textures; };

    //
    virtual cpp21::bucket<vk::DescriptorImageInfo>& getSamplerDescriptors() { return samplers; };
    virtual cpp21::bucket<vk::DescriptorImageInfo> const& getSamplerDescriptors() const { return samplers; };

    //
    virtual cpp21::bucket<vk::DescriptorImageInfo>& getImageDescriptors() { return images; };
    virtual cpp21::bucket<vk::DescriptorImageInfo> const& getImageDescriptors() const { return images; };

    // 
    DescriptorsObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };
    
    // 
    DescriptorsObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) {
      auto shared = std::make_shared<DescriptorsObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  protected:
    //
    virtual void createDescriptorLayout(cpp21::const_wrap_arg<vk::DescriptorType> type, cpp21::const_wrap_arg<uint32_t> count = 1u) {
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
        .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool
        })->setBindings(layoutBindingStack->bindings);
      this->layouts.push_back(device.createDescriptorSetLayout(layoutInfo));
      this->descriptorCounts.push_back(count);
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());

      //
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) DPI = infoMap->set(vk::StructureType::eDescriptorPoolCreateInfo, vk::DescriptorPoolCreateInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
        .maxSets = 6u,
      })->setPoolSizes(this->DPC = std::vector<vk::DescriptorPoolSize>{
        vk::DescriptorPoolSize{ vk::DescriptorType::eSampler, 64u },
        vk::DescriptorPoolSize{ vk::DescriptorType::eSampledImage, 256u },
        vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBuffer, 2u },
        vk::DescriptorPoolSize{ vk::DescriptorType::eStorageImage, 64u },
      });

      // 
      this->layoutBindings = cpp21::vector_of_shared<DescriptorBindings>();
      this->layouts = std::vector<vk::DescriptorSetLayout>{};
      this->sets = std::vector<vk::DescriptorSet>{};
      this->layoutInfoMaps = cpp21::vector_of_shared<MSS>();
      this->descriptorCounts = std::vector<uint32_t>{};
      this->createDescriptorLayout(vk::DescriptorType::eUniformBuffer, 1u);
      this->createDescriptorLayout(vk::DescriptorType::eSampledImage, 256u);
      this->createDescriptorLayout(vk::DescriptorType::eSampler, 64u);
      this->createDescriptorLayout(vk::DescriptorType::eStorageImage, 64u);

      //
      this->sets = device.allocateDescriptorSets(this->infoMap->set(vk::StructureType::eDescriptorSetAllocateInfo, vk::DescriptorSetAllocateInfo{
        .pNext = &this->infoMap->set(vk::StructureType::eDescriptorSetVariableDescriptorCountAllocateInfo, vk::DescriptorSetVariableDescriptorCountAllocateInfo{

        })->setDescriptorCounts(this->descriptorCounts),
        .descriptorPool = (this->pool = device.createDescriptorPool(DPI))
      })->setSetLayouts(this->layouts));

      //
      this->handle = device.createPipelineLayout(infoMap->set(vk::StructureType::ePipelineLayoutCreateInfo, vk::PipelineLayoutCreateInfo{

      })->setSetLayouts(this->layouts).setPushConstantRanges(this->pushConstantRanges));

      //
      this->cache = device.createPipelineCache(infoMap->set(vk::StructureType::ePipelineCacheCreateInfo, vk::PipelineCacheCreateInfo{

      })->setInitialData<char8_t>(this->initialData));

      //
      //lxvc::context->get(this->base)->registerObj(this->handle, shared_from_this());

      //
      decltype(auto) uniformSize = 65536ull;
      this->uniformBuffer = ResourceObj::make(this->base, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .size = uniformSize,
          .type = BufferType::eUniform
        }
      }).as<vk::Buffer>();

      //
      this->uniformBufferDesc = vk::DescriptorBufferInfo{ this->uniformBuffer, 0ull, uniformSize};
      this->updateDescriptors();

      // 
      //return this->SFT();
    };

  public:

    //
    virtual void updateDescriptors() {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) writes = std::vector<vk::WriteDescriptorSet>{};
      decltype(auto) temp = vk::WriteDescriptorSet{ .dstSet = this->sets[0u], .dstBinding = 0u, .dstArrayElement = 0u, .descriptorType = vk::DescriptorType::eUniformBuffer };
      writes.push_back(vk::WriteDescriptorSet(temp).setDstSet(this->sets[0u]).setDescriptorType(vk::DescriptorType::eUniformBuffer).setPBufferInfo(&this->uniformBufferDesc.value()).setDescriptorCount(1u));
      if (this->textures->size() > 0ull) { writes.push_back(vk::WriteDescriptorSet(temp).setDstSet(this->sets[1u]).setPImageInfo(this->textures.data()).setDescriptorCount(uint32_t(this->textures.size())).setDescriptorType(vk::DescriptorType::eSampledImage)); };
      if (this->samplers->size() > 0ull) { writes.push_back(vk::WriteDescriptorSet(temp).setDstSet(this->sets[2u]).setPImageInfo(this->samplers.data()).setDescriptorCount(uint32_t(this->samplers.size())).setDescriptorType(vk::DescriptorType::eSampler)); };
      if (this->images->size() > 0ull) { writes.push_back(vk::WriteDescriptorSet(temp).setDstSet(this->sets[3u]).setPImageInfo(this->images.data()).setDescriptorCount(uint32_t(this->images.size())).setDescriptorType(vk::DescriptorType::eStorageImage)); };
      device.updateDescriptorSets(writes, {});
      //return this->SFT();
    };

    //
    virtual tType writeUniformUpdateCommand(cpp21::const_wrap_arg<UniformDataWriteSet> cInfo) {
      size_t size = std::min(cInfo->data.size(), cInfo->region->size);
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) depInfo = vk::DependencyInfo{ .dependencyFlags = vk::DependencyFlagBits::eByRegion };

      //
      decltype(auto) bufferBarriersBegin = std::vector<vk::BufferMemoryBarrier2>{
        vk::BufferMemoryBarrier2{
          .srcStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eGeneralRead),
          .srcAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eGeneralRead) | vk::AccessFlagBits2::eUniformRead,
          .dstStageMask = vku::getCorrectPipelineStagesByAccessMask<vk::PipelineStageFlagBits2>(AccessFlagBitsSet::eTransferWrite),
          .dstAccessMask = vk::AccessFlagBits2(AccessFlagBitsSet::eTransferWrite),
          .srcQueueFamilyIndex = cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = cInfo->info->queueFamilyIndex,
          .buffer = this->uniformBuffer,
          .offset = cInfo->region->offset,
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
          .srcQueueFamilyIndex = cInfo->info->queueFamilyIndex,
          .dstQueueFamilyIndex = cInfo->info->queueFamilyIndex,
          .buffer = this->uniformBuffer,
          .offset = cInfo->region->offset,
          .size = size
        }
      };

      // 
      cInfo->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
      cInfo->cmdBuf.updateBuffer(this->uniformBuffer, cInfo->region->offset, size, cInfo->data.data());
      cInfo->cmdBuf.pipelineBarrier2(depInfo.setBufferMemoryBarriers(bufferBarriersEnd));

      // 
      return SFT();
    };

    //
    virtual FenceType executeUniformUpdateOnce(cpp21::const_wrap_arg<UniformDataSet> cInfo) {
      decltype(auto) submission = CommandOnceSubmission{ .submission = cInfo->submission };

      // 
      submission.commandInits.push_back([=](cpp21::const_wrap_arg<vk::CommandBuffer> cmdBuf) {
        this->writeUniformUpdateCommand(cInfo->writeInfo->with(cmdBuf));
        return cmdBuf;
      });

      //
      return lxvc::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };
    
  };

};
