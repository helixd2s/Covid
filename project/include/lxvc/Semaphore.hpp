#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"

// 
namespace lxvc {
  
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

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    //
    std::optional<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{};

  public:

    // 
    SemaphoreObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->handle;
      this->construct(deviceObj, cInfo);
    };

    // 
    SemaphoreObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{}) : cInfo(cInfo) {
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
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{}) {
      auto shared = std::make_shared<SemaphoreObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  protected:

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<SemaphoreCreateInfo> cInfo = SemaphoreCreateInfo{}) {
      this->base = deviceObj->handle;
      //this->deviceObj = deviceObj;
      if (cInfo) { this->cInfo = cInfo; };
      this->infoMap = std::make_shared<MSS>(MSS());

      // 
      decltype(auto) semType = infoMap->set(vk::StructureType::eSemaphoreTypeCreateInfo, vk::SemaphoreTypeCreateInfo{ .semaphoreType = vk::SemaphoreType::eBinary, .initialValue = 0ull });
      decltype(auto) semInfo = infoMap->set(vk::StructureType::eSemaphoreCreateInfo, vk::SemaphoreCreateInfo{ .pNext = semType.get(), .flags = {} });
      decltype(auto) semSubmit = infoMap->set(vk::StructureType::eSemaphoreSubmitInfo, vk::SemaphoreSubmitInfo{ .semaphore = (this->handle = this->base.as<vk::Device>().createSemaphore(semInfo.ref())).as<vk::Semaphore>(), .value = semType->initialValue, .stageMask = vk::PipelineStageFlagBits2::eAllCommands});
    };

  public:
    
  };

};
