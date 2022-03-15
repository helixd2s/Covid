#pragma once

// 
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"

// 
namespace ZNAMED {

  // 
  class MemoryAllocatorObj : public BaseObj {
  public:
    using BaseObj::BaseObj;
    using tType = WrapShared<MemoryAllocatorObj>;
    using cType = const char const*;
    //using BaseObj;

  protected:
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  protected:

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) {
      //this->deviceObj = deviceObj;
      this->base = deviceObj->getHandle();
      this->infoMap = std::make_shared<MSS>(MSS());
      this->callstack = std::make_shared<CallStack>();
    };

  public:

    //
    ~MemoryAllocatorObj() {
      this->tickProcessing();
    };

    // 
    MemoryAllocatorObj(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::const_wrap_arg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : cInfo(cInfo) {
      this->base = deviceObj->getHandle();
      this->construct(deviceObj, cInfo);
    };

    // 
    MemoryAllocatorObj(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : cInfo(cInfo) {
      this->construct(ZNAMED::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    //
    virtual tType registerSelf() {
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      deviceObj->registerExt(ExtensionName::eMemoryAllocator, shared_from_this());
      return SFT();
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };
    
    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) {
      auto shared = std::make_shared<MemoryAllocatorObj>(handle, cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  public:

    //
    virtual std::optional<AllocatedMemory>& allocateMemory(cpp21::const_wrap_arg<MemoryRequirements> requirements, std::optional<AllocatedMemory>& allocated, ExtHandle& extHandle) {
      decltype(auto) deviceObj = ZNAMED::context->get<DeviceObj>(this->base);
      auto& device = this->base.as<vk::Device>();
      auto& physicalDevice = deviceObj->getPhysicalDevice();
      auto PDInfoMap = deviceObj->getPhysicalDeviceInfoMap();
      auto memTypeHeap = deviceObj->findMemoryTypeAndHeapIndex(*requirements);

      //
      decltype(auto) exportMemory = infoMap->set(vk::StructureType::eExportMemoryAllocateInfo, vk::ExportMemoryAllocateInfo{
         .handleTypes = requirements->memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{}
      });

      // 
      //auto& allocated = (this->allocated = AllocatedMemory{}).value();
      allocated = AllocatedMemory{
        .memory = device.allocateMemory(infoMap->set(vk::StructureType::eMemoryAllocateInfo, vk::MemoryAllocateInfo{
          .pNext = infoMap->set(vk::StructureType::eMemoryAllocateFlagsInfo, vk::MemoryAllocateFlagsInfo{
            .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedAllocateInfo, vk::MemoryDedicatedAllocateInfo{
              .pNext = requirements->memoryUsage == MemoryUsage::eGpuOnly ? exportMemory.get() : nullptr,
              .image = requirements->dedicated ? requirements->dedicated->image : vk::Image{},
              .buffer = requirements->dedicated ? requirements->dedicated->buffer : vk::Buffer{}
            }).get(),
            .flags = requirements->hasDeviceAddress ? vk::MemoryAllocateFlagBits::eDeviceAddress : vk::MemoryAllocateFlagBits{},
          }).get(),
          .allocationSize = requirements->size,
          .memoryTypeIndex = std::get<0>(memTypeHeap)
        }).ref()),
        .offset = 0ull,
        .size = requirements->size
      };

      //
      if (requirements->memoryUsage == MemoryUsage::eGpuOnly) {
#ifdef _WIN32
        extHandle = device.getMemoryWin32HandleKHR(vk::MemoryGetWin32HandleInfoKHR{ .memory = allocated->memory, .handleType = extMemFlagBits }, deviceObj->getDispatch());
#else
#ifdef __linux__ 
        extHandle = device.getMemoryFdKHR(vk::MemoryGetFdInfoKHR{ .memory = allocated->memory, .handleType = extMemFlagBits }, deviceObj->getDispatch());
#endif
#endif
      };

      // 
      return allocated;
    };

  };

  // 
  inline std::shared_ptr<MemoryAllocatorObj> DeviceObj::createDefaultMemoryAllocator() {
    decltype(auto) allocator = MemoryAllocatorObj::make(this->handle, MemoryAllocatorCreateInfo{});
    this->registerExt<MemoryAllocatorObj>(ExtensionName::eMemoryAllocator, allocator);
    return allocator;
  };

};
