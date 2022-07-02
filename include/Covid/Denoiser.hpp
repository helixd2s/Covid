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
#include <NRI.h>
#include <NRD.h>
#include <NRIDescs.hpp>
#include <Extensions/NRIWrapperVK.h>
#include <Extensions/NRIHelper.h>
#include <NRDIntegration.h>

// 
namespace ANAMED {

    //
    //struct MethodSettings {

    //};

    // 
    class DenoiserObj : public BaseObj {
    public:
        using tType = WrapShared<DenoiserObj>;
        using BaseObj::BaseObj;

    protected:
        friend DeviceObj;
        friend FramebufferObj;
        friend PipelineLayoutObj;

        //
        std::optional<DenoiserCreateInfo> cInfo = DenoiserCreateInfo{};

        // 
        std::vector<vk::Viewport> viewports = {};
        std::vector<vk::Rect2D> scissors = {};
        
        //
        nri::Device* nriDevice = nullptr;
        NrdIntegration NRD = NrdIntegration(1u);

        //
        struct NriInterface : public nri::CoreInterface, public nri::HelperInterface, public nri::WrapperVKInterface {};
        NriInterface NRI;

        //
        nrd::ReblurSettings settings = {};

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    public:
        // 
        DenoiserObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<DenoiserCreateInfo> cInfo = DenoiserCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            //this->construct(deviceObj, cInfo);
        };

        // 
        DenoiserObj(Handle const& handle, cpp21::optional_ref<DenoiserCreateInfo> cInfo = DenoiserCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
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
        inline static tType make(Handle const& handle, cpp21::optional_ref<DenoiserCreateInfo> cInfo = DenoiserCreateInfo{}) {
            auto shared = std::make_shared<DenoiserObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

        // 
        DenoiserCreateInfo& getCInfo() { return this->cInfo.value(); };
        DenoiserCreateInfo const& getCInfo() const { return this->cInfo.value(); };

    public:

        //
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<DenoiserCreateInfo> cInfo = DenoiserCreateInfo{}) {
            decltype(auto) device = this->base.as<vk::Device>();

            //
            nri::DeviceCreationVulkanDesc deviceDesc = {};
            deviceDesc.vkDevice = nri::NRIVkDevice(VkDevice(device));
            deviceDesc.vkInstance = nri::NRIVkInstance(VkInstance(deviceObj->getBase().as<VkInstance>()));
            deviceDesc.vkPhysicalDevices = (nri::NRIVkPhysicalDevice*)&deviceObj->getPhysicalDevice();
            deviceDesc.deviceGroupSize = 1u;
            deviceDesc.queueFamilyIndices = cInfo->queueFamilyIndices.data();
            deviceDesc.queueFamilyIndexNum = uint32_t(cInfo->queueFamilyIndices.size());
            deviceDesc.enableNRIValidation = true;

            // Wrap the device
            nri::Result result = nri::CreateDeviceFromVkDevice(deviceDesc, nriDevice);
            result = nri::GetInterface(*nriDevice, NRI_INTERFACE(nri::CoreInterface), (nri::CoreInterface*)&NRI);
            result = nri::GetInterface(*nriDevice, NRI_INTERFACE(nri::HelperInterface), (nri::HelperInterface*)&NRI);
            result = nri::GetInterface(*nriDevice, NRI_INTERFACE(nri::WrapperVKInterface), (nri::WrapperVKInterface*)&NRI);

            // 
            const nrd::MethodDesc methodDescs[] =
            {
              { nrd::Method::REBLUR_DIFFUSE_SPECULAR, cInfo->extent.width, cInfo->extent.height },
            };

            //
            nrd::DenoiserCreationDesc denoiserCreationDesc = {};
            denoiserCreationDesc.requestedMethods = methodDescs;
            denoiserCreationDesc.requestedMethodNum = 1;

            //
            NRD.Initialize(*nriDevice, NRI, NRI, denoiserCreationDesc);

        };

        //
        virtual void executeDenoise(std::tuple<nri::CommandBuffer*, NrdUserPool> writed) {
            // Populate common settings
            nrd::CommonSettings commonSettings = {};


            //
            NRD.SetMethodSettings(nrd::Method::REBLUR_DIFFUSE_SPECULAR, &settings);
            NRD.Denoise(0u, *std::get<0u>(writed), commonSettings, std::get<1u>(writed), true);
        };

        // 
        virtual std::tuple<nri::CommandBuffer*, NrdUserPool> writeDenoiseCommand(cpp21::optional_ref<DenoiseCommandWriteInfo> info) {
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

            // 
            nri::CommandBufferVulkanDesc cmdDesc = {};
            cmdDesc.vkCommandBuffer = nri::NRIVkCommandBuffer(VkCommandBuffer(info->cmdBuf));
            cmdDesc.commandQueueType = nri::CommandQueueType::COMPUTE; // Not needed for NRD Integration layer

            // 
            nri::CommandBuffer* cmdBuffer = nullptr;
            NRI.CreateCommandBufferVK(*nriDevice, cmdDesc, cmdBuffer);

            // 
            NrdUserPool userPool =
            { {
                    // 
                  } };

            // 
            for (uint32_t i = 0; i < info->images.size(); i++)
            { // 
                decltype(auto) myResource = deviceObj->get<ResourceImageObj>(info->images[i]);
                decltype(auto) imgCreateInfo = myResource->getInfoMap()->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);

                // 
                nri::TextureTransitionBarrierDesc& entryDesc = *userPool[i].subresourceStates;
                nri::TextureVulkanDesc textureDesc = {};
                textureDesc.vkImage = myResource.as<VkImage>();
                textureDesc.vkFormat = std::to_underlying(imgCreateInfo->format);
                textureDesc.vkImageAspectFlags = std::to_underlying(myResource->aspectMask());
                textureDesc.size[0] = imgCreateInfo->extent.width;
                textureDesc.size[1] = imgCreateInfo->extent.height;
                textureDesc.size[2] = imgCreateInfo->extent.depth;
                textureDesc.mipNum = imgCreateInfo->mipLevels;
                textureDesc.arraySize = imgCreateInfo->arrayLayers;
                textureDesc.sampleNum = 1u;
                textureDesc.physicalDeviceMask = 0x1u;
                NRI.CreateTextureVK(*nriDevice, textureDesc, (nri::Texture*&)entryDesc.texture);

                // 
                entryDesc.nextAccess = nri::AccessBits::SHADER_RESOURCE_STORAGE | nri::AccessBits::SHADER_RESOURCE;
                entryDesc.nextLayout = nri::TextureLayout::GENERAL;

                //
                userPool[i].format = nri::Format::RGBA16_SFLOAT;
            };

            // 
            return std::make_tuple<nri::CommandBuffer*, NrdUserPool>(std::move(cmdBuffer), std::move(userPool));
        };


    };

};
#endif
