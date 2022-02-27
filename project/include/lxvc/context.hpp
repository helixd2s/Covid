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
    std::optional<ContextCreateInfo> cInfo = {};

    // 
    inline decltype(auto) SFT() { using T = std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };
    inline decltype(auto) SFT() const { using T = const std::decay_t<decltype(*this)>; return WrapShared<T>(std::dynamic_pointer_cast<T>(shared_from_this())); };

  public: 
    // 
    ContextObj(std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      this->construct(cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

    //
    inline static tType make(Handle const& handle, std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      return WrapShared<ContextObj>(std::make_shared<ContextObj>(handle, cInfo));
    };

  protected: 
    // 
    virtual void construct(std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      this->base = 0ull;
      //return SFT();
    };
  };

  //
  inline extern WrapShared<ContextObj> context = { {} };

  // 
  inline static decltype(auto) initialize(std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
    lxvc::registerTypes();
    lxvc::context = ContextObj::make(Handle(0ull), cInfo);
    return lxvc::context;
  };

};
