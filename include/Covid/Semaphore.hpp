#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"

// 
namespace ANAMED {

    // 
    class SemaphoreObj : public BaseObj {
    public:
        using tType = WrapShared<SemaphoreObj>;
        using BaseObj::BaseObj;
        //using BaseObj;

    protected:
        // 
        friend DeviceObj;
        friend PipelineObj;
        friend ResourceObj;
        friend FramebufferObj;
        friend SwapchainObj;
        friend PingPongObj;

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

        //
        ExtHandle extHandle = {};
        std::optional<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{};

    public:

        // 
        SemaphoreObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            this->construct(deviceObj.shared(), cInfo);
        };

        // 
        SemaphoreObj(Handle const& handle, cpp21::optional_ref<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
            this->construct(ANAMED::context->get<DeviceObj>(this->base = handle).shared(), cInfo);
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
        inline static tType make(Handle const& handle, cpp21::optional_ref<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{}) {
            auto shared = std::make_shared<SemaphoreObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

        virtual ExtHandle& getExtHandle() { return extHandle; };
        virtual ExtHandle const& getExtHandle() const { return extHandle; };

    protected:

        // 
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{}) {
            if (cInfo) { this->cInfo = cInfo; };

            //
            decltype(auto) device = this->base.as<vk::Device>();
            //decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);

            // 
            decltype(auto) semExport = infoMap->set(vk::StructureType::eExportSemaphoreCreateInfoKHR, vk::ExportSemaphoreCreateInfoKHR{ .handleTypes = extSemFlags });
            decltype(auto) semType = infoMap->set(vk::StructureType::eSemaphoreTypeCreateInfo, vk::SemaphoreTypeCreateInfo{ .pNext = cInfo->hasExport ? semExport.get() : nullptr, .semaphoreType = vk::SemaphoreType::eBinary, .initialValue = 0ull });
            decltype(auto) semInfo = infoMap->set(vk::StructureType::eSemaphoreCreateInfo, vk::SemaphoreCreateInfo{ .pNext = semType.get(), .flags = {} });
            decltype(auto) semSubmit = infoMap->set(vk::StructureType::eSemaphoreSubmitInfo, vk::SemaphoreSubmitInfo{ .semaphore = (this->handle = handleResult(this->base.as<vk::Device>().createSemaphore(semInfo.value()))).as<vk::Semaphore>(), .value = semType->initialValue, .stageMask = vk::PipelineStageFlagBits2::eAllCommands });

            //
            //this->handle = device.createSemaphore(semInfo.value());

            // 
            if (cInfo->hasExport) {
#ifdef _WIN32
                this->extHandle = handleResult(device.getSemaphoreWin32HandleKHR(vk::SemaphoreGetWin32HandleInfoKHR{ .semaphore = this->handle.as<vk::Semaphore>(), .handleType = extSemFlagBits }, deviceObj->getDispatch()));
#else
#ifdef __linux__ 
                this->extHandle = handleResult(device.getSemaphoreFdKHR(vk::SemaphoreGetFdInfoKHR{ .semaphore = this->handle.as<vk::Semaphore>(), .handleType = extSemFlagBits }, deviceObj->getDispatch()));
#endif
#endif
            };
        };

    public:

    };

};
#endif
