#pragma once
#include <LXVC/lxvc.hpp>

// 
int main() {
  lxvc::initialize();

  // first cherep
  decltype(auto) instance = lxvc::InstanceObj::make(lxvc::context, lxvc::InstanceCreateInfo{

  });

  // second cherep
  decltype(auto) device = lxvc::DeviceObj::make(instance, lxvc::DeviceCreateInfo{

  });

  // final cherep for today
  decltype(auto) descriptions = lxvc::DescriptorsObj::make(device, lxvc::DescriptorsCreateInfo{

  });

  //
  uint32_t variable = 32u;

  // no, that is really final cherep...
  descriptions->setUniformData(lxvc::UniformDataSet{
    .data = cpp21::data_view<char8_t>((char8_t*)&variable, 4ull),
    .region = lxvc::DataRegion{0ull, 4ull},
    .info = lxvc::QueueGetInfo{0u, 0u}
  });

};
