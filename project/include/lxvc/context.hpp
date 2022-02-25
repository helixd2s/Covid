#pragma once

// 
#include "./Core.hpp"

// 
namespace lxvc {
  
  //
  class ContextObj : std::enable_shared_from_this<ContextObj> {
  public:
    using tType = std::shared_ptr<ContextObj>;
    //using SFT = shared_from_this;

    std::optional<ContextCreateInfo> cInfo = {};

    // 
    inline decltype(auto) SFT() { return shared_from_this(); };

    // 
    ContextObj(std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      this->construct(cInfo);
    };

    // 
    virtual tType construct(std::optional<ContextCreateInfo> cInfo = ContextCreateInfo{}) {
      return SFT();
    };
  };
  
};