#pragma once

// 
#ifdef __cplusplus
#include "./Core.hpp"

// 
namespace ANAMED {

    //
    class ContextObj : public BaseObj {
    public:
        //using BaseObj;
        using tType = WrapShared<ContextObj>;
        using BaseObj::BaseObj;
        //using SFT = shared_from_this;

        // GPU crash dump tracker using Nsight Aftermath instrumentation
        GpuCrashTracker::MarkerMap markerMap;
        GpuCrashTracker gpuCrashTracker;
        uint64_t frameNumber;

    protected:
        std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{};

        // 
        inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
        inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

    public:

        // 
        ContextObj(Handle const& handle = Handle(0ull, HandleType::eUnknown), cpp21::optional_ref<ContextCreateInfo> cInfo = ContextCreateInfo{}) : BaseObj(handle), cInfo(cInfo), markerMap{}, gpuCrashTracker{ markerMap } {
            //this->construct(cInfo);
        };

        // 
        std::type_info const& type_info() const override {
            return typeid(std::decay_t<decltype(this)>);
        };

        //
        inline static tType make(Handle const& handle = Handle(0ull, HandleType::eUnknown), cpp21::optional_ref<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
            auto shared = std::make_shared<ContextObj>(handle, cInfo);
            shared->construct(cInfo);
            auto wrap = shared->SFT();
            return wrap;
        };

    protected:
        // 
        virtual void construct(cpp21::optional_ref<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
            this->base = 0ull;
            if (cInfo) { this->cInfo = cInfo; };
            //return SFT();
        };
    };

    //
    inline extern WrapShared<ContextObj> context = {};

    // 
    inline static decltype(auto) initialize(cpp21::optional_ref<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
        ANAMED::registerTypes();
        ANAMED::context = ContextObj::make(Handle(0ull, HandleType::eUnknown), cInfo);
        return ANAMED::context;
    };

    //
    inline HMAP_I BaseObj::destroy(Handle const& parent, HMAP_S parentMap) {
        //
        if (parent.value == this->base.value && this->alive) {
            this->tickProcessing();

            // 
            std::decay_t<decltype(*this->handleObjectMap)>::iterator map = handleObjectMap->begin();
            while (map != this->handleObjectMap->end()) {
                std::decay_t<decltype(*(map->second))>& mapc = *(map->second);
                std::decay_t<decltype(*mapc)>::iterator pair = (*mapc).begin();
                while (pair != mapc->end()) {
                    decltype(auto) optPair = pair->second && pair->second->isAlive() ? pair->second->destroy(this->handle, this->handleObjectMap) : HMAP_I{};
                    if (optPair)
                    {
                        pair = optPair.value();
                    }
                    else
                    {
                        pair++;
                    };
                    //if (!optPair) { break; };
                };
                map = this->handleObjectMap->erase(map); // always remove the first elements
            };

            // needs only before deleting main object...
            for (decltype(auto) fn : this->destructors) { if (fn) { (*fn)(this); }; };
            this->destructors = {};
            this->alive = false;

            // erase from parent element
            HMAP_S handleMap = parentMap;
            if (!handleMap && ANAMED::context && ANAMED::context->isAlive()) {
                decltype(auto) baseObj = ANAMED::context->get(this->base);
                if (baseObj) {
                    handleMap = baseObj->getHandleMap();
                };
            };

            //
            if (handleMap && handleMap->size() > 0 && handleMap->find(this->handle.type) != handleMap->end()) {
                decltype(auto) handleValMap = handleMap->at(this->handle.type);
                if (handleValMap) {
                    decltype(auto) interator = (*handleValMap)->find(this->handle.value);
                    if (interator != (*handleValMap)->end()) {
                        return (*handleValMap)->erase(interator);
                    };
                };
            };
        };

        return {};//handleValMap->end();
    };

};

#endif
