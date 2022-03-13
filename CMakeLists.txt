cmake_minimum_required(VERSION 3.20)
project(LXVC VERSION 0.1.0)
set(PROJECT_NAME LXVC)

# 
IF (WIN32)
  set(ENV{ASAN_OPTIONS} [windows_hook_legacy_allocators=true])

  #
  IF(NOT DEFINED VCPKG_PATH OR NOT DEFINED ${VCPKG_PATH})
    set (VCPKG_PATH "C:/vcpkg/")
  ENDIF()

  # trying load GLFW3 from VCPKG in Windows
  IF(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR NOT DEFINED ${CMAKE_TOOLCHAIN_FILE})
    set (CMAKE_TOOLCHAIN_FILE "${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake")
  ENDIF()

  IF(NOT DEFINED CMAKE_MODULE_PATH OR NOT DEFINED ${CMAKE_MODULE_PATH})
    set (CMAKE_MODULE_PATH "${VCPKG_PATH}/installed/x64-windows")
  ENDIF()
  
  IF(NOT DEFINED glfw3_DIR OR NOT DEFINED ${glfw3_DIR})
    set (glfw3_DIR "${CMAKE_MODULE_PATH}/share/glfw3")
  ENDIF()
ENDIF()

# 
find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glfw3 REQUIRED)

# 
set(CMAKE_CXX_STANDARD 20)
set(_src_root_path "${PROJECT_SOURCE_DIR}/project")
file(GLOB_RECURSE _source_list 
  LIST_DIRECTORIES false
  "${_src_root_path}/../prebuild/include/vk-utils/*.inl"
  "${_src_root_path}/../prebuild/include/vk-utils/*.hpp"
  "${_src_root_path}/../prebuild/include/vk-utils/*.h"
  "${_src_root_path}/../prebuild/include/vku/*.inl"
  "${_src_root_path}/../prebuild/include/vku/*.hpp"
  "${_src_root_path}/../prebuild/include/vku/*.h"
  "${_src_root_path}/../prebuild/include/vk-utils/cpp21/*.inl"
  "${_src_root_path}/../prebuild/include/vk-utils/cpp21/*.hpp"
  "${_src_root_path}/../prebuild/include/vk-utils/cpp21/*.h"
  "${_src_root_path}/../prebuild/include/vku/cpp21/*.inl"
  "${_src_root_path}/../prebuild/include/vku/cpp21/*.hpp"
  "${_src_root_path}/../prebuild/include/vku/cpp21/*.h"

  "${_src_root_path}/include/lxvc/*.inl"
  "${_src_root_path}/include/lxvc/*.hpp"
  "${_src_root_path}/include/lxvc/*.h"
  "${_src_root_path}/src/lxvc/*.inl"
  "${_src_root_path}/src/lxvc/*.cpp"
  "${_src_root_path}/src/lxvc/*.c"
  "${_src_root_path}/test/test.cpp"
  "${_src_root_path}/test/*"
)

#
SET(PLATFORM_DEF )
IF (WIN32)
  SET (PLATFORM_DEF VK_USE_PLATFORM_WIN32_KHR)
ELSE()
  SET (PLATFORM_DEF )
ENDIF()

# 
add_executable(${PROJECT_NAME} ${_source_list})
target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_HPP_NO_CONSTRUCTORS NOMINMAX ${PLATFORM_DEF})
target_include_directories(${PROJECT_NAME} PUBLIC 
  ${PROJECT_SOURCE_DIR}/project/include 
  ${PROJECT_SOURCE_DIR}/prebuild/include 
  ${PROJECT_SOURCE_DIR}/external/include 
  ${PROJECT_SOURCE_DIR}/submodules/*/include 
  ${PROJECT_SOURCE_DIR}/submodules/glm
  ${PROJECT_SOURCE_DIR}/../external/include 
  ${Vulkan_INCLUDE_DIRS}
)
target_link_directories(${PROJECT_NAME} PUBLIC 
  ${PROJECT_SOURCE_DIR}/project/lib 
  ${PROJECT_SOURCE_DIR}/prebuild/lib 
  ${PROJECT_SOURCE_DIR}/external/lib 
  ${PROJECT_SOURCE_DIR}/submodules/*/lib/
  ${PROJECT_SOURCE_DIR}/../external/lib
)
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} ${Vulkan_LIBRARIES} glfw)
target_precompile_headers(${PROJECT_NAME}
  PUBLIC
  <memory>
  <vector>
  <map>
  <future>
  <iostream>
  <optional>
  <coroutine>
  <tuple>
  <type_traits>
  <utility>
  <unordered_map>
  <iterator>
  <fstream>
  <typeindex>
  <vulkan/vulkan.h>
  <vulkan/vulkan.hpp>
  <vulkan/vulkan_raii.hpp>
  <glm/glm.hpp>
)

# 
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /bigobj /MP /EHa /fsanitize=address /Ot /Od /Oy /Ob0 /GT /GL /GR /Zc:threadSafeInit /arch:AVX2")
    set(CMAKE_EXE_LINKER_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP /MAP /incremental /debug:fastlink /experimental /errorReport")
endif()

# 
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()
