#pragma once
#include <LXVC/lxvc.hpp>

// 
int main() {
  lxvc::initialize();

  decltype(auto) instance = lxvc::InstanceObj::make(lxvc::context, lxvc::InstanceCreateInfo{

  });

  decltype(auto) device = lxvc::DeviceObj::make(instance, lxvc::DeviceCreateInfo{

  });

};
