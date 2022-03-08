#pragma once
#include <LXVC/lxvc.hpp>
#include <GLFW/glfw3.h>
#include <windows.h>
#include "renderdoc_app.h"
#include <eh.h>

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

  // Be sure to enable "Yes with SEH Exceptions (/EHa)" in C++ / Code Generation;
  _set_se_translator([](unsigned int u, EXCEPTION_POINTERS* pExp) {
    std::string error = "SE Exception: ";
    switch (u) {
    case 0xC0000005:
      error += "Access Violation";
      break;
    default:
      char result[11];
      sprintf_s(result, 11, "0x%08X", u);
      error += result;
    };
    throw std::exception(error.c_str());
  });

  //
  RENDERDOC_API_1_1_2* rdoc_api = NULL;

#ifdef _WIN32
  // At init, on windows
  if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
  {
    pRENDERDOC_GetAPI RENDERDOC_GetAPI =
      (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
    assert(ret == 1);
  }
#else
#ifdef __linux__
  // At init, on linux/android.
  // For android replace librenderdoc.so with libVkLayer_GLES_RenderDoc.so
  if (void* mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
  {
    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
    assert(ret == 1);
  }
#endif
#endif

  //
  if (rdoc_api) rdoc_api->SetCaptureOptionU32(eRENDERDOC_Option_DebugOutputMute, true);


  //
  lxvc::initialize();


  //
  //std::cout << "We running experimental renderer... continue?" << std::endl;
  //system("PAUSE");


  // first cherep
  decltype(auto) instance = lxvc::InstanceObj::make(lxvc::context, lxvc::InstanceCreateInfo{

  });

  // second cherep
  decltype(auto) device = lxvc::DeviceObj::make(instance, lxvc::DeviceCreateInfo{
    .physicalDeviceGroupIndex = 0u,
    .physicalDeviceIndex = 0u
  });

  // final cherep for today
  decltype(auto) descriptions = lxvc::DescriptorsObj::make(device.with(0u), lxvc::DescriptorsCreateInfo{

  });

  //
  decltype(auto) uniformData = UniformData{};

  //
  decltype(auto) buffer = lxvc::ResourceObj::make(device, lxvc::ResourceCreateInfo{
    .bufferInfo = lxvc::BufferCreateInfo{
      .type = lxvc::BufferType::eStorage,
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
  decltype(auto) qfAndQueue = lxvc::QueueGetInfo{ 0u, 0u };
  std::array<lxvc::FenceType, 4> fences = {};

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
    .layout = descriptions.as<vk::PipelineLayout>(),
    .surface = surface,
    .info = qfAndQueue
  });

  //
  decltype(auto) framebuffer = lxvc::FramebufferObj::make(device.with(0u), lxvc::FramebufferCreateInfo{
    .layout = descriptions.as<vk::PipelineLayout>(),
    .extent = swapchain->getRenderArea().extent,
    .info = qfAndQueue
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
  uniformData.currentImage = uint32_t(imageIndices.size()) - 1u;

  //
  std::cout << "" << std::endl;
  //system("PAUSE");

  // 
  while (!glfwWindowShouldClose(window)) { // 
    glfwPollEvents();

    //
    if (rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);

    // 
    decltype(auto) semIndex = (uniformData.currentImage + 1u) % imageIndices.size();
    decltype(auto) acquired = device.as<vk::Device>().acquireNextImage2KHR(vk::AcquireNextImageInfoKHR{ .swapchain = swapchain.as<vk::SwapchainKHR>(), .timeout = 1000*1000*1000, .semaphore = presentSemaphoreInfos[semIndex].semaphore, .deviceMask = 0x1u });

    //
    swapchain->switchToReady(semIndex = uniformData.currentImage = acquired, qfAndQueue);

    // 
    decltype(auto) uniformFence = descriptions->executeUniformUpdateOnce(lxvc::UniformDataSet{
      .writeInfo = lxvc::UniformDataWriteSet{
        .region = lxvc::DataRegion{0ull, sizeof(UniformData)},
        .data = std::span<char8_t>((char8_t*)&uniformData, sizeof(UniformData)),
      },
      .submission = lxvc::SubmissionInfo{
        .info = qfAndQueue,
      }
    });

    // 
    //framebuffer->switchToAttachment(qfAndQueue);

    //
    decltype(auto) graphicsFence = graphics->executePipelineOnce(lxvc::ExecutePipelineInfo{
      .graphics = lxvc::WriteGraphicsInfo{
        .framebuffer = framebuffer.as<uintptr_t>(),
        .multiDrawInfo = std::vector<vk::MultiDrawInfoEXT>{ vk::MultiDrawInfoEXT{.firstVertex = 0u, .vertexCount = 6u } },
        .layout = descriptions.as<vk::PipelineLayout>(),
      },
      .submission = lxvc::SubmissionInfo{
        .info = qfAndQueue,
      }
    });

    //
    //framebuffer->switchToShaderRead(qfAndQueue);

    //
    decltype(auto) computeFence = compute->executePipelineOnce(lxvc::ExecutePipelineInfo{
      .compute = lxvc::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 256u), renderArea.extent.height, 1u},
        .layout = descriptions.as<vk::PipelineLayout>(),
      },
      
      .submission = lxvc::SubmissionInfo{
        .info = qfAndQueue,
        .waitSemaphores = std::vector<vk::SemaphoreSubmitInfo>{presentSemaphoreInfos[semIndex]},
        .signalSemaphores = std::vector<vk::SemaphoreSubmitInfo>{readySemaphoreInfos[semIndex]},
      }
    });

    //
    auto& fence = fences[semIndex]; 
    if (fence) { decltype(auto) unleak = std::get<0u>(*fence); }; device->tickProcessing();
    fence = swapchain->switchToPresent(semIndex, qfAndQueue);
    
    //
    decltype(auto) result = device->getQueue(qfAndQueue).presentKHR(vk::PresentInfoKHR{
      .waitSemaphoreCount = 1u,
      .pWaitSemaphores = &readySemaphoreInfos[semIndex].semaphore,
      .swapchainCount = 1u,
      .pSwapchains = &swapchain.as<vk::SwapchainKHR>(),
      .pImageIndices = &uniformData.currentImage
    });

    // stop the capture
    if (rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
  };

  // 
  return 0;
};

