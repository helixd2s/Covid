#pragma once

//
#ifdef _WIN32
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#endif
#else
#ifdef __linux__ 
//FD defaultly
#endif
#endif

// 
#include <LXVC/lxvc.hpp>
#include <GLFW/glfw3.h>
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
  uint64_t accStruct = 0ull;
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
  decltype(auto) descriptors = lxvc::DescriptorsObj::make(device.with(0u), lxvc::DescriptorsCreateInfo{

  });

  //
  decltype(auto) uniformData = UniformData{};

  //
  decltype(auto) buffer = lxvc::ResourceObj::make(device, lxvc::ResourceCreateInfo{
    .descriptors = descriptors.as<vk::PipelineLayout>(),
    .bufferInfo = lxvc::BufferCreateInfo{
      .size = 1024ull,
      .type = lxvc::BufferType::eUniversal,
    }
  });

  //
  decltype(auto) uploader = lxvc::UploaderObj::make(device, lxvc::UploaderCreateInfo{

  });

  //
  decltype(auto) samplers = descriptors->getSamplerDescriptors();

  //
  std::vector<glm::vec4> vertices{ 
    glm::vec4{0.f, 0.f, 0.1f, 1.0}, glm::vec4{1.f, 0.f, 0.1f, 1.0}, glm::vec4{0.f, 1.f, 0.1f, 1.0},
    glm::vec4{1.f, 1.f, 0.1f, 1.0}, glm::vec4{0.f, 1.f, 0.1f, 1.0}, glm::vec4{1.f, 0.f, 0.1f, 1.0},
  };

  //
  std::vector<uint16_t> indices{0u,1u,2u,3u,4u,5u};

  //
  uploader->executeUploadToResourceOnce(lxvc::UploadExecutionOnce{
    .host = std::span<char8_t>{(char8_t*)vertices.data(), vertices.size() * sizeof(glm::vec4)},
    .writeInfo = lxvc::UploadCommandWriteInfo{
      .dstBuffer = lxvc::BufferRegion{buffer.as<vk::Buffer>(), lxvc::DataRegion{0ull, sizeof(glm::vec4) * vertices.size()}},
      
    }
  });

  //
  uploader->executeUploadToResourceOnce(lxvc::UploadExecutionOnce{
    .host = std::span<char8_t>{(char8_t*)indices.data(), indices.size() * sizeof(uint16_t)},
    .writeInfo = lxvc::UploadCommandWriteInfo{
      .dstBuffer = lxvc::BufferRegion{buffer.as<vk::Buffer>(), lxvc::DataRegion{sizeof(glm::vec4) * vertices.size(), sizeof(uint16_t) * indices.size()}},
    }
  });

  //
  uint64_t verticesAddress = buffer->getDeviceAddress();
  uint64_t indicesAddress = verticesAddress + (sizeof(glm::vec4) * vertices.size());




  


  //
  decltype(auto) compute = lxvc::PipelineObj::make(device.with(0u), lxvc::PipelineCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
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
    .layout = descriptors.as<vk::PipelineLayout>(),
    .graphics = lxvc::GraphicsPipelineCreateInfo{
      .stageCodes = stageMaps
    }
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
    .layout = descriptors.as<vk::PipelineLayout>(),
    .surface = surface,
    .info = qfAndQueue
  });

  //
  decltype(auto) framebuffer = lxvc::FramebufferObj::make(device.with(0u), lxvc::FramebufferCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
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
  decltype(auto) geometryLevel = lxvc::GeometryLevelObj::make(device, lxvc::GeometryLevelCreateInfo{
    .geometryData = std::vector<lxvc::GeometryInfo>{lxvc::GeometryInfo{
      .vertices = lxvc::BufferViewInfo{.deviceAddress = verticesAddress, .stride = sizeof(glm::vec4), .format = lxvc::BufferViewFormat::eFloat3},
      .indices = lxvc::BufferViewInfo{.deviceAddress = indicesAddress, .stride = sizeof(uint16_t), .format = lxvc::BufferViewFormat::eShort},
      .primitiveCount = 2u,
    }},
    .uploader = uploader.as<uintptr_t>(),
  });

  //
  decltype(auto) instanceLevel = lxvc::InstanceLevelObj::make(device, lxvc::InstanceLevelCreateInfo{
    .instanceData = std::vector<lxvc::InstanceInfo>{lxvc::InstanceInfo{
      .transform = reinterpret_cast<vk::TransformMatrixKHR&&>(glm::mat3x4(1.f)),
      .instanceCustomIndex = 0u,
      .mask = 0xFFu,
      .instanceShaderBindingTableRecordOffset = 0u,
      .flags = 0u,
      .accelerationStructureReference = geometryLevel.as<uintptr_t>()
    }},
    .uploader = uploader.as<uintptr_t>(),
  });

  // 
  uniformData.accStruct = instanceLevel.as<uintptr_t>();


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
    decltype(auto) uniformFence = descriptors->executeUniformUpdateOnce(lxvc::UniformDataSet{
      .writeInfo = lxvc::UniformDataWriteSet{
        .region = lxvc::DataRegion{0ull, sizeof(UniformData)},
        .data = cpp21::data_view<char8_t>((char8_t*)&uniformData, sizeof(UniformData)),
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
        .layout = descriptors.as<vk::PipelineLayout>(),
        .framebuffer = framebuffer.as<uintptr_t>(),
        .instanceInfos = instanceLevel->getDrawInfo(),
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
        .layout = descriptors.as<vk::PipelineLayout>(),
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

