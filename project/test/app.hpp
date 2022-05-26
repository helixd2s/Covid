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

  // TODO: make array of
  glm::mat3x4 lookAt[2] = { glm::mat3x4(1.f) };
  glm::mat3x4 lookAtInverse[2] = { glm::mat3x4(1.f) };
};

//
#pragma pack(push, 1)
struct UniformData {
  ANAMED::SwapchainStateInfo swapchain;
  ANAMED::PingPongStateInfo pingpong;
  ANAMED::FramebufferStateInfo framebuffers[2];
  Constants constants = {};

  //glm::uvec2 extent = {}; 
  uint32_t r0 = 0u;
  uint32_t frameCounter = 0u;
  uint32_t backgroundObj = 0u;
  uint32_t blueNoiseObj = 0u;
  uint64_t pixelData = 0ull;
  uint64_t writeData = 0ull;
  uint64_t rasterData[2] = { 0ull };
  uint64_t surfaceData = 0ull;
};
#pragma pack(pop)

//
struct CounterData {
  uint32_t counters[4];
  uint32_t previousCounters[4];
};

//
struct PixelHitInfo {
  glm::uvec4 indices;
  glm::vec4 origin;
  glm::vec4 direct;
};

//
struct PixelSurfaceInfo {
  glm::uvec4 indices;
  glm::vec3 origin;
  glm::vec3 normal;
  glm::vec4 tex[2];
  glm::uvec4 accum[3];
  glm::uvec4 color[3];
};

//
struct RasterInfo {
  glm::uvec4 indices = glm::uvec4(0u); // indlude .W are pNext
  glm::vec4 barycentric = glm::vec4(0.f);
  glm::uvec4 derivatives = glm::uvec4(0.f); // f16[dUx, dUy], f16[dVx, dVy], f16[dWx, dWy], f16[dDx, dDy]
};

// 
class App {
protected: 
  ANAMED::WrapShared<ANAMED::InstanceObj> instanceObj = {};
  ANAMED::WrapShared<ANAMED::DeviceObj> deviceObj = {};
  ANAMED::WrapShared<ANAMED::MemoryAllocatorObj> memoryAllocatorVma = {};
  ANAMED::WrapShared<ANAMED::PipelineLayoutObj> descriptorsObj = {};
  ANAMED::WrapShared<ANAMED::UploaderObj> uploaderObj = {};
  ANAMED::WrapShared<ANAMED::GltfLoaderObj> gltfLoaderObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> reserveObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> resampleObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> pathTracerObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> resortObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> recopyObj = {};

  ANAMED::WrapShared<ANAMED::PipelineObj> preOpaqueObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> preTranslucentObj = {};

  ANAMED::WrapShared<ANAMED::PipelineObj> nativeOpaqueObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> nativeTranslucentObj = {};

  ANAMED::WrapShared<ANAMED::PipelineObj> postObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> controlObj = {};
  
  ANAMED::WrapShared<ANAMED::SwapchainObj> swapchainObj = {};
  ANAMED::WrapShared<ANAMED::FramebufferObj> framebufferObj[2] = {};
  ANAMED::WrapShared<ANAMED::PingPongObj> pingPongObj = {};
  ANAMED::WrapShared<ANAMED::ResourceObj> backgroundObj = {};
  ANAMED::WrapShared<ANAMED::ResourceObj> blueNoiseObj = {};
  ANAMED::WrapShared<ANAMED::ResourceObj> pixelDataObj = {};
  ANAMED::WrapShared<ANAMED::ResourceObj> writeDataObj = {};
  ANAMED::WrapShared<ANAMED::ResourceObj> rasterDataObj = {};
  ANAMED::WrapShared<ANAMED::ResourceObj> surfaceDataObj = {};

  //
  UniformData uniformData = {};
  CounterData counterData = {};

  //
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
  float scale = 1.f;

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
    auto persp = glm::perspective(60.f / 180 * glm::pi<float>(), float(renderArea.extent.width) / float(renderArea.extent.height), 0.002f, 20000.f);
    auto lkat = glm::lookAt(controller->viewPos, controller->viewCnt, controller->viewUp);
    uniformData.constants.perspective = glm::transpose(persp);
    uniformData.constants.perspectiveInverse = glm::transpose(glm::inverse(persp));
    uniformData.constants.lookAt[1] = cpp21::exchange(uniformData.constants.lookAt[0], glm::mat3x4(glm::transpose(lkat)));
    uniformData.constants.lookAtInverse[1] = cpp21::exchange(uniformData.constants.lookAtInverse[0], glm::mat3x4(glm::transpose(glm::inverse(lkat))));

    // 
    for (uint32_t i = 0; i < 2; i++) {
      framebufferObj[i]->acquireImage(qfAndQueue);
      framebufferObj[i]->clearAttachments(qfAndQueue);
    };

    // 
    decltype(auto) acquired = swapchainObj->acquireImage(qfAndQueue);
    decltype(auto) pingPong = pingPongObj->acquireImage(qfAndQueue);

    //
    for (uint32_t i = 0; i < 2; i++) {
      uniformData.framebuffers[i] = framebufferObj[i]->getStateInfo();
    };
    uniformData.swapchain = swapchainObj->getStateInfo();
    uniformData.pingpong = pingPongObj->getStateInfo();

    //
    double currentTime = glfwGetTime();
    frameCount++;

    // 
    if (currentTime - previousTime >= 1.0)
    {
      // Display the frame count here any way you want.
      displayFPS(frameCount);
      previousTime = currentTime;
    };

    //
    pingPongObj->clearImage(qfAndQueue, 0u, glm::uintBitsToFloat(glm::uvec4(0u)));

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
    decltype(auto) controlFence = controlObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{1u, 1u, 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue
      }
    });

    //
    decltype(auto) nativeOpaqueFence = nativeOpaqueObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .framebuffer = framebufferObj[0].as<uintptr_t>(),
        .instanceDraws = modelObj->getDefaultScene()->opaque->instanced->getDrawInfo(),
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
    });

    //
    decltype(auto) nativeTranslucentFence = nativeTranslucentObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .framebuffer = framebufferObj[1].as<uintptr_t>(),
        .instanceDraws = modelObj->getDefaultScene()->translucent->instanced->getDrawInfo(),
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
    });

    //
    decltype(auto) preOpaqueFence = preOpaqueObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .framebuffer = framebufferObj[0].as<uintptr_t>(),
        .instanceDraws = modelObj->getDefaultScene()->opaque->instanced->getDrawInfo(),
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
      });

    //
    decltype(auto) preTranslucentFence = preTranslucentObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .framebuffer = framebufferObj[1].as<uintptr_t>(),
        .instanceDraws = modelObj->getDefaultScene()->translucent->instanced->getDrawInfo(),
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
      });

    //
    decltype(auto) resortFence = resortObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(uniformData.framebuffers[0].extent.x, 32u), cpp21::tiled(uniformData.framebuffers[0].extent.y, 8u), 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue
      }
      });

    //
    decltype(auto) resampleFence = resampleObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 32u), cpp21::tiled(renderArea.extent.height, 4u), 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue
      }
      });

    //
    decltype(auto) computeFence = pathTracerObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 32u), cpp21::tiled(renderArea.extent.height, 4u), 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue
      }
    });

    //
    decltype(auto) recopyFence = recopyObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 32u), cpp21::tiled(renderArea.extent.height, 4u), 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue
      }
      });

    //
    decltype(auto) reserveFence = reserveObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 32u), cpp21::tiled(renderArea.extent.height, 4u), 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue
      }
    });

    //
    decltype(auto) postFence = postObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(uniformData.swapchain.extent.x, 32u), cpp21::tiled(uniformData.swapchain.extent.y, 4u), 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
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

    //
    std::swap(uniformData.rasterData[0], uniformData.rasterData[1]);

    // 
    co_yield true;
  };

  //
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
    uniformData.swapchain = swapchainObj->getStateInfo();

    //
    float xscale = 1.f, yscale = 1.f;
    glfwGetWindowContentScale(window, &xscale, &yscale);

    //
    renderArea = swapchainObj->getRenderArea();
    renderArea.extent.width *= 2.f / xscale;
    renderArea.extent.height *= 2.f / yscale;

    // 
    surfaceDataObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
      .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
      .bufferInfo = ANAMED::BufferCreateInfo{
        .size = sizeof(PixelSurfaceInfo) * renderArea.extent.width * renderArea.extent.height,
        .type = ANAMED::BufferType::eStorage,
      }
    });

    // 
    pixelDataObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
      .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
      .bufferInfo = ANAMED::BufferCreateInfo{
        .size = sizeof(PixelHitInfo) * renderArea.extent.width * renderArea.extent.height * 3u,
        .type = ANAMED::BufferType::eStorage,
      }
    });

    // 
    writeDataObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
      .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
      .bufferInfo = ANAMED::BufferCreateInfo{
        .size = sizeof(PixelHitInfo) * renderArea.extent.width * renderArea.extent.height * 3u,
        .type = ANAMED::BufferType::eStorage,
      }
    });

    // 
    rasterDataObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
      .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
      .bufferInfo = ANAMED::BufferCreateInfo{
        .size = sizeof(RasterInfo) * renderArea.extent.width * renderArea.extent.height * 32u,
        .type = ANAMED::BufferType::eStorage,
      }
    });

    //
    uniformData.surfaceData = surfaceDataObj->getDeviceAddress();
    uniformData.pixelData = pixelDataObj->getDeviceAddress();
    uniformData.writeData = writeDataObj->getDeviceAddress();
    uniformData.rasterData[0] = rasterDataObj->getDeviceAddress();
    uniformData.rasterData[1] = uniformData.rasterData[0] + sizeof(RasterInfo) * renderArea.extent.width * renderArea.extent.height * 16u;

    //
    uint32_t testDivision = 1u;

    //
    for (uint32_t i = 0; i < 2; i++) {
      framebufferObj[i] = ANAMED::FramebufferObj::make(deviceObj.with(0u), ANAMED::FramebufferCreateInfo{
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .extent = vk::Extent2D{uniformData.swapchain.extent.x / testDivision, uniformData.swapchain.extent.y / testDivision},
        .info = qfAndQueue
      });
      uniformData.framebuffers[i] = framebufferObj[i]->getStateInfo();
    };

    //
    uniformData.frameCounter = 0u;

    //
    pingPongObj = ANAMED::PingPongObj::make(deviceObj.with(0u), ANAMED::PingPongCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .extent = renderArea.extent,
      .minImageCount = 2u,

      // first image is accumulation, second image is back buffer, third image is index buffer, fourth image is position buffer
      // 5th for reflection buffer, 6th for reflection back buffer, 7th for transparency, 8th for transparency back
      .split = std::vector<uint32_t>{ 4, 4, 4, 4, 1, 1 },
      .formats = std::vector<vk::Format>{ vk::Format::eR32Uint, vk::Format::eR32Uint, vk::Format::eR32Uint, vk::Format::eR32Uint, vk::Format::eR32G32B32A32Sfloat, vk::Format::eR32G32B32A32Sfloat },
      .info = qfAndQueue
    });

    //
    uniformData.pingpong = pingPongObj->getStateInfo();
    
  };

  //
  void loadModel(std::string model, float scale = 1.f) {
    instanceAddressBlock = (modelObj = gltfLoaderObj->load(model, this->scale = scale))->getDefaultScene()->addressBlock;
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
    descriptorsObj = ANAMED::PipelineLayoutObj::make(deviceObj.with(0u), ANAMED::DescriptorsCreateInfo{

    });

    //
    uploaderObj = ANAMED::UploaderObj::make(deviceObj, ANAMED::UploaderCreateInfo{

    }.use(ANAMED::ExtensionName::eMemoryAllocatorVma));

    //
    gltfLoaderObj = ANAMED::GltfLoaderObj::make(deviceObj, ANAMED::GltfLoaderCreateInfo{
      .uploader = uploaderObj.as<uintptr_t>(),
      .descriptors = descriptorsObj.as<vk::PipelineLayout>()
    });

    //
    pathTracerObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./shaders/path-tracer.comp.spv")
      }
    });

    //
    reserveObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./shaders/reserve.comp.spv")
      }
      });

    //
    resampleObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./shaders/resample.comp.spv")
      }
    });

    //
    resortObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./shaders/resort.comp.spv")
      }
      });

    //
    recopyObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./shaders/recopy.comp.spv")
      }
      });

    //
    postObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./shaders/post.comp.spv")
      }
    });

    //
    controlObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./shaders/control.comp.spv")
      }
    });

    //
    std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> preOpaqueStageMaps = {};
    preOpaqueStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./shaders/pre-opaque.vert.spv");
    preOpaqueStageMaps[vk::ShaderStageFlagBits::eGeometry] = cpp21::readBinaryU32("./shaders/pre-opaque.geom.spv");
    preOpaqueStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./shaders/pre-opaque.frag.spv");
    preOpaqueObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .graphics = ANAMED::GraphicsPipelineCreateInfo{
        .stageCodes = preOpaqueStageMaps,
        .hasDepthTest = false,
        .hasDepthWrite = false,
        .hasConservativeRaster = true,
      }
    });

    //
    std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> preTranslucentStageMaps = {};
    preTranslucentStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./shaders/pre-translucent.vert.spv");
    preTranslucentStageMaps[vk::ShaderStageFlagBits::eGeometry] = cpp21::readBinaryU32("./shaders/pre-translucent.geom.spv");
    preTranslucentStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./shaders/pre-translucent.frag.spv");
    preTranslucentObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .graphics = ANAMED::GraphicsPipelineCreateInfo{
        .stageCodes = preTranslucentStageMaps,
        .hasDepthTest = false,
        .hasDepthWrite = false,
        .hasConservativeRaster = true,
      }
    });


    //
    std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> nativeOpaqueStageMaps = {};
    nativeOpaqueStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./shaders/native-opaque.vert.spv");
    nativeOpaqueStageMaps[vk::ShaderStageFlagBits::eGeometry] = cpp21::readBinaryU32("./shaders/native-opaque.geom.spv");
    nativeOpaqueStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./shaders/native-opaque.frag.spv");
    nativeOpaqueObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .graphics = ANAMED::GraphicsPipelineCreateInfo{
        .stageCodes = nativeOpaqueStageMaps,
        .hasDepthTest = true,
        .hasDepthWrite = true,
        .hasConservativeRaster = false,
      }
      });

    //
    std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> nativeTranslucentStageMaps = {};
    nativeTranslucentStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./shaders/native-translucent.vert.spv");
    nativeTranslucentStageMaps[vk::ShaderStageFlagBits::eGeometry] = cpp21::readBinaryU32("./shaders/native-translucent.geom.spv");
    nativeTranslucentStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./shaders/native-translucent.frag.spv");
    nativeTranslucentObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .graphics = ANAMED::GraphicsPipelineCreateInfo{
        .stageCodes = nativeTranslucentStageMaps,
        .hasDepthTest = true,
        .hasDepthWrite = true,
        .hasConservativeRaster = false,
      }
      });



    //
    qfAndQueue = ANAMED::QueueGetInfo{ 0u, 0u };
    fences = std::make_shared<std::array<ANAMED::FenceType, 8>>();

    //
    previousTime = glfwGetTime();
    frameCount = 0;

    //
    {
      stbi_ldr_to_hdr_scale(1.0f);
      stbi_ldr_to_hdr_gamma(2.2f);

      //
      int w = 0, h = 0, c = 0;
      float* data = (float*)stbi_loadf("./background.hdr", &w, &h, &c, STBI_rgb_alpha);

      //
      backgroundObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
        .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
        .imageInfo = ANAMED::ImageCreateInfo{
          .format = vk::Format::eR32G32B32A32Sfloat,
          .extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u},
          .type = ANAMED::ImageType::eTexture
        }
        });

      //
      decltype(auto) pair = backgroundObj->createImageView(ANAMED::ImageViewCreateInfo{
        .viewType = vk::ImageViewType::e2D
        });


      // complete loader
      //
      decltype(auto) status = uploaderObj->executeUploadToResourceOnce(ANAMED::UploadExecutionOnce{
        .host = cpp21::data_view<char8_t>((char8_t*)data, 0ull, h * w * 16ull),
        .writeInfo = ANAMED::UploadCommandWriteInfo{
          // # yet another std::optional problem (implicit)
          .dstImage = std::optional<ANAMED::ImageRegion>(ANAMED::ImageRegion{.image = backgroundObj.as<vk::Image>(), .region = ANAMED::ImageDataRegion{.extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u}}}),
        }
        });

      //
      descriptorsObj->updateDescriptors();

      //
      uniformData.backgroundObj = std::get<1u>(pair);
    }

    //
    {
      //
      int w = 0, h = 0, c = 0;
      float* data = (float*)stbi_loadf("./BlueNoise470.png", &w, &h, &c, STBI_rgb_alpha);

      //
      blueNoiseObj = ANAMED::ResourceObj::make(deviceObj, ANAMED::ResourceCreateInfo{
        .descriptors = descriptorsObj.as<vk::PipelineLayout>(),
        .imageInfo = ANAMED::ImageCreateInfo{
          .format = vk::Format::eR8G8B8A8Uint,
          .extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u},
          .type = ANAMED::ImageType::eTexture
        }
        });

      //
      decltype(auto) pair = blueNoiseObj->createImageView(ANAMED::ImageViewCreateInfo{
        .viewType = vk::ImageViewType::e2D
        });


      // complete loader
      //
      decltype(auto) status = uploaderObj->executeUploadToResourceOnce(ANAMED::UploadExecutionOnce{
        .host = cpp21::data_view<char8_t>((char8_t*)data, 0ull, h * w * 4ull),
        .writeInfo = ANAMED::UploadCommandWriteInfo{
          // # yet another std::optional problem (implicit)
          .dstImage = std::optional<ANAMED::ImageRegion>(ANAMED::ImageRegion{.image = blueNoiseObj.as<vk::Image>(), .region = ANAMED::ImageDataRegion{.extent = vk::Extent3D{uint32_t(w), uint32_t(h), 1u}}}),
        }
        });

      //
      descriptorsObj->updateDescriptors();

      //
      uniformData.blueNoiseObj = std::get<1u>(pair);
    };
  };

};

#endif
