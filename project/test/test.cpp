#pragma once
#include <LXVC/lxvc.hpp>
#include <GLFW/glfw3.h>

// 
void error(int errnum, const char* errmsg)
{
  std::cerr << errnum << ": " << errmsg << std::endl;
};


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
  decltype(auto) uniformFence = descriptions->executeUniformUpdateOnce(lxvc::UniformDataSet{
    .data = std::span<char8_t>((char8_t*)&address, 8ull),
    .region = lxvc::DataRegion{0ull, 8ull},
    .info = lxvc::QueueGetInfo{0u, 0u}
  });


  //
  decltype(auto) computeFence = compute->executeComputeOnce(lxvc::ExecuteComputeInfo{
    .dispatch = vk::Extent3D{1u,1u,1u},
    .layout = descriptions.as<vk::PipelineLayout>(),
    .info = lxvc::QueueGetInfo{ 0u, 0u }
  });


  //
  std::vector<uint32_t> results(256ull); // data_view - vector without changing pointer address!
  std::span<char8_t> dataview{ (char8_t*)results.data(), 1024ull };


  //
  decltype(auto) uploadeFence = uploader->executeDownloadFromBufferOnce(lxvc::BufferRegion{ buffer, lxvc::DataRegion{0ull, 1024ull} }, dataview);
  decltype(auto) awaited = std::get<0u>(uploadeFence).get();


  //
  decltype(auto) framebuffer = lxvc::FramebufferObj::make(device.with(0u), lxvc::FramebufferCreateInfo{
    .extent = {1280u, 720u},
    .layout = descriptions.as<vk::PipelineLayout>()
  });

  //
  //framebuffer->switchToShaderRead();
  //framebuffer->switchToAttachment();

  //
  decltype(auto) buf = (std::cout << "");
  for (decltype(auto) rc : results) {
    buf << rc << ", ";
  };
  buf << std::endl;



  //
  glfwSetErrorCallback(error);
  glfwInit();

  // 
  if (GLFW_FALSE == glfwVulkanSupported()) {
    glfwTerminate(); return -1;
  };

  // 
  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  //
  uint32_t WIDTH = 640u, HEIGHT = 360u;

  // 
  float xscale = 1.f, yscale = 1.f;
  GLFWmonitor* primary = glfwGetPrimaryMonitor();
  glfwGetMonitorContentScale(primary, &xscale, &yscale);
  uint32_t SC_WIDTH = WIDTH * xscale, SC_HEIGHT = HEIGHT * yscale;

  //
  vk::SurfaceKHR surface = {};
  std::string title = "LXVC.TEON.A";
  decltype(auto) window = glfwCreateWindow(SC_WIDTH, SC_HEIGHT, title.c_str(), nullptr, nullptr);
  glfwCreateWindowSurface(instance.as<VkInstance>(), window, nullptr, (VkSurfaceKHR*)&surface);

  //
  decltype(auto) swapchain = lxvc::SwapchainObj::make(device, lxvc::SwapchainCreateInfo{
    .surface = surface,
    .layout = descriptions.as<vk::PipelineLayout>()
  });

  // 
  while (!glfwWindowShouldClose(window)) { // 
    glfwPollEvents();

  };

  // 
  return 0;
};

