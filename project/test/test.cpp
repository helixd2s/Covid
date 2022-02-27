#pragma once
#include <LXVC/lxvc.hpp>

// 
int main() {
  lxvc::initialize();

  decltype(auto) instance = std::make_shared<lxvc::InstanceObj>(lxvc::context, lxvc::InstanceCreateInfo{

  })->registerSelf();

};
