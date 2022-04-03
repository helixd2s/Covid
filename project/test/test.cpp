#pragma once

#ifdef __cplusplus
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
#define GLM_FORCE_SWIZZLE
#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

// 
#include <Alter/Alter.hpp>
#include <GLFW/glfw3.h>
#ifdef ENABLE_RENDERDOC
#include "renderdoc_app.h"
#include <eh.h>
#endif

//
#include <args/args.hxx>

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
  glm::uvec2 extent = {}; uint32_t frameCounter, reserved;
  Constants constants = {};
  //uint64_t verticesAddress = 0ull;
};

//
class Controller;

//
inline void CtlKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
inline void CtlMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
inline void CtlMouseKeyCallback(GLFWwindow* window, int button, int action, int mods);

//
class Controller {
public:
  glm::vec3 viewPos = glm::vec3(0.f, 0.f, 0.f);
  glm::vec3 viewCnt = glm::vec3(0.f, 0.f, 1.f);
  glm::vec3 viewDir = glm::vec3(0.f, 0.f, 1.f);
  glm::vec3 viewUp = glm::vec3(0.f, 1.f, 0.f);

  //
  double moveSpeed = 0.1f;
  double viewSpeed = 0.005f;

  // 
  bool hasEscPressed = false;
  GLFWwindow* window = nullptr;

  //
  using ctl = Controller;

  //
  inline static bool keys[350];
  inline static bool mouse[16];

  // 
  inline static double dx = 0.f, dy = 0.f;
  inline static double mx = 0.f, my = 0.f;

  //
  inline static double time = 0.f;
  inline static double dt = 0.f;

  // 
  Controller(GLFWwindow* window = nullptr) : window(window) {
    if (window) {
      glfwGetCursorPos(window, &mx, &my);
      glfwSetKeyCallback(window, CtlKeyCallback);
      glfwSetCursorPosCallback(window, CtlMouseMoveCallback);
      glfwSetMouseButtonCallback(window, CtlMouseKeyCallback);
    };
    time = glfwGetTime();
    viewCnt = viewPos + viewDir;
  };

  // 
  void handleMousePos() {
    double mx = 0.f, my = 0.f; glfwGetCursorPos(window, &mx, &my);
    ctl::dx = mx - ctl::mx, ctl::dy = my - ctl::my, ctl::mx = mx, ctl::my = my;
    //this->handleAction();
  };

  // 
  static void handleMousePos(double mx, double my) {
    //ctl::dx = mx - ctl::mx, ctl::dy = my - ctl::my, ctl::mx = mx, ctl::my = my;
    //this->handleAction();
  };

  //
  static void handleKey(int key, int action) {
    ctl::keys[key] = action == GLFW_PRESS ? true : (action == GLFW_RELEASE ? false : ctl::keys[key]);
    //this->handleAction();
  };

  //
  static void handleMouseKey(int key, int action) {
    ctl::mouse[key] = action == GLFW_PRESS ? true : (action == GLFW_RELEASE ? false : ctl::mouse[key]);
    //this->handleAction();
  };

  //
  static void handleTime() {
    double time = glfwGetTime();
    ctl::dt = time - ctl::time;
    ctl::time = time;
    //this->handleAction();
  };

  //
  void handleFrame() {
    this->handleTime();
    this->handleMousePos();
    this->handleAction();
  };

  //
  void handleAction() {
    using ctl = Controller;

    //
    if (ctl::keys[GLFW_KEY_ESCAPE] && !hasEscPressed) { hasEscPressed = true; };
    if (ctl::keys[GLFW_KEY_ESCAPE] && hasEscPressed) { glfwTerminate(); exit(0); };

    // 
    glm::mat4 lkt = glm::lookAt(viewPos, viewPos + viewDir, viewUp);
    glm::mat4 xrot = glm::rotate(glm::mat4x4(1.f), float(ctl::dx * viewSpeed), glm::vec3(0.0, -1.0, 0.0));
    glm::mat4 yrot = glm::rotate(glm::mat4x4(1.f), float(ctl::dy * viewSpeed), glm::vec3(-1.0, 0.0, 0.0));

    //
    glm::vec3 viewPosLocal = (lkt * glm::vec4(viewPos, 1.f)).xyz();
    glm::vec3 viewDirLocal = (glm::vec4(viewDir, 1.f) * glm::inverse(lkt)).xyz();
    glm::vec3 moveDir = glm::vec3(0.f);

    //
    if (ctl::mouse[GLFW_MOUSE_BUTTON_1]) {
      viewDirLocal = (xrot * glm::vec4(viewDirLocal, 1.0)).xyz();
      viewDirLocal = (yrot * glm::vec4(viewDirLocal, 1.0)).xyz();
    };

    // Z, forward should be correct
    bool doMove = false;

    // 
    if (ctl::keys[GLFW_KEY_UP] || ctl::keys[GLFW_KEY_W]) {
      moveDir.z -= 1.f, doMove = true;
    };
    if (ctl::keys[GLFW_KEY_DOWN] || ctl::keys[GLFW_KEY_S]) {
      moveDir.z += 1.f, doMove = true;
    };

    // X, right should be right i.e. positive x, left is left i.e. negative x
    if (ctl::keys[GLFW_KEY_LEFT] || ctl::keys[GLFW_KEY_A]) {
      moveDir.x -= 1.f, doMove = true;
    };
    if (ctl::keys[GLFW_KEY_RIGHT] || ctl::keys[GLFW_KEY_D]) {
      moveDir.x += 1.f, doMove = true;
    };

    // Y, up should be right i.e. positive y or negative y relative vulkan coordinate system
    if (ctl::keys[GLFW_KEY_SPACE]) {
      moveDir.y += 1.f, doMove = true;
    };
    if (ctl::keys[GLFW_KEY_LEFT_SHIFT]) {
      moveDir.y -= 1.f, doMove = true;
    };

    //
    if (doMove && glm::length(moveDir) > 0.f) {
      viewPosLocal += float(ctl::dt * this->moveSpeed) * glm::normalize(moveDir);
    };

    //
    viewPos = (glm::inverse(lkt) * glm::vec4(viewPosLocal, 1.f)).xyz();
    viewDir = glm::normalize((glm::vec4(viewDirLocal, 1.f) * lkt).xyz());
    viewCnt = viewPos + viewDir;
  };
};

//
inline void CtlKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  Controller::handleKey(key, action);
  //Controller::handleTime();
};

//
inline void CtlMouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
  Controller::handleMousePos(xpos, ypos);
  //Controller::handleTime();
};

// 
inline void CtlMouseKeyCallback(GLFWwindow* window, int button, int action, int mods) {
  Controller::handleMouseKey(button, action);
  //Controller::handleTime();
};



// 
int main(int argc, char** argv) {
  //
  args::ArgumentParser parser("This is a test rendering program.", "");
  args::HelpFlag help(parser, "help", "Available flags", { 'h', "help" });
  args::ValueFlag<float> scaleflag(parser, "scale", "Scaling of model object", { 's' }, 1.f);
  args::ValueFlag<std::string> modelflag(parser, "model", "Model to view", { 'm' }, "BoomBoxWithAxes.gltf");

  //
  try { parser.ParseCLI(argc, argv); }
  catch (args::Help) { std::cout << parser; glfwTerminate(); exit(1); };

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
  ANAMED::initialize();




  //
  //std::cout << "We running experimental renderer... continue?" << std::endl;
  //system("PAUSE");


  // first cherep
  decltype(auto) instance = ANAMED::InstanceObj::make(ANAMED::context, ANAMED::InstanceCreateInfo{

  });

  // second cherep
  decltype(auto) device = ANAMED::DeviceObj::make(instance, ANAMED::DeviceCreateInfo{
    .physicalDeviceGroupIndex = 0u,
    .physicalDeviceIndex = 0u
  });

  //
  decltype(auto) memoryAllocatorVma = ANAMED::MemoryAllocatorVma::make(device, ANAMED::MemoryAllocatorCreateInfo{

  });

  // final cherep for today
  decltype(auto) descriptors = ANAMED::DescriptorsObj::make(device.with(0u), ANAMED::DescriptorsCreateInfo{

  });

  //
  decltype(auto) uniformData = UniformData{};

  //
  decltype(auto) uploader = ANAMED::UploaderObj::make(device, ANAMED::UploaderCreateInfo{

  });

  //
  decltype(auto) gltfLoader = ANAMED::GltfLoaderObj::make(device, ANAMED::GltfLoaderCreateInfo{
    .uploader = uploader.as<uintptr_t>(),
    .descriptors = descriptors.as<vk::PipelineLayout>()
  });

  // 
  decltype(auto) modelObj = gltfLoader->load(args::get(modelflag), args::get(scaleflag));

  //
  decltype(auto) instanceAddressBlock = ANAMED::InstanceAddressBlock{
    .opaqueAddressInfo = modelObj->getDefaultScene()->instanced->getAddressInfo()
  };

  //
  decltype(auto) compute = ANAMED::PipelineObj::make(device.with(0u), ANAMED::PipelineCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .compute = ANAMED::ComputePipelineCreateInfo{
      .code = cpp21::readBinaryU32("./test.comp.spv")
    }
  });

  //
  std::unordered_map<vk::ShaderStageFlagBits, cpp21::shared_vector<uint32_t>> stageMaps = {};
  stageMaps[vk::ShaderStageFlagBits::eVertex] = cpp21::readBinaryU32("./opaque.vert.spv");
  stageMaps[vk::ShaderStageFlagBits::eFragment] = cpp21::readBinaryU32("./opaque.frag.spv");

  //
  decltype(auto) graphics = ANAMED::PipelineObj::make(device.with(0u), ANAMED::PipelineCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .graphics = ANAMED::GraphicsPipelineCreateInfo{
      .stageCodes = stageMaps
    }
  });

  //
  decltype(auto) qfAndQueue = ANAMED::QueueGetInfo{ 0u, 0u };
  //std::shared_ptr<std::array<ANAMED::FenceType, 4>> fences = std::make_shared<std::array<ANAMED::FenceType, 4>>();
  decltype(auto) fences = std::make_shared<std::array<ANAMED::FenceType, 8>>();

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
  std::string title = "Alter.TEON.A";
  decltype(auto) window = glfwCreateWindow(SC_WIDTH, SC_HEIGHT, title.c_str(), nullptr, nullptr);
  glfwCreateWindowSurface(instance.as<VkInstance>(), window, nullptr, (VkSurfaceKHR*)&surface);

  //
  decltype(auto) swapchain = ANAMED::SwapchainObj::make(device, ANAMED::SwapchainCreateInfo{
    .layout = descriptors.as<vk::PipelineLayout>(),
    .surface = surface,
    .info = qfAndQueue
  });

  //
  decltype(auto) renderArea = swapchain->getRenderArea();

  //
  Controller controller(window);

  //
  decltype(auto) framebuffer = ANAMED::FramebufferObj::make(device.with(0u), ANAMED::FramebufferCreateInfo{
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
  double previousTime = glfwGetTime();
  int frameCount = 0;



  //
  decltype(auto) renderGen = [=, &previousTime, &frameCount, &uniformData, &controller]() -> std::experimental::generator<bool> {
    co_yield false;

    // set perspective
    controller.handleFrame();
    auto persp = glm::perspective(60.f / 180 * glm::pi<float>(), float(renderArea.extent.width) / float(renderArea.extent.height), 0.001f, 10000.f);
    auto lkat = glm::lookAt(controller.viewPos, controller.viewCnt, controller.viewUp);
    uniformData.constants.perspective = glm::transpose(persp);
    uniformData.constants.perspectiveInverse = glm::transpose(glm::inverse(persp));
    uniformData.constants.lookAt = glm::mat3x4(glm::transpose(lkat));
    uniformData.constants.lookAtInverse = glm::mat3x4(glm::transpose(glm::inverse(lkat)));
    uniformData.extent = glm::uvec2(renderArea.extent.width, renderArea.extent.height);
    uniformData.frameCounter = 0u;

    //
#ifdef ENABLE_RENDERDOC
    if (rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
#endif

    //
    double currentTime = glfwGetTime();
    frameCount++;

    if (currentTime - previousTime >= 1.0)
    {
      // Display the frame count here any way you want.
      //displayFPS(frameCount);
      glfwSetWindowTitle(window, (std::string("Alter.TEON.A; FPS: ") + std::to_string(frameCount)).c_str());

      frameCount = 0;
      previousTime = currentTime;
    };

    // 
    decltype(auto) acquired = swapchain->acquireImage(qfAndQueue);

    // wait ready for filling
    auto& fence = (*fences)[acquired];
    decltype(auto) status = false;
    //if (fence) { decltype(auto) unleak = fence->future->get(); }; device->tickProcessing();
    if (fence) { while (!(status = fence->checkStatus())) { co_yield status; }; };

    //
    uniformData.frameCounter++;

    // 
    decltype(auto) uniformFence = descriptors->executeUniformUpdateOnce(ANAMED::UniformDataSet{
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
    framebuffer->clearAttachments(qfAndQueue);

    //
    decltype(auto) graphicsFence = graphics->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .graphics = std::optional<ANAMED::WriteGraphicsInfo>(ANAMED::WriteGraphicsInfo{
        .layout = descriptors.as<vk::PipelineLayout>(),
        .framebuffer = framebuffer.as<uintptr_t>(),
        .swapchain = swapchain.as<vk::SwapchainKHR>(),
        .instanceDraws = modelObj->getDefaultScene()->instanced->getDrawInfo(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
        .info = qfAndQueue,
      }
      });

    //
    decltype(auto) computeFence = compute->executePipelineOnce(ANAMED::ExecutePipelineInfo{
      // # yet another std::optional problem (implicit)
      .compute = std::optional<ANAMED::WriteComputeInfo>(ANAMED::WriteComputeInfo{
        .dispatch = vk::Extent3D{cpp21::tiled(renderArea.extent.width, 256u), renderArea.extent.height, 1u},
        .layout = descriptors.as<vk::PipelineLayout>(),
        .swapchain = swapchain.as<vk::SwapchainKHR>(),
        // # yet another std::optional problem (implicit)
        .instanceAddressBlock = std::optional<ANAMED::InstanceAddressBlock>(instanceAddressBlock)
      }),
      .submission = ANAMED::SubmissionInfo{
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
  decltype(auto) iterator = rendering.begin();

  // 
  while (!glfwWindowShouldClose(window)) { // 
    glfwPollEvents();
    device->tickProcessing();
    _CrtDumpMemoryLeaks();

    //
    if (iterator == rendering.end()) { rendering = renderGen(), iterator = rendering.begin(); };
    iterator++;
  };

  // 
  return 0;
};
#endif
