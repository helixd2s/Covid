#pragma once

// 
#include "./Core.hpp"

// 
namespace ZNAMED {
  
  //
  class ContextObj : public BaseObj {
  public:
    //using BaseObj;
    using tType = WrapShared<ContextObj>;
    using BaseObj::BaseObj;
    //using SFT = shared_from_this;

  protected:
    std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::const_pointer_cast<T>(std::dynamic_pointer_cast<T const>(shared_from_this()))); };

  public: 
    // 
    ContextObj(cpp21::const_wrap_arg<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      this->construct(cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    inline static tType make(cpp21::const_wrap_arg<Handle> handle, cpp21::const_wrap_arg<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      auto shared = std::make_shared<ContextObj>(cInfo);
      auto wrap = shared->SFT();//->registerSelf();
      return wrap;
    };

  protected: 
    // 
    virtual void construct(cpp21::const_wrap_arg<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      this->base = 0ull;
      if (cInfo) { this->cInfo = cInfo; };
      //return SFT();
    };
  };

  //
  inline extern WrapShared<ContextObj> context = { {} };

  // 
  inline static decltype(auto) initialize(cpp21::const_wrap_arg<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
    ZNAMED::registerTypes();
    ZNAMED::context = ContextObj::make(Handle(0ull), cInfo);
    return ZNAMED::context;
  };

  //
  inline std::optional<UNORDER_MAP<uintptr_t, std::shared_ptr<BaseObj>>::iterator> BaseObj::destroy(Handle const& parent, HMAP_T* parentMap) {
    //
    if (parent.value == this->base.value && this->alive) {
      //
      this->tickProcessing();

      // 
      std::decay_t<decltype(handleObjectMap)>::iterator map = handleObjectMap.begin();
      while (map != this->handleObjectMap.end()) {
        std::decay_t<decltype(*(map->second))>& mapc = *(map->second);
        std::decay_t<decltype(mapc)>::iterator pair = mapc.begin();
        while (pair != mapc.end() && pair->second && pair->second->isAlive()) {
          decltype(auto) optPair = pair->second->destroy(this->handle, &this->handleObjectMap);
          if (optPair) { pair = optPair.value(); } else { pair++; };
          //pair = mapc.erase(pair);
        };
        map = handleObjectMap.erase(map);
      };

      // needs only before deleting main object...
      for (decltype(auto) fn : this->destructors) { fn(this); };
      this->destructors = {};

      // erase from parent element
      bool wasAlive = this->alive; this->alive = false;
      //decltype(auto) nullMap = HMAP_T{};
      auto* handleMap = parentMap;
      if (wasAlive && !handleMap && ZNAMED::context && ZNAMED::context->isAlive()) {
        decltype(auto) baseObj = ZNAMED::context->get(this->base);
        if (baseObj) {
          handleMap = &baseObj->getHandleMap();
        };
      };
      if (wasAlive && handleMap && handleMap->size() > 0) {
        auto& handleValMap = handleMap->at(this->handle.type);
        decltype(auto) interator = handleValMap->find(this->handle.value);
        if (interator != handleValMap->end()) {
          return handleValMap->erase(interator);
        };
      };
    };

    return {};//handleValMap->end();
  };

};
