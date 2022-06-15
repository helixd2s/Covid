#pragma once 

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Device.hpp"
#include "./PipelineLayout.hpp"

// 
namespace ANAMED {

    // 
    class SamplerObj : public BaseObj {
    public:
        using tType = WrapShared<SamplerObj>;
        using BaseObj::BaseObj;

    protected:
        friend DeviceObj;
        friend PipelineLayoutObj;
        friend UploaderObj;
        friend FramebufferObj;
        friend SwapchainObj;
        friend GeometryLevelObj;
        friend InstanceLevelObj;
        friend MemoryAllocatorObj;

        //
        uint32_t descriptorId = 0xFFFFFFFFu;

        // 
        std::optional<SamplerCreateInfo> cInfo = SamplerCreateInfo{};

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    public:
        // 
        SamplerObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<SamplerCreateInfo> cInfo = SamplerCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            //this->construct(deviceObj, cInfo);
        };

        // 
        SamplerObj(Handle const& handle, cpp21::optional_ref<SamplerCreateInfo> cInfo = SamplerCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
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
        inline static tType make(Handle const& handle, cpp21::optional_ref<SamplerCreateInfo> cInfo = SamplerCreateInfo{}) {
            auto shared = std::make_shared<SamplerObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

    protected:

        // 
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<SamplerCreateInfo> cInfo = SamplerCreateInfo{}) {
            if (cInfo) { this->cInfo = cInfo; };

            // 
            decltype(auto) device = this->base.as<vk::Device>();
            decltype(auto) samplerInfo = infoMap->set(vk::StructureType::eSamplerCreateInfo, this->cInfo->native);
            this->handle = device.createSampler(this->cInfo->native.value());

            //
            decltype(auto) descriptorsObj = this->cInfo->descriptors ? deviceObj->get<PipelineLayoutObj>(this->cInfo->descriptors) : WrapShared<PipelineLayoutObj>{};
            if (descriptorsObj) {
                descriptorId = descriptorsObj->getSamplerDescriptors().add(vk::DescriptorImageInfo{ .sampler = this->handle.as<vk::Sampler>() });
            };

            //
            this->destructors.insert(this->destructors.begin(), 1, std::make_shared<std::function<DFun>>([device, sampler = this->handle.as<vk::Sampler>()](BaseObj const* baseObj) {
                device.destroySampler(sampler);
            }));

            //
            if (descriptorsObj) {
                this->destructors.insert(this->destructors.begin(), 1, std::make_shared<std::function<DFun>>([descriptorId = this->descriptorId, samplers = descriptorsObj->getSamplerDescriptors()](BaseObj const* baseObj) {
                    const_cast<cpp21::bucket<vk::DescriptorImageInfo>&>(samplers).removeByIndex(descriptorId);
                }));
            };
        };

    public:

        // 
        virtual uint32_t& getId() { return descriptorId; };
        virtual uint32_t const& getId() const { return descriptorId; };

    };


};
#endif
