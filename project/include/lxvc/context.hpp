#pragma once

// 
#include "./Core.hpp"

// 
namespace lxvc {
  
  //
  class ContextObj : public BaseObj {
  public:
    //using BaseObj;
    using tType = std::shared_ptr<ContextObj>;
    using BaseObj::BaseObj;
    //using SFT = shared_from_this;

  protected:
    std::optional<ContextCreateInfo> cInfo = {};

    // 
    inline decltype(auto) SFT() { return std::dynamic_pointer_cast<std::decay_t<decltype(*this)>>(shared_from_this()); };
    inline decltype(auto) SFT() const { return std::dynamic_pointer_cast<const std::decay_t<decltype(*this)>>(shared_from_this()); };

  public: 
    // 
    ContextObj(std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      this->construct(cInfo);
    };

    // 
    virtual std::type_info const& type_info() const override {
      return typeid(std::decay_t<decltype(this)>);
    };

  protected: 
    // 
    virtual tType construct(std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      this->base = 0ull;
      return SFT();
    };
  };

  //
  inline static std::shared_ptr<ContextObj> context = {};

  // 
  inline static decltype(auto) initialize(std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
    lxvc::context = std::make_shared<ContextObj>(cInfo);
    lxvc::registerTypes();
    return lxvc::context;
  };

};
