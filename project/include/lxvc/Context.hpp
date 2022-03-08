#pragma once

// 
#include "./Core.hpp"

// 
namespace lxvc {
  
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
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

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
    inline static tType make(Handle const& handle, cpp21::const_wrap_arg<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      auto shared = std::make_shared<ContextObj>(handle, cInfo);
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
    lxvc::registerTypes();
    lxvc::context = ContextObj::make(Handle(0ull), cInfo);
    return lxvc::context;
  };

};
