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
#include <vector>
#include <string>
#include <memory>
#include <string_view>
#include <utility>
#include <map>
#include <future>
#include <iostream>
#include <optional>
#include <coroutine>
#include <tuple>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <iterator>
#include <fstream>
#include <typeindex>
#include <coroutine>
#include <experimental/generator>
#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <robin_hood.h>