#pragma once
#define GLM_FORCE_SWIZZLE

#ifndef USE_CMAKE_PCH
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#include <cmake_pch.hxx>
#endif

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
  double moveSpeed = 1.f;
  double viewSpeed = 1.f;

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
  inline static int width = 1u;
  inline static int height = 1u;

  // 
  Controller(GLFWwindow* window = nullptr) : window(window) {
    if (window) {
      glfwGetCursorPos(window, &mx, &my);
      glfwSetKeyCallback(window, CtlKeyCallback);
      glfwSetCursorPosCallback(window, CtlMouseMoveCallback);
      glfwSetMouseButtonCallback(window, CtlMouseKeyCallback);
      glfwGetWindowSize(window, &ctl::width, &ctl::height);
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
    glm::mat4 xrot = glm::rotate(glm::mat4x4(1.f), float(ctl::dx / double(ctl::height) * viewSpeed), glm::vec3(0.0, -1.0, 0.0));
    glm::mat4 yrot = glm::rotate(glm::mat4x4(1.f), float(ctl::dy / double(ctl::height) * viewSpeed), glm::vec3(-1.0, 0.0, 0.0));

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
