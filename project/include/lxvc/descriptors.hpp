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
    std::optional<vk::DescriptorBufferInfo> uniformBufferDesc = {};

    // 
    std::vector<vk::DescriptorPoolSize> DPC = {};
    std::optional<DescriptorsCreateInfo> cInfo = {};

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
    DescriptorsObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };
    
    // 
    DescriptorsObj(Handle const& handle, std::optional<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(Handle const& handle, std::optional<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) {
      return std::make_shared<DescriptorsObj>(handle, cInfo)->registerSelf();
    };

  protected:
    //
    virtual void createDescriptorLayout(vk::DescriptorType const& type, uint32_t const& count = 1u) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) last = this->layouts.size();
      this->layoutInfoMaps->push_back(std::make_shared<MSS>());
      this->layoutBindings->push_back(std::make_shared<DescriptorBindings>());
      decltype(auto) layoutInfoMap = this->layoutInfoMaps[last];
      decltype(auto) layoutBindingStack = this->layoutBindings[last];
      opt_ref(layoutBindingStack->bindings)->push_back(vk::DescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = type, .descriptorCount = count, .stageFlags = vk::ShaderStageFlagBits::eAll });
      opt_ref(layoutBindingStack->bindingFlags)->push_back(vk::DescriptorBindingFlags{ vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind });
      decltype(auto) layoutInfo = layoutInfoMap->set(vk::StructureType::eDescriptorSetLayoutCreateInfo, vk::DescriptorSetLayoutCreateInfo{
        .pNext = &(layoutInfoMap->set(vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo, vk::DescriptorSetLayoutBindingFlagsCreateInfo{

        })->setBindingFlags(layoutBindingStack->bindingFlags)),
        .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool
        })->setBindings(layoutBindingStack->bindings);
      this->layouts.push_back(device.createDescriptorSetLayout(layoutInfo));
      this->descriptorCounts.push_back(count);
    };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();

      //
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) DPI = infoMap->set(vk::StructureType::eDescriptorPoolCreateInfo, vk::DescriptorPoolCreateInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
        .maxSets = 6u,
      })->setPoolSizes(this->DPC = std::vector<vk::DescriptorPoolSize>{
        vk::DescriptorPoolSize{ vk::DescriptorType::eSampler, 64u },
        vk::DescriptorPoolSize{ vk::DescriptorType::eSampledImage, 256u },
        vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBuffer, 2u }
      });

      // 
      this->layoutBindings = std::vector<std::shared_ptr<DescriptorBindings>>{};
      this->layouts = std::vector<vk::DescriptorSetLayout>{};
      this->sets = std::vector<vk::DescriptorSet>{};
      this->layoutInfoMaps = std::vector<std::shared_ptr<MSS>>{};
      this->descriptorCounts = std::vector<uint32_t>{};
      this->createDescriptorLayout(vk::DescriptorType::eUniformBuffer, 1u);
      this->createDescriptorLayout(vk::DescriptorType::eSampledImage, 256u);
      this->createDescriptorLayout(vk::DescriptorType::eSampler, 64u);

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
          .type = BufferType::eUniform,
          .size = uniformSize
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
      writes.push_back(vk::WriteDescriptorSet{ .dstSet = this->sets[0u], .dstBinding = 0u, .dstArrayElement = 0u, .descriptorCount = 1u, .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &this->uniformBufferDesc.value() });
      if (this->textures->size() > 0ull) writes.push_back(vk::WriteDescriptorSet{ .dstSet = this->sets[1u], .dstBinding = 0u, .dstArrayElement = 0u, .descriptorCount = (uint32_t)this->textures->size(), .descriptorType = vk::DescriptorType::eSampledImage, .pImageInfo = this->textures->data() });
      if (this->samplers->size() > 0ull) writes.push_back(vk::WriteDescriptorSet{ .dstSet = this->sets[2u], .dstBinding = 0u, .dstArrayElement = 0u, .descriptorCount = (uint32_t)this->samplers->size(), .descriptorType = vk::DescriptorType::eSampler, .pImageInfo = this->samplers->data() });
      device.updateDescriptorSets(writes, {});
      //return this->SFT();
    };

    //
    virtual FenceType setUniformData(UniformDataSet const& cInfo) {
      size_t size = std::min(cInfo.data.size(), cInfo.region->size);
      decltype(auto) submission = CommandOnceSubmission{ .info = cInfo.info };
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
      submission.commandInits.push_back([=](vk::CommandBuffer const& cmdBuf) {
        auto _depInfo = depInfo;
        cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersBegin));
        cmdBuf.updateBuffer(this->uniformBuffer, cInfo.region->offset, size, cInfo.data.data());
        cmdBuf.pipelineBarrier2(_depInfo.setBufferMemoryBarriers(bufferBarriersEnd));
        return cmdBuf;
      });

      //
      return lxvc::context->get<DeviceObj>(this->base)->executeCommandOnce(submission);
    };
    
  };

};
