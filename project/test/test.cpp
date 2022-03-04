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
  decltype(auto) descriptions = lxvc::DescriptorsObj::make(device.with(0u), lxvc::DescriptorsCreateInfo{

  });

  //
  decltype(auto) buffer = lxvc::ResourceObj::make(device, lxvc::ResourceCreateInfo{
    .bufferInfo = lxvc::BufferCreateInfo{
      .type = lxvc::BufferType::eDevice,
      .size = 1024ull,
    }
  }).as<vk::Buffer>();

  //
  decltype(auto) uploader = lxvc::UploaderObj::make(device, lxvc::UploaderCreateInfo{

  });

  //
  decltype(auto) compute = lxvc::PipelineObj::make(device.with(0u), lxvc::PipelineCreateInfo{
    .layout = descriptions.as<vk::PipelineLayout>(),
    .compute = lxvc::ComputePipelineCreateInfo{
      .code = cpp21::readBinaryU32("./test.comp.spv")
    }
  });


  //
  uint64_t address = device.as<vk::Device>().getBufferAddress(vk::BufferDeviceAddressInfo{
    .buffer = buffer
  });

  // 
  auto uniformFence = descriptions->executeUniformUpdateOnce(lxvc::UniformDataSet{
    .data = std::span<char8_t>((char8_t*)&address, 8ull),
    .region = lxvc::DataRegion{0ull, 8ull},
    .info = lxvc::QueueGetInfo{0u, 0u}
  });


  //
  auto computeFence = compute->executeComputeOnce(lxvc::ExecuteComputeInfo{
    .dispatch = vk::Extent3D{1u,1u,1u},
    .layout = descriptions.as<vk::PipelineLayout>(),
    .info = lxvc::QueueGetInfo{ 0u, 0u }
  });


  //
  std::vector<uint32_t> results(256ull); // data_view - vector without changing pointer address!
  std::span<char8_t> dataview{ (char8_t*)results.data(), 1024ull };


  //
  auto uploadeFence = uploader->executeDownloadFromBufferOnce(lxvc::BufferRegion{ buffer, lxvc::DataRegion{0ull, 1024ull} }, dataview);
  auto awaited = std::get<0u>(uploadeFence).get();


  //
  auto framebuffer = lxvc::FramebufferObj::make(device.with(0u), lxvc::FramebufferCreateInfo{
    .extent = {1280u, 720u},
    .layout = descriptions.as<vk::PipelineLayout>()
  });

  //
  framebuffer->switchToShaderRead();
  framebuffer->switchToAttachment();

  //
  decltype(auto) buf = (std::cout << "");
  for (decltype(auto) rc : results) {
    buf << rc << ", ";
  };
  buf << std::endl;


};

