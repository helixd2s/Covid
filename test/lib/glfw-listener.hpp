#pragma once

//
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//
inline void CtlKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
inline void CtlMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
inline void CtlMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// 
class GLFWListener {
public:
  inline static std::unordered_map<uintptr_t, std::vector<std::function<void(GLFWwindow*, int, int, int, int)>>> KeyCallbacks = {};
  inline static std::unordered_map<uintptr_t, std::vector<std::function<void(GLFWwindow*, double, double)>>> MouseMoveCallbacks = {};
  inline static std::unordered_map<uintptr_t, std::vector<std::function<void(GLFWwindow*, int, int, int)>>> MouseButtonCallbacks = {};

  //
  GLFWwindow* window = nullptr;

  // 
  GLFWListener(GLFWwindow* window) : window(window) {
    glfwSetKeyCallback(window, CtlKeyCallback);
    glfwSetCursorPosCallback(window, CtlMouseMoveCallback);
    glfwSetMouseButtonCallback(window, CtlMouseButtonCallback);
  };

  //
  void registerKeyCallback(std::function<void(GLFWwindow*, int, int, int, int)> const& fn) {
    if (GLFWListener::KeyCallbacks.find(uintptr_t(window)) == GLFWListener::KeyCallbacks.end()) {
      GLFWListener::KeyCallbacks[uintptr_t(window)] = {};
    };
    GLFWListener::KeyCallbacks.at(uintptr_t(window)).push_back(fn);
  };

  //
  void registerMouseMoveCallback(std::function<void(GLFWwindow*, double, double)> const& fn) {
    if (GLFWListener::MouseMoveCallbacks.find(uintptr_t(window)) == GLFWListener::MouseMoveCallbacks.end()) {
      GLFWListener::MouseMoveCallbacks[uintptr_t(window)] = {};
    };
    GLFWListener::MouseMoveCallbacks.at(uintptr_t(window)).push_back(fn);
  };

  //
  void registerMouseButtonCallback(std::function<void(GLFWwindow*, int, int, int)> const& fn) {
    if (GLFWListener::MouseButtonCallbacks.find(uintptr_t(window)) == GLFWListener::MouseButtonCallbacks.end()) {
      GLFWListener::MouseButtonCallbacks[uintptr_t(window)] = {};
    };
    GLFWListener::MouseButtonCallbacks.at(uintptr_t(window)).push_back(fn);
  };
};

//
inline void CtlKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (GLFWListener::KeyCallbacks.find(uintptr_t(window)) != GLFWListener::KeyCallbacks.end()) {
    for (decltype(auto) fn : GLFWListener::KeyCallbacks.at(uintptr_t(window))) { fn(window, key, scancode, action, mods); };
  };
};

//
inline void CtlMouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
  if (GLFWListener::MouseMoveCallbacks.find(uintptr_t(window)) != GLFWListener::MouseMoveCallbacks.end()) {
    for (decltype(auto) fn : GLFWListener::MouseMoveCallbacks.at(uintptr_t(window))) { fn(window, xpos, ypos); };
  };
};

// 
inline void CtlMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  if (GLFWListener::MouseButtonCallbacks.find(uintptr_t(window)) != GLFWListener::MouseButtonCallbacks.end()) {
    for (decltype(auto) fn : GLFWListener::MouseButtonCallbacks.at(uintptr_t(window))) { fn(window, button, action, mods); };
  };
};
