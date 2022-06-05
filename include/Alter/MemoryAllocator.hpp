#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"
#include "./Instance.hpp"
#include "./Device.hpp"

// 
namespace ANAMED {

  // 
  class MemoryAllocatorObj : public BaseObj {
  public:
    using BaseObj::BaseObj;
    using tType = WrapShared<MemoryAllocatorObj>;

  protected:
    vk::DispatchLoaderDynamic dispatch = {};
    std::optional<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{};

  protected:

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    // 
    virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::carg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) {
      //this->handle = Handle(uintptr_t(this), HandleType::eMemoryAllocator);
      this->handle = Handle(uintptr_t(this), HandleType::eExtension);
      
      // 
      if (!this->cInfo->extInfoMap) {
        this->cInfo->extInfoMap = std::make_shared<EXIF>();
      };
    };

  public:

    //
    ~MemoryAllocatorObj() {
      this->tickProcessing();
    };

    // 
    MemoryAllocatorObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::carg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
      this->base = deviceObj->getHandle();
      //this->construct(deviceObj, cInfo);
    };

    // 
    MemoryAllocatorObj(cpp21::carg<Handle> handle, cpp21::carg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
      //this->construct(ANAMED::context->get<DeviceObj>(this->base = handle), cInfo);
    };

    //
    virtual tType registerSelf() {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
      deviceObj->registerExt(ExtensionName::eMemoryAllocator, shared_from_this());
      return SFT();
    };

    // 
     std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };
    
    //
    inline static tType make(cpp21::carg<Handle> handle, cpp21::carg<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) {
      auto shared = std::make_shared<MemoryAllocatorObj>(handle, cInfo);
      shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
      auto wrap = shared->registerSelf();
      return wrap;
    };

  public:

    //
    virtual std::shared_ptr<AllocatedMemory> allocateMemory(cpp21::carg<MemoryRequirements> requirements, std::shared_ptr<AllocatedMemory> allocated, ExtHandle& extHandle, std::shared_ptr<EXIF> extInfoMap, void*& mapped, std::vector<std::shared_ptr<std::function<DFun>>>& destructors) {
      decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
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
      *allocated = AllocatedMemory{
        .memory = device.allocateMemory(infoMap->set(vk::StructureType::eMemoryAllocateInfo, vk::MemoryAllocateInfo{
          .pNext = infoMap->set(vk::StructureType::eMemoryAllocateFlagsInfo, vk::MemoryAllocateFlagsInfo{
            .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedAllocateInfo, vk::MemoryDedicatedAllocateInfo{
              .pNext = requirements->memoryUsage == MemoryUsage::eGpuOnly ? exportMemory.get() : nullptr,
              .image = requirements->dedicated ? requirements->dedicated->image : vk::Image{},
              .buffer = requirements->dedicated ? requirements->dedicated->buffer : vk::Buffer{}
            }).get(),
            .flags = requirements->hasDeviceAddress ? vk::MemoryAllocateFlagBits::eDeviceAddress : vk::MemoryAllocateFlagBits{},
          }).get(),
          .allocationSize = requirements->requirements.size,
          .memoryTypeIndex = std::get<0>(memTypeHeap)
        }).ref()),
        .offset = 0ull,
        .size = requirements->requirements.size
      };

      //
      device.setMemoryPriorityEXT(allocated->memory, 1.f, deviceObj->getDispatch());

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
      if (requirements->memoryUsage != MemoryUsage::eGpuOnly) {
        mapped = allocated->mapped = device.mapMemory(allocated->memory, allocated->offset, requirements->requirements.size);
      };

      //
      if (requirements->needsDestructor) {
        destructors.push_back(allocated->destructor = std::make_shared<std::function<DFun>>([device, &memory = allocated->memory, &mapped = allocated->mapped](BaseObj const*) {
          //device.waitIdle();
          if (memory) {
            if (mapped) {
              device.unmapMemory(memory);
              mapped = nullptr;
            };
            device.freeMemory(memory);
          };
          memory = vk::DeviceMemory{};
        }));
      };

      // 
      return allocated;
    };

  };

  // 
  inline std::shared_ptr<MemoryAllocatorObj> DeviceObj::createDefaultMemoryAllocator() {
    decltype(auto) allocator = MemoryAllocatorObj::make(this->handle, MemoryAllocatorCreateInfo{});
    //this->registerExt<MemoryAllocatorObj>(ExtensionName::eMemoryAllocator, allocator);
    return allocator.shared();
  };

};
#endif
