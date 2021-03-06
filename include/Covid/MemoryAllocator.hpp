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
        virtual void construct(std::shared_ptr<DeviceObj> deviceObj = {}, cpp21::optional_ref<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) {
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
        MemoryAllocatorObj(WrapShared<DeviceObj> deviceObj = {}, cpp21::optional_ref<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : BaseObj(std::move(deviceObj->getHandle())), cInfo(cInfo) {
            this->base = deviceObj->getHandle();
            //this->construct(deviceObj, cInfo);
        };

        // 
        MemoryAllocatorObj(Handle const& handle, cpp21::optional_ref<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) : BaseObj(handle), cInfo(cInfo) {
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
        inline static tType make(Handle const& handle, cpp21::optional_ref<MemoryAllocatorCreateInfo> cInfo = MemoryAllocatorCreateInfo{}) {
            auto shared = std::make_shared<MemoryAllocatorObj>(handle, cInfo);
            shared->construct(ANAMED::context->get<DeviceObj>(handle).shared(), cInfo);
            auto wrap = shared->registerSelf();
            return wrap;
        };

    public:

        //
        virtual vk::Buffer createBuffer(std::shared_ptr<MSS> infoMap, std::vector<std::shared_ptr<std::function<DFun>>>& destructors) {
            auto& device = this->base.as<vk::Device>();
            decltype(auto) bufferInfo = infoMap->get<vk::BufferCreateInfo>(vk::StructureType::eBufferCreateInfo);
            decltype(auto) externalInfo = infoMap->get<vk::ExternalMemoryBufferCreateInfo>(vk::StructureType::eExternalMemoryBufferCreateInfo);
            if (externalInfo) { bufferInfo->pNext = externalInfo.get(); } else { bufferInfo->pNext = nullptr;};

            decltype(auto) handle = handleResult(device.createBuffer(bufferInfo.value()));
            destructors.push_back(std::make_shared<std::function<DFun>>([device, buffer = handle](BaseObj const*) {
                device.destroyBuffer(buffer);
                }));
            return handle;
        };

        //
        virtual vk::Image createImage(std::shared_ptr<MSS> infoMap, std::vector<std::shared_ptr<std::function<DFun>>>& destructors) {
            auto& device = this->base.as<vk::Device>();
            decltype(auto) handle = handleResult(device.createImage(infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo)));
            destructors.push_back(std::make_shared<std::function<DFun>>([device, image = handle](BaseObj const*) {
                device.destroyImage(image);
                }));
            return handle;
        };


        //
        virtual vk::Buffer createBufferAndAllocateMemory(std::shared_ptr<AllocatedMemory>& allocated, cpp21::optional_ref<MemoryRequirements> requirements, std::shared_ptr<MSS> infoMap, cpp21::optional_ref<std::vector<std::shared_ptr<std::function<DFun>>>> destructors = {}) {
            decltype(auto) handle = this->createBuffer(infoMap, destructors);
            requirements->dedicated = DedicatedMemory{ .buffer = handle };
            allocated = this->allocateMemory(allocated, requirements, infoMap, destructors);
            return handle;
        };

        //
        virtual vk::Image createImageAndAllocateMemory(std::shared_ptr<AllocatedMemory>& allocated, cpp21::optional_ref<MemoryRequirements> requirements, std::shared_ptr<MSS> infoMap, cpp21::optional_ref<std::vector<std::shared_ptr<std::function<DFun>>>> destructors = {}) {
            decltype(auto) handle = this->createImage(infoMap, destructors);
            requirements->dedicated = DedicatedMemory{ .image = handle };
            allocated = this->allocateMemory(allocated, requirements, infoMap, destructors);
            return handle;
        };

        //
        virtual std::shared_ptr<AllocatedMemory> allocateMemory(std::shared_ptr<AllocatedMemory>& allocated, cpp21::optional_ref<MemoryRequirements> requirements, std::shared_ptr<MSS> infoMap, cpp21::optional_ref<std::vector<std::shared_ptr<std::function<DFun>>>> destructors = {}) {
            decltype(auto) deviceObj = ANAMED::context->get<DeviceObj>(this->base);
            auto& device = this->base.as<vk::Device>();
            auto& physicalDevice = deviceObj->getPhysicalDevice();
            auto PDInfoMap = deviceObj->getPhysicalDeviceInfoMap();
            auto bufferInfo = infoMap->get<vk::BufferCreateInfo>(vk::StructureType::eBufferCreateInfo);
            auto imageInfo = infoMap->get<vk::ImageCreateInfo>(vk::StructureType::eImageCreateInfo);

            //
            decltype(auto) exportMemory = infoMap->set(vk::StructureType::eExportMemoryAllocateInfo, vk::ExportMemoryAllocateInfo{
              .handleTypes = requirements->memoryUsage == MemoryUsage::eGpuOnly ? extMemFlags : vk::ExternalMemoryHandleTypeFlags{}
                });

            // 
            decltype(auto) memReqInfo2 = infoMap->set(vk::StructureType::eMemoryRequirements2, vk::MemoryRequirements2{
              .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedRequirements, vk::MemoryDedicatedRequirements{}).get()
                });

            //
            if (requirements->dedicated) {
                if (requirements->dedicated->image) { //
                    device.getImageMemoryRequirements2(infoMap->set(vk::StructureType::eImageMemoryRequirementsInfo2, vk::ImageMemoryRequirementsInfo2{
                      .image = requirements->dedicated->image
                        }).get(), memReqInfo2.get());
                };
                if (requirements->dedicated->buffer) { //
                    device.getBufferMemoryRequirements2(infoMap->set(vk::StructureType::eBufferMemoryRequirementsInfo2, vk::BufferMemoryRequirementsInfo2{
                      .buffer = requirements->dedicated->buffer
                        }).get(), memReqInfo2.get());
                };
            };

            //
            bool hasDBA = false;
            if (bufferInfo) { hasDBA = !!(bufferInfo->usage & vk::BufferUsageFlagBits::eShaderDeviceAddress); };
            if (!requirements->requirements) { requirements->requirements = memReqInfo2->memoryRequirements; };

            // 
            if (requirements->requirements && requirements->requirements->size > 0) {
                auto memTypeHeap = deviceObj->findMemoryTypeAndHeapIndex(*requirements);
                //auto& allocated = (this->allocated = AllocatedMemory{}).value();
                *allocated = AllocatedMemory{
                  .memory = handleResult(device.allocateMemory(infoMap->set(vk::StructureType::eMemoryAllocateInfo, vk::MemoryAllocateInfo{
                    .pNext = infoMap->set(vk::StructureType::eMemoryAllocateFlagsInfo, vk::MemoryAllocateFlagsInfo{
                      .pNext = infoMap->set(vk::StructureType::eMemoryDedicatedAllocateInfo, vk::MemoryDedicatedAllocateInfo{
                        .pNext = requirements->memoryUsage == MemoryUsage::eGpuOnly ? exportMemory.get() : nullptr,
                        .image = requirements->dedicated ? requirements->dedicated->image : vk::Image{},
                        .buffer = requirements->dedicated ? requirements->dedicated->buffer : vk::Buffer{}
                      }).get(),
                      .flags = hasDBA ? vk::MemoryAllocateFlagBits::eDeviceAddress : vk::MemoryAllocateFlagBits{},
                    }).get(),
                    .allocationSize = requirements->requirements->size,
                    .memoryTypeIndex = std::get<0>(memTypeHeap)
                  }).value())),
                  .offset = 0ull,
                  .size = requirements->requirements->size
                };

                //
                //device.setMemoryPriorityEXT(allocated->memory, 1.f, deviceObj->getDispatch());

                //
                if (requirements->dedicated) {
                    if (requirements->dedicated->image) {
                        std::vector<vk::BindImageMemoryInfo> bindInfos = { vk::BindImageMemoryInfo{
                          .image = requirements->dedicated->image, .memory = allocated->memory, .memoryOffset = allocated->offset
                        } };
                        device.bindImageMemory2(bindInfos);
                    };
                    if (requirements->dedicated->buffer) {
                        std::vector<vk::BindBufferMemoryInfo> bindInfos = { vk::BindBufferMemoryInfo{
                          .buffer = requirements->dedicated->buffer, .memory = allocated->memory, .memoryOffset = allocated->offset
                        } };
                        device.bindBufferMemory2(bindInfos);
                    };
                };

                //
                if (requirements->memoryUsage == MemoryUsage::eGpuOnly) {
#ifdef _WIN32
                    allocated->extHandle = handleResult(device.getMemoryWin32HandleKHR(vk::MemoryGetWin32HandleInfoKHR{ .memory = allocated->memory, .handleType = extMemFlagBits }, deviceObj->getDispatch()));
#else
#ifdef __linux__ 
                    allocated->extHandle = handleResult(device.getMemoryFdKHR(vk::MemoryGetFdInfoKHR{ .memory = allocated->memory, .handleType = extMemFlagBits }, deviceObj->getDispatch()));
#endif
#endif
                };

                //
                if (requirements->memoryUsage != MemoryUsage::eGpuOnly) {
                    allocated->mapped = handleResult(device.mapMemory(allocated->memory, allocated->offset, requirements->requirements->size));
                };

                //
                allocated->destructor = std::make_shared<std::function<DFun>>([device, &memory = allocated->memory, &mapped = allocated->mapped](BaseObj const*) {
                    if (memory) {
                        if (mapped) {
                            device.unmapMemory(memory);
                            mapped = nullptr;
                        };
                        device.freeMemory(memory);
                    };
                    memory = vk::DeviceMemory{};
                });
                if (destructors) {
                    destructors->push_back(allocated->destructor);
                };
                //};
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
