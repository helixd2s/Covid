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
  class DescriptorsObj : std::enable_shared_from_this<DescriptorsObj> {
  protected: 
    using tType = std::shared_ptr<DescriptorsObj>;
    friend DeviceObj;
    friend PipelineObj;
    friend ResourceObj;

    //
    vk::PipelineCache cache = {};
    vk::PipelineLayout layout = {};
    vk::DescriptorPool pool = {};
    std::vector<vk::DescriptorSet> sets = {};
    std::vector<vk::DescriptorSetLayout> layouts = {};
    std::vector<vk::PushConstantRange> pushConstantRanges = {};

    // 
    cpp21::vector_of_shared<MSS> layoutInfoMaps = {};
    cpp21::vector_of_shared<DescriptorBindings> layoutBindings = {};

    //
    cpp21::bucket<vk::DescriptorImageInfo> textures = std::vector<vk::DescriptorImageInfo>{};
    cpp21::bucket<vk::DescriptorImageInfo> samplers = std::vector<vk::DescriptorImageInfo>{};
    std::optional<vk::DescriptorBufferInfo> uniformBufferDesc = {};

    // 
    std::vector<vk::DescriptorPoolSize> DPC = {};
    std::optional<DescriptorsCreateInfo> cInfo = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};
    std::shared_ptr<ResourceObj> uniformBuffer = {};

    //
    std::vector<char8_t> initialData = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

  public:
    // 
    DescriptorsObj(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

  protected:
    //
    virtual tType createDescriptorLayout(vk::DescriptorType const& type, uint32_t const& count = 1u) {
      decltype(auto) device = this->deviceObj->device;
      decltype(auto) last = this->layouts.size();
      this->layouts.push_back(vk::DescriptorSetLayout{});
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
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, std::optional<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();

      //
      decltype(auto) device = this->deviceObj->device;
      decltype(auto) DPI = infoMap->set(vk::StructureType::eDescriptorPoolCreateInfo, vk::DescriptorPoolCreateInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
      })->setPoolSizes(this->DPC = std::vector<vk::DescriptorPoolSize>{
        vk::DescriptorPoolSize{ vk::DescriptorType::eSampler, 64u },
        vk::DescriptorPoolSize{ vk::DescriptorType::eSampledImage, 256u },
        vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBuffer, 2u }
      });

      // 
      this->createDescriptorLayout(vk::DescriptorType::eUniformBuffer, 1u);
      this->createDescriptorLayout(vk::DescriptorType::eSampledImage, 256u);
      this->createDescriptorLayout(vk::DescriptorType::eSampler, 64u);

      //
      this->sets = device.allocateDescriptorSets(this->infoMap->set(vk::StructureType::eDescriptorSetAllocateInfo, vk::DescriptorSetAllocateInfo{
        .descriptorPool = (this->pool = device.createDescriptorPool(DPI))
      })->setSetLayouts(this->layouts));

      //
      this->layout = device.createPipelineLayout(infoMap->set(vk::StructureType::ePipelineLayoutCreateInfo, vk::PipelineLayoutCreateInfo{

      })->setSetLayouts(this->layouts).setPushConstantRanges(this->pushConstantRanges));

      //
      this->cache = device.createPipelineCache(infoMap->set(vk::StructureType::ePipelineCacheCreateInfo, vk::PipelineCacheCreateInfo{

      })->setInitialData<char8_t>(this->initialData));

      //
      decltype(auto) uniformSize = 65536ull;
      this->uniformBuffer = std::make_shared<ResourceObj>(this->deviceObj, ResourceCreateInfo{
        .bufferInfo = BufferCreateInfo{
          .type = BufferType::eUniform,
          .size = uniformSize
        }
      });

      //
      this->uniformBufferDesc = vk::DescriptorBufferInfo{ this->uniformBuffer->buffer, 0ull, uniformSize };
      this->updateDescriptors();

      // 
      return this->SFT();
    };

    //
    virtual tType updateDescriptors() {
      decltype(auto) device = this->deviceObj->device;
      decltype(auto) writes = std::vector<vk::WriteDescriptorSet>{};
      writes.push_back(vk::WriteDescriptorSet{ .dstSet = this->sets[0u], .dstBinding = 0u, .dstArrayElement = 0u, .descriptorCount = 1u, .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &this->uniformBufferDesc.value() });
      writes.push_back(vk::WriteDescriptorSet{ .dstSet = this->sets[1u], .dstBinding = 0u, .dstArrayElement = 0u, .descriptorCount = (uint32_t)this->textures->size(), .descriptorType = vk::DescriptorType::eSampledImage, .pImageInfo = this->textures->data() });
      writes.push_back(vk::WriteDescriptorSet{ .dstSet = this->sets[2u], .dstBinding = 0u, .dstArrayElement = 0u, .descriptorCount = (uint32_t)this->samplers->size(), .descriptorType = vk::DescriptorType::eSampler, .pImageInfo = this->samplers->data() });
      device.updateDescriptorSets(writes, {});
      return this->SFT();
    };
    
  };

};
