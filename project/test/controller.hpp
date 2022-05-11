#pragma once
#define GLM_FORCE_SWIZZLE

#ifndef USE_CMAKE_PCH
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#include <cmake_pch.hxx>
#endif

// 
#include "./glfw-listener.hpp"

//
inline static bool terminated = false;

//
class Controller {
public:
  glm::vec3 viewPos = glm::vec3(0.f, 0.f, 0.f);
  glm::vec3 viewCnt = glm::vec3(0.f, 0.f, 1.f);
  glm::vec3 viewDir = glm::vec3(0.f, 0.f, 1.f);
  glm::vec3 viewUp = glm::vec3(0.f, 1.f, 0.f);

  //
  double moveSpeed = 10.f;
  double viewSpeed = 1.f;

  // 
  bool hasEscPressed = false;
  std::shared_ptr<GLFWListener> listener = {};

  //
  using ctl = Controller;

  //
  bool keys[350] = { false };
  bool mouse[16] = { false };

  // 
  double dx = 0.f, dy = 0.f;
  double mx = 0.f, my = 0.f;

  //
  double beginTime = 0.f;
  double time = 0.f;
  double dt = 0.f;

  //
  int width = 1u;
  int height = 1u;

  //
  bool needsClear = false;

  // 
  Controller(std::shared_ptr<GLFWListener> listener = {}) : listener(listener) {
    if (listener->window) {
      glfwGetCursorPos(listener->window, &mx, &my);
      glfwGetWindowSize(listener->window, &width, &height);

      // 
      listener->registerKeyCallback([this](GLFWwindow* window, int key, int scancode, int action, int mods){
        this->handleKey(key, action);
      });
      listener->registerMouseMoveCallback([this](GLFWwindow* window, double xpos, double ypos){
        this->handleMousePos(xpos, ypos);
      });
      listener->registerMouseButtonCallback([this](GLFWwindow* window, int button, int action, int mods){
        this->handleMouseKey(button, action);
      });
    };
    time = glfwGetTime();
    beginTime = glfwGetTime();
    viewCnt = viewPos + viewDir;
  };

  // 
  void handleMousePos() {
    double mx = 0.f, my = 0.f; glfwGetCursorPos(listener->window, &mx, &my);
    this->dx = mx - this->mx, this->dy = my - this->my, this->mx = mx, this->my = my;
    //this->handleAction();
  };

  // 
  void handleMousePos(double mx, double my) {
    //dx = mx - mx, dy = my - my, mx = mx, my = my;
    //this->handleAction();
  };

  //
  void handleKey(int key, int action) {
    keys[key] = action == GLFW_PRESS ? true : (action == GLFW_RELEASE ? false : keys[key]);
    //this->handleAction();
  };

  //
  void handleMouseKey(int key, int action) {
    mouse[key] = action == GLFW_PRESS ? true : (action == GLFW_RELEASE ? false : mouse[key]);
    //this->handleAction();
  };

  //
  void handleTime() {
    double time = glfwGetTime();
    dt = time - this->time;
    this->time = time;
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
    if (keys[GLFW_KEY_ESCAPE] && !hasEscPressed) { hasEscPressed = true; };
    if (keys[GLFW_KEY_ESCAPE] && hasEscPressed) { glfwTerminate(); exit(0); terminated = true; };

    // 
    glm::mat4 lkt = glm::lookAt(viewPos, viewPos + viewDir, viewUp);
    glm::mat4 xrot = glm::rotate(glm::mat4x4(1.f), float(dx / double(height) * viewSpeed), glm::vec3(0.0, -1.0, 0.0));
    glm::mat4 yrot = glm::rotate(glm::mat4x4(1.f), float(dy / double(height) * viewSpeed), glm::vec3(-1.0, 0.0, 0.0));

    //
    glm::vec3 viewPosLocal = (lkt * glm::vec4(viewPos, 1.f)).xyz();
    glm::vec3 viewDirLocal = (glm::vec4(viewDir, 1.f) * glm::inverse(lkt)).xyz();
    glm::vec3 moveDir = glm::vec3(0.f);

    //
    if (mouse[GLFW_MOUSE_BUTTON_1] && (glm::abs(dx) > 0.0 || glm::abs(dy) > 0.0)) {
      viewDirLocal = (xrot * glm::vec4(viewDirLocal, 1.0)).xyz();
      viewDirLocal = (yrot * glm::vec4(viewDirLocal, 1.0)).xyz();
      needsClear = true;
    };

    // Z, forward should be correct
    bool doMove = false;

    // 
    if (keys[GLFW_KEY_UP] || keys[GLFW_KEY_W]) {
      moveDir.z -= 1.f, doMove = true;
    };
    if (keys[GLFW_KEY_DOWN] || keys[GLFW_KEY_S]) {
      moveDir.z += 1.f, doMove = true;
    };

    // X, right should be right i.e. positive x, left is left i.e. negative x
    if (keys[GLFW_KEY_LEFT] || keys[GLFW_KEY_A]) {
      moveDir.x -= 1.f, doMove = true;
    };
    if (keys[GLFW_KEY_RIGHT] || keys[GLFW_KEY_D]) {
      moveDir.x += 1.f, doMove = true;
    };

    // Y, up should be right i.e. positive y or negative y relative vulkan coordinate system
    if (keys[GLFW_KEY_SPACE]) {
      moveDir.y += 1.f, doMove = true;
    };
    if (keys[GLFW_KEY_LEFT_SHIFT]) {
      moveDir.y -= 1.f, doMove = true;
    };

    //
    if (doMove && glm::length(moveDir) > 0.f) {
      viewPosLocal += float(dt * this->moveSpeed) * glm::normalize(moveDir);
      needsClear = true;
    };

    //
    viewPos = (glm::inverse(lkt) * glm::vec4(viewPosLocal, 1.f)).xyz();
    viewDir = glm::normalize((glm::vec4(viewDirLocal, 1.f) * lkt).xyz());
    viewCnt = viewPos + viewDir;
  };
};

