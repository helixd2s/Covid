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
#include <NRDIntegration.hpp>
#include <NRIDescs.hpp>
#include <Extensions/NRIWrapperD3D12.h>
#include <Extensions/NRIHelper.h>
#include <NRD.h>

// 
namespace ANAMED {

  // 
  class DenoiserObj : public PipelineObj {
  public: 
    using tType = WrapShared<DenoiserObj>;
    using BaseObj::BaseObj;

  protected:
    friend DeviceObj;
    friend FramebufferObj;
    friend PipelineLayoutObj;
    friend PipelineObj;

    //
    nri::Device nriDevice = nullptr;
    NrdIntegration NRD = NrdIntegration(1u);

    //
    struct NriInterface : public nri::CoreInterface, public nri::HelperInterface, public nri::WrapperD3D12Interface{};
    NriInterface NRI;

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public:
    // 
    DenoiserObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::carg<DenoiserCreateInfo> cInfo = DenoiserCreateInfo{}) : PipelineObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
      this->construct(deviceObj, cInfo);
    };

    // 
    DenoiserObj(cpp21::carg<Handle> handle, cpp21::carg<DenoiserCreateInfo> cInfo = DenoiserCreateInfo{}) : PipelineObj(handle), cInfo(cInfo) {
      this->construct(ANAMED::context->get<DeviceObj>(this->base), cInfo);
    };

    // 
     std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    inline static tType make(cpp21::carg<Handle> handle, cpp21::carg<DenoiserCreateInfo> cInfo = DenoiserCreateInfo{}) {
      auto shared = std::make_shared<DenoiserObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

    //
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::carg<PipelineCreateInfo> cInfo = PipelineCreateInfo{}) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

      //
      nri::DeviceCreationVulkanDesc deviceDesc = {};
      deviceDesc.vkDevice = VkDevice(device);
      deviceDesc.vkInstance = deviceObj->getBase().as<VkInstance>();
      deviceDesc.vkPhysicalAdapter = &deviceObj->getPhysicalDevice();
      deviceDesc.deviceGroupSize = 1u;
      deviceDesc.queueFamilyIndices = cInfo->queueFamilyIndices.data();
      deviceDesc.queueFamilyIndexNum = uint32_t(cInfo->queueFamilyIndices.size());
      deviceDesc.enableNRIValidation = false;

      // Wrap the device
      nri::Result result = nri::CreateDeviceFromVulkanDevice(deviceDesc, &nriDevice);
      result = nri::GetInterface(nriDevice, NRI_INTERFACE(nri::CoreInterface), (nri::CoreInterface*)&NRI);
      result = nri::GetInterface(nriDevice, NRI_INTERFACE(nri::HelperInterface), (nri::HelperInterface*)&NRI);
      result = nri::GetInterface(nriDevice, NRI_INTERFACE(nri::WrapperXXXInterface), (nri::WrapperXXXInterface*)&NRI);

      // 
      const nrd::MethodDesc methodDescs[] =
      {
        { nrd::Method::REBLUR_DIFFUSE_SPECULAR, cInfo->extent.width, cInfo->extent.height },
      };

      //
      nrd::DenoiserCreationDesc denoiserCreationDesc = {};
      denoiserCreationDesc.requestedMethods = methodDescs;
      denoiserCreationDesc.requestedMethodNum = methodNum;

      //
      bool result = NRD.Initialize(nriDevice, NRI, NRI, denoiserCreationDesc);

    };

    //
    virtual FenceType executeDenoise(std::tuple<nri::CommandBuffer, NrdUserPool> writed) {
      // Populate common settings
      //  - for the first time use defaults
      //  - currently NRD supports only the following view space: X - right, Y - top, Z - forward or backward
      nrd::CommonSettings commonSettings = {};
      PopulateCommonSettings(commonSettings);

      // Set settings for each denoiser
      nrd::NrdXxxSettings settings = {};
      PopulateDenoiserSettings(settings);

      //
      NRD.SetMethodSettings(nrd::Method::NRD_XXX, &settings);
      NRD.Denoise(std::get<0u>(writed), commonSettings, std::get<1u>(writed.userPool));
    };

    // 
    virtual std::tuple<nri::CommandBuffer, NrdUserPool> writeDenoiseCommand(cpp21::carg<DenoiseCommandWriteInfo> info) {
      decltype(auto) device = this->base.as<vk::Device>();
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

      // 
      nri::CommandBufferVulkanDesc cmdDesc = {};
      cmdDesc.vkCommandBuffer = info->cmdBuf;
      cmdDesc.commandQueueType = nri::CommandQueueType::COMPUTE; // Not needed for NRD Integration layer

      // 
      nri::CommandBuffer cmdBuffer = nullptr;
      NRI.CreateCommandBufferVK(nriDevice, cmdDesc, &cmdBuffer);

      // 
      NrdUserPool userPool =
      {{
        // 
      }};

      // 
      for (uint32_t i = 0; i < info->images.size(); i++)
      { // 
        decltype(auto) myResource = deviceObj->get<ResourceObj>(info->images[i]);
        decltype(auto) imgCreateInfo = myResource->infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);

        // 
        nri::TextureTransitionBarrierDesc& entryDesc = *userPool[i].subresourceStates;
        nri::TextureVulkanDesc textureDesc = {};
        textureDesc.vkImage = myResource.as<VkImage>();
        textureDesc.vkFormat = uint32_t(imgCreateInfo->format);
        textureDesc.vkImageAspectFlags = uint32_t(myResource->aspectMask());
        textureDesc.size = {imgCreateInfo->extent.width, imgCreateInfo->extent.height, imgCreateInfo->extent.depth};
        textureDesc.mipNum = imgCreateInfo->mipLevels;
        textureDesc.arraySize = imgCreateInfo->arrayLayers;
        textureDesc.sampleNum = 1u;
        textureDesc.physicalDeviceMask = 0x1u;
        NRI.CreateTextureVulkan(nriDevice, textureDesc, (nri::Texture*&)entryDesc.texture );

        // 
        entryDesc.nextAccess = nri::AccessBits::SHADER_RESOURCE_STORAGE | nri::AccessBits::SHADER_RESOURCE;
        entryDesc.nextLayout = nri::TextureLayout::GENERAL;
      };

      // 
      return std::make_tuple<nri::CommandBuffer, NrdUserPool>(*cmdBuffer, userPool);
    };

  protected:
    
  public:


  };
  
};
#endif
