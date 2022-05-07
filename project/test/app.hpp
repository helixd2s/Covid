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
  uint32_t framebufferAttachments[8] = { 0u,0u,0u,0u };
  glm::uvec2 extent = {}; uint32_t frameCounter, reserved;
  Constants constants = {};
  uint64_t pixelData = 0ull;
  uint64_t writeData = 0ull;
  uint64_t rasterData = 0ull;
  uint64_t surfaceData = 0ull;
  uint32_t backgroundObj = 0u;
};

//
struct CounterData {
  uint32_t counters[4];
  uint32_t previousCounters[4];
};

//
struct PixelHitInfo {
  glm::uvec4 indices;
  glm::vec4 origin;
};

//
struct PixelSurfaceInfo {
  glm::uvec4 indices;
  glm::vec3 origin;
  glm::vec3 normal;
  glm::vec4 tex[2];
  glm::uvec4 accum[3];
  glm::vec4 color[3];
};

//
struct RasterInfo {
  glm::uvec4 indices = glm::uvec4(0u); // indlude .W are pNext
  glm::vec4 barycentric = glm::vec4(0.f);
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
  ANAMED::WrapShared<ANAMED::PipelineObj> resampleObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> pathTracerObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> opaqueObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> postObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> controlObj = {};
  ANAMED::WrapShared<ANAMED::PipelineObj> translucentObj = {};
  ANAMED::WrapShared<ANAMED::SwapchainObj> swapchainObj = {};
  ANAMED::WrapShared<ANAMED::FramebufferObj> framebufferObj = {};
  ANAMED::WrapShared<ANAMED::PingPongObj> pingPongObj = {};
  ANAMED::WrapShared<ANAMED::ResourceObj> backgroundObj = {};
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
    auto persp = glm::perspective(60.f / 180 * glm::pi<float>(), float(renderArea.extent.width) / float(renderArea.extent.height), 0.001f, 20000.f);
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
    //gltfLoaderObj->updateInstances(0u, glm::dmat4(1.f) * glm::scale(glm::dmat4(1.0f), glm::dvec3(1.f * scale, 1.f * scale, 1.f * scale)) * glm::rotate(glm::dmat4(1.0f), (controller->time - controller->beginTime) * 0.01, glm::dvec3(0.f, 1.f, 0.f)));
    //gltfLoaderObj->updateNodes(glm::dmat4(1.f) * glm::scale(glm::dmat4(1.0f), glm::dvec3(1.f * scale, 1.f * scale, 1.f * scale)));

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
    //pingPongObj->clearImage(qfAndQueue, 1u, glm::uintBitsToFloat(glm::uvec4(0u)));
    //pingPongObj->clearImage(qfAndQueue, 5u, glm::uintBitsToFloat(glm::uvec4(0u)));
    //pingPongObj->clearImage(qfAndQueue, 7u, glm::uintBitsToFloat(glm::uvec4(0u)));

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
    decltype(auto) controlFence = controlObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{1u, 1u, 1u},
        .layout = descriptorsObj.as<vk::PipelineLayout>(),
        .swapchain = swapchainObj.as<uintptr_t>(),
        .pingpong = pingPongObj.as<uintptr_t>(),
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue
      }
    });

    // 
    /*decltype(auto) counterFence = descriptorsObj->executeCacheUpdateOnce(ANAMED::CacheDataSet{
      // # yet another std::optional problem (implicit)
      .writeInfo = std::optional<ANAMED::CacheDataWriteSet>(ANAMED::CacheDataWriteSet{
        .region = ANAMED::DataRegion{0ull, 4ull, sizeof(CounterData)},
        .data = cpp21::data_view<char8_t>((char8_t*)&counterData, 0ull, sizeof(CounterData)),
        .page = 0u
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
    });*/


    //
    framebufferObj->clearAttachments(qfAndQueue);

    //
    decltype(auto) opaqueFence = opaqueObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
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
    decltype(auto) translucentFence = translucentObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
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
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 32u), cpp21::tiled(renderArea.extent.height, 8u), 1u},
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
    decltype(auto) computeFence = pathTracerObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 32u), cpp21::tiled(renderArea.extent.height, 8u), 1u},
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
    decltype(auto) postFence = postObj->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 32u), cpp21::tiled(renderArea.extent.height, 8u), 1u},
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
        .size = sizeof(RasterInfo) * renderArea.extent.width * renderArea.extent.height * 4u,
        .type = ANAMED::BufferType::eStorage,
      }
    });

    //
    uniformData.surfaceData = surfaceDataObj->getDeviceAddress();
    uniformData.pixelData = pixelDataObj->getDeviceAddress();
    uniformData.writeData = writeDataObj->getDeviceAddress();
    uniformData.rasterData = rasterDataObj->getDeviceAddress();

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
      .split = std::vector<uint32_t>{ 4, 4, 4, 4, 4},
      .formats = std::vector<vk::Format>{ vk::Format::eR32Uint, vk::Format::eR32Uint, vk::Format::eR32Uint, vk::Format::eR32Uint, vk::Format::eR32Uint },
      .info = qfAndQueue
    });

    // 
    decltype(auto) framebufferAttachments = framebufferObj->getImageViewIndices();
    memcpy(uniformData.framebufferAttachments, framebufferAttachments.data(), std::min(framebufferAttachments.size(), 8ull) * sizeof(uint32_t));
  };

  //
  void loadModel(std::string model, float scale = 1.f) {
    instanceAddressBlock = ANAMED::InstanceAddressBlock{
      .opaqueAddressInfo = (modelObj = gltfLoaderObj->load(model, this->scale = scale))->getDefaultScene()->instanced->getAddressInfo()
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
    descriptorsObj = ANAMED::PipelineLayoutObj::make(deviceObj.with(0u), ANAMED::DescriptorsCreateInfo{

    });

    //
    uploaderObj = ANAMED::UploaderObj::make(deviceObj, ANAMED::UploaderCreateInfo{

    });

    //
    gltfLoaderObj = ANAMED::GltfLoaderObj::make(deviceObj, ANAMED::GltfLoaderCreateInfo{
      .uploader = uploaderObj.as<uintptr_t>(),
      .descriptors = descriptorsObj.as<vk::PipelineLayout>()
    });

    //
    pathTracerObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./path-tracer.comp.spv")
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
    postObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./post.comp.spv")
      }
    });

    //
    controlObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .compute = ANAMED::ComputePipelineCreateInfo{
        .code = cpp21::readBinaryU32("./control.comp.spv")
      }
    });

    //
    std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> opaqueStageMaps = {};
    opaqueStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./opaque.vert.spv");
    opaqueStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./opaque.frag.spv");
    opaqueObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .graphics = ANAMED::GraphicsPipelineCreateInfo{
        .stageCodes = opaqueStageMaps
      }
    });

    //
    std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> translucentStageMaps = {};
    translucentStageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./translucent.vert.spv");
    translucentStageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./translucent.frag.spv");
    translucentObj = ANAMED::PipelineObj::make(deviceObj.with(0u), ANAMED::PipelineCreateInfo{
      .layout = descriptorsObj.as<vk::PipelineLayout>(),
      .graphics = ANAMED::GraphicsPipelineCreateInfo{
        .stageCodes = translucentStageMaps
      }
    });

    //
    qfAndQueue = ANAMED::QueueGetInfo{ 0u, 0u };
    fences = std::make_shared<std::array<ANAMED::FenceType, 8>>();

    //
    previousTime = glfwGetTime();
    frameCount = 0;

    //
    stbi_ldr_to_hdr_scale(1.0f);
    stbi_ldr_to_hdr_gamma(2.2f);

    //
    int w = 0, h = 0, c = 0;
    float* data = (float*)stbi_loadf("./HDR_111_Parking_Lot_2_Ref.hdr", &w, &h, &c, STBI_rgb_alpha);

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
  };

};

#endif
