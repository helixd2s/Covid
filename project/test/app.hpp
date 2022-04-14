#pragma once
#define GLM_FORCE_SWIZZLE

//
#ifdef __cplusplus
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
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <Alter/Alter.hpp>
#include "./controller.hpp"

//
struct Constants
{
  glm::mat4x4 perspective = glm::mat4x4(1.f);
  glm::mat4x4 perspectiveInverse = glm::mat4x4(1.f);
  glm::mat3x4 lookAt = glm::mat3x4(1.f);
  glm::mat3x4 lookAtInverse = glm::mat3x4(1.f);
  glm::mat3x4 previousLookAt = glm::mat3x4(1.f);
  glm::mat3x4 previousLookAtInverse = glm::mat3x4(1.f);
};

//
struct UniformData {
  uint32_t framebufferAttachments[4] = { 0u,0u,0u,0u };
  glm::uvec2 extent = {}; uint32_t frameCounter, reserved;
  Constants constants = {};
  //uint64_t verticesAddress = 0ull;
};

// 
class App {
protected: 
  ANAMED::WrapShared<ANAMED::InstanceObj> instanceObj = {};
  ANAMED::WrapShared<ANAMED::DeviceObj> deviceObj = {};
  ANAMED::WrapShared<ANAMED::MemoryAllocatorObj> memoryAllocatorVma = {};
  ANAMED::WrapShared<ANAMED::DescriptorsObj> descriptorsObj = {};
  ANAMED::WrapShared<ANAMED::UploaderObj> uploaderObj = {};
  ANAMED::WrapShared<ANAMED::GltfLoaderObj> gltfLoader = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> resampleObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> computeObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> graphicsObj = {};
  ANAMED::WrapShared<ANAMED::SwapchainObj> swapchainObj = {};
  ANAMED::WrapShared<ANAMED::FramebufferObj> framebufferObj = {};
  ANAMED::WrapShared<ANAMED::PingPongObj> pingPongObj = {};

  //
  UniformData uniformData = {};
  ANAMED::InstanceAddressBlock instanceAddressBlock = {};
  ANAMED::QueueGetInfo qfAndQueue = ANAMED::QueueGetInfo{ 0u, 0u };
  vk::Rect2D renderArea = {};

  //
  std::shared_ptr<std::array<ANAMED::FenceType, 8>> fences = {};
  std::shared_ptr<Controller> controller = {};
  std::shared_ptr<ANAMED::GltfModel> modelObj = {};
  std::shared_ptr<GLFWListener> listener = {};

  //
  GLFWwindow* window = nullptr;

  //
  double previousTime = glfwGetTime();
  uint32_t frameCount = 0;

  //
public: 
  App() {
    this->construct();
  };

  //
  virtual void tickProcessing() {
    deviceObj->tickProcessing();
  };

  //
  virtual void displayFPS(uint32_t& frameCount) {
    glfwSetWindowTitle(window, (std::string("Alter.TEON.A; FPS: ") + std::to_string(frameCount)).c_str()); frameCount = 0;
  };

  // 
  virtual std::experimental::generator<bool> renderGen() {
    co_yield false;

    // set perspective
    controller->handleFrame();
    auto persp = glm::perspective(60.f / 180 * glm::pi<float>(), float(renderArea.extent.width) / float(renderArea.extent.height), 0.001f, 10000.f);
    auto lkat = glm::lookAt(controller->viewPos, controller->viewCnt, controller->viewUp);
    uniformData.constants.perspective = glm::transpose(persp);
    uniformData.constants.perspectiveInverse = glm::transpose(glm::inverse(persp));
    uniformData.constants.previousLookAt = uniformData.constants.lookAt;
    uniformData.constants.previousLookAtInverse = uniformData.constants.lookAtInverse;
    uniformData.constants.lookAt = glm::mat3x4(glm::transpose(lkat));
    uniformData.constants.lookAtInverse = glm::mat3x4(glm::transpose(glm::inverse(lkat)));
    uniformData.extent = glm::uvec2(renderArea.extent.width, renderArea.extent.height);
    uniformData.frameCounter = 0u;

    //
//#ifdef ENABLE_RENDERDOC
    //if (rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
//#endif

    //
    double currentTime = glfwGetTime();
    frameCount++;

    if (currentTime - previousTime >= 1.0)
    {
      // Display the frame count here any way you want.
      displayFPS(frameCount);
      previousTime = currentTime;
    };

    // 
    decltype(auto) acquired = swapchainObj->acquireImage(qfAndQueue);
    decltype(auto) pingPong = pingPongObj->acquireImage(qfAndQueue);

    //
    pingPongObj->clearImage(qfAndQueue, 1u, glm::uintBitsToFloat(glm::uvec4(0u)));
    pingPongObj->clearImage(qfAndQueue, 5u, glm::uintBitsToFloat(glm::uvec4(0u)));

    //
    //if (controller->needsClear) {
      //pingPongObj->clearImage(qfAndQueue, 4u, glm::uintBitsToFloat(glm::uvec4(0u)));
      //controller->needsClear = false;
    //};

    // wait ready for filling
    auto& fence = (*fences)[acquired];
    decltype(auto) status = false;
    if (fence) { while (!(status = fence->checkStatus())) { co_yield status; }; };

    //
    uniformData.frameCounter++;

    // 
    decltype(auto) uniformFence = descriptorsObj->executeUniformUpdateOnce(ANAMED::UniformDataSet{
      // # yet another std::optional problem (implicit)
      .writeInfo = std::optional<ANAMED::UniformDataWriteSet>(ANAMED::UniformDataWriteSet{
        .region = ANAMED::DataRegion{0ull, 4ull, sizeof(UniformData)},
        .data = cpp21::data_view<char8_t>((char8_t*)&uniformData, 0ull, sizeof(UniformData)),
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
    });

    //
    framebufferObj->clearAttachments(qfAndQueue);

    //
    decltype(auto) graphicsFence = graphicsObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .framebuffer = framebufferObj.as<uintptr_t>(),
        .swapchain = swapchainObj.as<uintptr_t>(),
        .pingpong = pingPongObj.as<uintptr_t>(),
        .instanceDraws = modelObj->getDefaultScene()->instanced->getDrawInfo(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
    });

    //
    decltype(auto) resampleFence = resampleObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 256u), renderArea.extent.height, 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .swapchain = swapchainObj.as<uintptr_t>(),
        .pingpong = pingPongObj.as<uintptr_t>(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue
      }
      });

    //
    decltype(auto) computeFence = computeObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 256u), renderArea.extent.height, 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .swapchain = swapchainObj.as<uintptr_t>(),
        .pingpong = pingPongObj.as<uintptr_t>(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue
      }
    });

    //
    pingPongObj->presentImage(qfAndQueue);
    fence = std::get<0u>(swapchainObj->presentImage(qfAndQueue));

    // stop the capture
//#ifdef ENABLE_RENDERDOC
    //if (rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
//#endif

    // 
    co_yield true;
  };

  virtual void initSurface(GLFWwindow* window) {
    // 
    this->controller = std::make_shared<Controller>(this->listener = std::make_shared<GLFWListener>(this->window = window));

    //
    vk::SurfaceKHR surface = {};
    glfwCreateWindowSurface(instanceObj.as<VkInstance>(), window, nullptr, (VkSurfaceKHR*)&surface);

    //
    swapchainObj = ANAMED::SwapchainObj::make(deviceObj, ANAMED::SwapchainCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .surface = surface,
      .info = qfAndQueue
      });

    //
    renderArea = swapchainObj->getRenderArea();

    //
    framebufferObj = ANAMED::FramebufferObj::make(deviceObj.with(0u), ANAMED::FramebufferCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .extent = renderArea.extent,
      .info = qfAndQueue
      });

    //
    pingPongObj = ANAMED::PingPongObj::make(deviceObj.with(0u), ANAMED::PingPongCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .extent = renderArea.extent,
      .minImageCount = 1u,

      // first image is accumulation, second image is back buffer, third image is index buffer, fourth image is position buffer
      // 5th for reflection buffer, 6th for reflection back buffer, 7th for transparency, 8th for transparency back
      .split = std::vector<bool>{false, true, false, false, false, true, false, true, false, false, false, false},
      .formats = std::vector<vk::Format>{ 
        vk::Format::eR32G32B32A32Uint, 
        vk::Format::eR32G32B32A32Uint, 
        vk::Format::eR32G32B32A32Uint, 
        vk::Format::eR32G32B32A32Sfloat, 
        vk::Format::eR32G32B32A32Uint, 
        vk::Format::eR32G32B32A32Uint,
        vk::Format::eR32G32B32A32Uint,
        vk::Format::eR32G32B32A32Uint,
        vk::Format::eR32G32B32A32Sfloat,
        vk::Format::eR32G32B32A32Sfloat,
        vk::Format::eR32G32B32A32Uint,
        vk::Format::eR32G32B32A32Uint,
      },
      .info = qfAndQueue
    });

    // 
    decltype(auto) framebufferAttachments = framebufferObj->getImageViewIndices();
    memcpy(uniformData.framebufferAttachments, framebufferAttachments.data(), std::min(framebufferAttachments.size(), 4ull) * sizeof(uint32_t));
  };

  //
  void loadModel(std::string model, float scale = 1.f) {
    instanceAddressBlock = ANAMED::InstanceAddressBlock{
      .opaqueAddressInfo = (modelObj = gltfLoader->load(model, scale))->getDefaultScene()->instanced->getAddressInfo()
    };
  };

protected: 

  // 
  virtual void construct() {
    uniformData = UniformData{};

    // 
    instanceObj = ANAMED::InstanceObj::make(ANAMED::context, ANAMED::InstanceCreateInfo{

    });

    // 
    deviceObj = ANAMED::DeviceObj::make(instanceObj, ANAMED::DeviceCreateInfo{
      .physicalDeviceGroupIndex = 0u,
      .physicalDeviceIndex = 0u
    });

    //
    memoryAllocatorVma = ANAMED::MemoryAllocatorVma::make(deviceObj, ANAMED::MemoryAllocatorCreateInfo{

    });

    // 
    descriptorsObj = ANAMED::DescriptorsObj::make(deviceObj.with(0u), ANAMED::DescriptorsCreateInfo{

    });

    //
    uploaderObj = ANAMED::UploaderObj::make(deviceObj, ANAMED::UploaderCreateInfo{

    });

    //
    gltfLoader = ANAMED::GltfLoaderObj::make(deviceObj, ANAMED::GltfLoaderCreateInfo{
      .uploader = uploaderObj.as<uintptr_t>(),
      .descriptors = descriptorsObj.as<vk::PipelineLayout>()
    });

    //
    computeObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./test.comp.spv")
      }
    });

    //
    resampleObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./resample.comp.spv")
      }
    });

    //
    std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> stageMaps = {};
    stageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./opaque.vert.spv");
    stageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./opaque.frag.spv");

    //
    graphicsObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .graphics = ANAMED::GraphicsPipelineCreateInfo{
        .stageCodes = stageMaps
      }
    });

    //
    qfAndQueue = ANAMED::QueueGetInfo{ 0u, 0u };
    fences = std::make_shared<std::array<ANAMED::FenceType, 8>>();

    //
    previousTime = glfwGetTime();
    frameCount = 0;
  };

};

#endif