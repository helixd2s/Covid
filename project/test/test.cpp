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
#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

// 
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//
#include <args/args.hxx>
#include "controller.hpp"
#include "app.hpp"

// 
void error(int errnum, const char* errmsg)
{
  std::cerr << errnum << ": " << errmsg << std::endl;
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

  /*
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
  */

  //
  ANAMED::initialize();
  decltype(auto) app = std::make_shared<App>();
  app->loadModel(args::get(modelflag), args::get(scaleflag));

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
  std::string title = "Alter.TEON.A";
  decltype(auto) window = glfwCreateWindow(SC_WIDTH, SC_HEIGHT, title.c_str(), nullptr, nullptr);
  
  //
  app->initSurface(window);

  //
  decltype(auto) rendering = app->renderGen();
  decltype(auto) iterator = rendering.begin();

  // 
  while (!glfwWindowShouldClose(window)) { // 
    glfwPollEvents();
    app->tickProcessing();
    _CrtDumpMemoryLeaks();

    //
    if (iterator == rendering.end()) { rendering = app->renderGen(), iterator = rendering.begin(); };
    iterator++;
  };

  // 
  return 0;
};
#endif
