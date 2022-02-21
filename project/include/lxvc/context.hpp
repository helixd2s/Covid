#pragma once

// 
#include "./core.hpp"

// 
namespace lxvc {
  
  //
  class ContextObj : std::enable_shared_from_this<ContextObj> {
  public:
    using tType = std::shared_ptr<ContextObj>;
    //using SFT = shared_from_this;

    ContextCreateInfo cInfo = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    ContextObj(cpp21::uni_arg<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      this->construct(cInfo);
    };

    // 
    virtual tType construct(cpp21::uni_arg<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      return SFT();
    };
  };
  
};