#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"

//
struct DescriptorBindings {
  std::vector<vk::DescriptorSetLayoutBinding> bindings = {};
  std::vector<vk::DescriptorBindingFlags> bindingFlags = {};
};

// 
namespace lxvc {
  
  // 
  class DescriptorsObj : std::enable_shared_from_this<DescriptorsObj> {
  public:
    using tType = std::shared_ptr<DescriptorsObj>;
    friend DeviceObj;

    //
    vk::DescriptorPool pool = {};
    std::vector<vk::DescriptorSet> sets = {};
    std::vector<vk::DescriptorSetLayout> layouts = {};
    std::vector<std::shared_ptr<MSS>> layoutInfoMaps = {};
    std::vector<std::shared_ptr<DescriptorBindings>> layoutBindings = {};

    //
    cpp21::bucket<vk::DescriptorImageInfo> textures = std::vector<vk::DescriptorImageInfo>{};
    cpp21::bucket<vk::DescriptorImageInfo> samplers = std::vector<vk::DescriptorImageInfo>{};

    // 
    std::vector<vk::DescriptorPoolSize> DPC = {};
    std::optional<DescriptorsCreateInfo> cInfo = {};
    std::shared_ptr<MSS> infoMap = {};

    //
    std::shared_ptr<DeviceObj> deviceObj = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    DescriptorsObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) : deviceObj(deviceObj), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    //
    virtual tType createDescriptorLayout(vk::DescriptorType const& type, uint32_t const& count = 1u) {
      decltype(auto) last = this->layouts.size();
      this->layouts.push_back(vk::DescriptorSetLayout{});
      this->layoutInfoMaps.push_back(std::make_shared<MSS>());
      this->layoutBindings.push_back(std::make_shared<DescriptorBindings>());
      decltype(auto) layoutInfoMap = this->layoutInfoMaps[last];
      decltype(auto) layoutBindingStack = this->layoutBindings[last];
      opt_ref(layoutBindingStack->bindings)->push_back(vk::DescriptorSetLayoutBinding{ .binding = 0u, .descriptorType = type, .descriptorCount = count, .stageFlags = {} });
      opt_ref(layoutBindingStack->bindingFlags)->push_back(vk::DescriptorBindingFlags{ vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind });
      decltype(auto) layoutInfo = layoutInfoMap->set(vk::StructureType::eDescriptorSetLayoutCreateInfo, vk::DescriptorSetLayoutCreateInfo{
        .pNext = &(layoutInfoMap->set(vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo, vk::DescriptorSetLayoutBindingFlagsCreateInfo{

        })->setBindingFlags(layoutBindingStack->bindingFlags)),
        .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool
        })->setBindings(layoutBindingStack->bindings);
      this->layouts.push_back(this->deviceObj->device.createDescriptorSetLayout(layoutInfo));
    };

    // 
    virtual tType construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::uni_arg<DescriptorsCreateInfo> cInfo = DescriptorsCreateInfo{}) {
      this->deviceObj = deviceObj;
      this->cInfo = cInfo;
      this->infoMap = std::make_shared<MSS>();

      //
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
      this->sets = this->deviceObj->device.allocateDescriptorSets(this->infoMap->set(vk::StructureType::eDescriptorSetAllocateInfo, vk::DescriptorSetAllocateInfo{
        .descriptorPool = (this->pool = this->deviceObj->device.createDescriptorPool(DPI))
      })->setSetLayouts(this->layouts));

      // 
      return this->SFT();
    };

  };

};
