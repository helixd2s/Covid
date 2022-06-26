#pragma once

// 
#ifdef __cplusplus

//
#define GLM_FORCE_SWIZZLE

//
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_TYPESAFE_CONVERSION
#define VULKAN_HPP_NO_SMART_HANDLE
#define VULKAN_HPP_DISABLE_IMPLICIT_RESULT_VALUE_CAST
#define USE_NSIGHT_AFTERMATH 1

//
#ifdef VULKAN_HPP_ASSERT_ON_RESULT
#undef VULKAN_HPP_ASSERT_ON_RESULT
#endif

//
#ifndef VULKAN_HPP_ASSERT_ON_RESULT
#define VULKAN_HPP_ASSERT_ON_RESULT(result)  
#endif

//
//

//
//namespace vku {
    //inline bool handleResult(bool const& result) { assert(result); return true; };
//};

//
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>

//
#include <cassert>
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
#include <span>
#include <numeric>


#ifdef _MSC_VER 
#include <experimental/generator>
#else
#include <tl/generator.hpp>
#endif

//
#ifndef VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_CONSTRUCTORS
#endif

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
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

//
#ifdef ALT_ENABLE_GLTF
#include <tinygltf/tiny_gltf.h>
#include <tinygltf/stb_image.h>
#endif

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
#include <half.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

// 
#define VKU_ENABLE_INTERVAL
#include <cpp21.hpp>
#include <vk-utils.hpp>
#endif
