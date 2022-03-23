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
#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

// 
#include <ZEON/ZEON.hpp>
#include <GLFW/glfw3.h>
#ifdef ENABLE_RENDERDOC
#include "renderdoc_app.h"
#include <eh.h>
#endif

// 
void error(int errnum, const char* errmsg)
{
  std::cerr << errnum << ": " << errmsg << std::endl;
};

//
struct Constants
{
  glm::mat4x4 perspective = glm::mat4x4(1.f);
  glm::mat4x4 perspectiveInverse = glm::mat4x4(1.f);
  glm::mat3x4 lookAt = glm::mat3x4(1.f);
  glm::mat3x4 lookAtInverse = glm::mat3x4(1.f);
};

//
struct UniformData {
  uint32_t framebufferAttachments[4] = { 0u,0u,0u,0u };
  glm::uvec2 extent = {}; uint32_t testTex, testSampler;
  Constants constants = {};
  //uint64_t verticesAddress = 0ull;
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

#ifdef ENABLE_RENDERDOC
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
#endif

  //
  ZNAMED::initialize();




  //
  //std::cout << "We running experimental renderer... continue?" << std::endl;
  //system("PAUSE");


  // first cherep
  decltype(auto) instance = ZNAMED::InstanceObj::make(ZNAMED::context, ZNAMED::InstanceCreateInfo{

  });

  // second cherep
  decltype(auto) device = ZNAMED::DeviceObj::make(instance, ZNAMED::DeviceCreateInfo{
    .physicalDeviceGroupIndex = 0u,
    .physicalDeviceIndex = 0u
  });

  //
  decltype(auto) memoryAllocatorVma = ZNAMED::MemoryAllocatorVma::make(device, ZNAMED::MemoryAllocatorCreateInfo{

  });

  // final cherep for today
  decltype(auto) descriptors = ZNAMED::DescriptorsObj::make(device.with(0u), ZNAMED::DescriptorsCreateInfo{

  });

  //
  decltype(auto) uniformData = UniformData{};

  //
  decltype(auto) uploader = ZNAMED::UploaderObj::make(device, ZNAMED::UploaderCreateInfo{

  });

  //
  decltype(auto) gltfLoader = ZNAMED::GltfLoaderObj::make(device, ZNAMED::GltfLoaderCreateInfo{
    .uploader = uploader.as<uintptr_t>(),
    .descriptors = descriptors.as<vk::PipelineLayout>()
  });

  // 
  decltype(auto) modelObj = gltfLoader->load("./BoomBox.gltf");

  //
  decltype(auto) instanceAddressBlock = ZNAMED::InstanceAddressBlock{
    .opaqueAddressInfo = modelObj->getDefaultScene()->instanced->getAddressInfo()
  };

  //
  decltype(auto) compute = ZNAMED::PipelineObj::make(device.with(0u), ZNAMED::PipelineCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .compute = ZNAMED::ComputePipelineCreateInfo{
      .code = cpp21::readBinaryU32("./test.comp.spv")
    }
  });

  //
  Z_UNORDERED_MAP<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> stageMaps = {};
  stageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./opaque.vert.spv");
  stageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./opaque.frag.spv");

  //
  decltype(auto) graphics = ZNAMED::PipelineObj::make(device.with(0u), ZNAMED::PipelineCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .graphics = ZNAMED::GraphicsPipelineCreateInfo{
      .stageCodes = stageMaps
    }
  });

  //
  decltype(auto) qfAndQueue = ZNAMED::QueueGetInfo{ 0u, 0u };
  //std::shared_ptr<std::array<ZNAMED::FenceType, 4>> fences = std::make_shared<std::array<ZNAMED::FenceType, 4>>();
  decltype(auto) fences = std::make_shared<std::array<ZNAMED::FenceType, 4>>();

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
  std::string title = "ZEON.TEON.A";
  decltype(auto) window = glfwCreateWindow(SC_WIDTH, SC_HEIGHT, title.c_str(), nullptr, nullptr);
  glfwCreateWindowSurface(instance.as<VkInstance>(), window, nullptr, (VkSurfaceKHR*)&surface);

  //
  decltype(auto) swapchain = ZNAMED::SwapchainObj::make(device, ZNAMED::SwapchainCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .surface = surface,
    .info = qfAndQueue
  });

  //
  decltype(auto) renderArea = swapchain->getRenderArea();
  

  // set perspective
  auto persp = glm::perspective(60.f / 180 * glm::pi<float>(), float(renderArea.extent.width) / float(renderArea.extent.height), 0.001f, 10000.f);
  auto lkat = glm::lookAt(glm::vec3(0.f, 0.f, 0.1f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
  uniformData.constants.perspective = glm::transpose(persp);
  uniformData.constants.perspectiveInverse = glm::transpose(glm::inverse(persp));
  uniformData.constants.lookAt = glm::mat3x4(glm::transpose(lkat));
  uniformData.constants.lookAtInverse = glm::mat3x4(glm::transpose(glm::inverse(lkat)));
  uniformData.extent = glm::uvec2(renderArea.extent.width, renderArea.extent.height);

  //
  decltype(auto) framebuffer = ZNAMED::FramebufferObj::make(device.with(0u), ZNAMED::FramebufferCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .extent = renderArea.extent,
    .info = qfAndQueue
  });

  //
  decltype(auto) readySemaphoreInfos = swapchain->getReadySemaphoreInfos();
  decltype(auto) presentSemaphoreInfos = swapchain->getPresentSemaphoreInfos();

  // 
  decltype(auto) framebufferAttachments = framebuffer->getImageViewIndices();
  memcpy(uniformData.framebufferAttachments, framebufferAttachments.data(), std::min(framebufferAttachments.size(), 4ull) * sizeof(uint32_t));

  //
  std::shared_ptr<std::future<bool>> processing = {};


  //
  decltype(auto) renderGen = [=]() -> std::experimental::generator<bool> {
    co_yield false;

    //
#ifdef ENABLE_RENDERDOC
    if (rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
#endif

    // 
    decltype(auto) acquired = swapchain->acquireImage(qfAndQueue);

    // wait ready for filling
    auto& fence = (*fences)[acquired];
    decltype(auto) status = false;
    //if (fence) { decltype(auto) unleak = fence->future->get(); }; device->tickProcessing();
    if (fence) { while (!(status = fence->checkStatus())) { co_yield status; }; };

    // 
    decltype(auto) uniformFence = descriptors->executeUniformUpdateOnce(ZNAMED::UniformDataSet{
      .writeInfo = ZNAMED::UniformDataWriteSet{
        .region = ZNAMED::DataRegion{0ull, 4ull, sizeof(UniformData)},
        .data = cpp21::data_view<char8_t>((char8_t*)&uniformData, 0ull, sizeof(UniformData)),
      },
      .submission = ZNAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
      });

    //
    framebuffer->clearAttachments(qfAndQueue);

    //
    decltype(auto) graphicsFence = graphics->executePipelineOnce(ZNAMED::ExecutePipelineInfo{
      .graphics = ZNAMED::WriteGraphicsInfo{
        .layout = descriptors.as<vk::PipelineLayout>(),
        .framebuffer = framebuffer.as<uintptr_t>(),
        .swapchain = swapchain.as<vk::SwapchainKHR>(),
        .instanceDraws = modelObj->getDefaultScene()->instanced->getDrawInfo(),
        .instanceAddressBlock = instanceAddressBlock
      },
      .submission = ZNAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
      });

    //
    decltype(auto) computeFence = compute->executePipelineOnce(ZNAMED::ExecutePipelineInfo{
      .compute = ZNAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 256u), renderArea.extent.height, 1u},
        .layout = descriptors.as<vk::PipelineLayout>(),
        .swapchain = swapchain.as<vk::SwapchainKHR>(),
        .instanceAddressBlock = instanceAddressBlock
      },
      .submission = ZNAMED::SubmissionInfo{
        .info = qfAndQueue
      }
    });

    //
    fence = std::get<0u>(swapchain->presentImage(qfAndQueue));

    // stop the capture
#ifdef ENABLE_RENDERDOC
    if (rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
#endif

    // 
    co_yield true;
  };

  //
  decltype(auto) rendering = renderGen();

  //
  decltype(auto) iterator = rendering.begin();

  // 
  while (!glfwWindowShouldClose(window)) { // 
    glfwPollEvents();
    _CrtDumpMemoryLeaks();

    // 
    if (iterator == rendering.end()) { rendering = renderGen(); iterator = rendering.begin(); };
    iterator++;

    //
    //if (!processing || cpp21::is_ready(*processing)) {
      //processing = std::make_shared<std::future<bool>>(std::async(std::launch::async | std::launch::deferred, renderGen));
    //};
  };

  // waiting at least...
  //if (processing) { processing.get(); };

  // 
  return 0;
};

