#pragma once
#include <LXVC/lxvc.hpp>
#include <GLFW/glfw3.h>

// 
void error(int errnum, const char* errmsg)
{
  std::cerr << errnum << ": " << errmsg << std::endl;
};

//
struct UniformData {
  uint32_t imageIndices[4] = { 0u,0u,0u,0u };
  uint32_t textureIndices[4] = { 0u,0u,0u,0u };
  uint32_t currentImage = 0u;
  uint32_t reserved = 0u;
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
  decltype(auto) uniformData = UniformData{};

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
  std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> stageMaps = {};
  stageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./test.vert.spv");
  stageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./test.frag.spv");

  //
  decltype(auto) graphics = lxvc::PipelineObj::make(device.with(0u), lxvc::PipelineCreateInfo{
    .layout = descriptions.as<vk::PipelineLayout>(),
    .graphics = lxvc::GraphicsPipelineCreateInfo{
      .stageCodes = stageMaps
    }
  });


  //
  uint64_t address = device.as<vk::Device>().getBufferAddress(vk::BufferDeviceAddressInfo{
    .buffer = buffer
  });

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
  decltype(auto) framebuffer = lxvc::FramebufferObj::make(device.with(0u), lxvc::FramebufferCreateInfo{
    .extent = swapchain->getRenderArea().extent,
    .layout = descriptions.as<vk::PipelineLayout>()
  });

  //
  decltype(auto) readySemaphoreInfos = swapchain->getReadySemaphoreInfos();
  decltype(auto) presentSemaphoreInfos = swapchain->getPresentSemaphoreInfos();

  // 
  decltype(auto) imageIndices = swapchain->getImageViewIndices();
  memcpy(uniformData.imageIndices, imageIndices.data(), std::min(imageIndices.size(), 4ull) * sizeof(uint32_t));

  // 
  decltype(auto) textureIndices = framebuffer->getImageViewIndices();
  memcpy(uniformData.textureIndices, textureIndices.data(), std::min(textureIndices.size(), 4ull) * sizeof(uint32_t));

  //
  decltype(auto) renderArea = swapchain->getRenderArea();
  decltype(auto) qfAndQueue = lxvc::QueueGetInfo{ 0u, 0u };

  //
  uniformData.currentImage = uint32_t(imageIndices.size())-1u;

  // 
  while (!glfwWindowShouldClose(window)) { // 
    glfwPollEvents();

    // 
    decltype(auto) semIndex = (uniformData.currentImage + 1u) % imageIndices.size();
    decltype(auto) acquired = device.as<vk::Device>().acquireNextImage2KHR(vk::AcquireNextImageInfoKHR{ .swapchain = swapchain.as<vk::SwapchainKHR>(), .timeout = 10000000000, .semaphore = presentSemaphoreInfos[semIndex].semaphore, .deviceMask = 0x1u });

    //
    swapchain->switchToReady(semIndex = uniformData.currentImage = acquired, qfAndQueue);

    // 
    decltype(auto) uniformFence = descriptions->executeUniformUpdateOnce(lxvc::UniformDataSet{
      .data = std::span<char8_t>((char8_t*)&uniformData, sizeof(UniformData)),
      .region = lxvc::DataRegion{0ull, sizeof(UniformData)},
      .info = qfAndQueue
    });

    //
    framebuffer->switchToAttachment();

    //
    decltype(auto) graphicsFence = graphics->executeGraphicsOnce(lxvc::ExecuteGraphicsInfo{
      .framebuffer = framebuffer.as<uintptr_t>(),
      .multiDrawInfo = std::vector<vk::MultiDrawInfoEXT>{ vk::MultiDrawInfoEXT{ .firstVertex = 0u, .vertexCount = 6u } },
      .layout = descriptions.as<vk::PipelineLayout>(),
      .info = qfAndQueue,
    });

    //
    framebuffer->switchToShaderRead();

    //
    decltype(auto) computeFence = compute->executeComputeOnce(lxvc::ExecuteComputeInfo{
      .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 256u), renderArea.extent.height, 1u},
      .layout = descriptions.as<vk::PipelineLayout>(),
      .info = qfAndQueue,
      .waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{presentSemaphoreInfos[semIndex]},
      .signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{readySemaphoreInfos[semIndex]},
    });


    //
    decltype(auto) presentFence = swapchain->switchToPresent(uniformData.currentImage, qfAndQueue);

    //
    std::get<0u>(presentFence).get();
    device->deleteTrash();

    //
    decltype(auto) result = device->getQueue(qfAndQueue).presentKHR(vk::PresentInfoKHR{
      .waitSemaphoreCount = 1u,
      .pWaitSemaphores = &readySemaphoreInfos[semIndex].semaphore,
      .swapchainCount = 1u,
      .pSwapchains = &swapchain.as<vk::SwapchainKHR>(),
      .pImageIndices = &uniformData.currentImage
    });
  };

  // 
  return 0;
};

