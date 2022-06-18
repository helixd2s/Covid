cmake_minimum_required(VERSION 3.20)
project(Alter VERSION 0.1.0)
set(PROJECT_NAME Alter)

#
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

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
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(_src_root_path "${PROJECT_SOURCE_DIR}")
file(GLOB_RECURSE _source_list 
  LIST_DIRECTORIES false
  "${_src_root_path}/../vk-utils/*.inl"
  "${_src_root_path}/../vk-utils/*.hpp"
  "${_src_root_path}/../vk-utils/*.h"
  "${_src_root_path}/../vk-utils/cpp21/*.inl"
  "${_src_root_path}/../vk-utils/cpp21/*.hpp"
  "${_src_root_path}/../vk-utils/cpp21/*.h"
  "${_src_root_path}/include/*.inl"
  "${_src_root_path}/include/*.hpp"
  "${_src_root_path}/include/*.cpp"
  "${_src_root_path}/include/*.hxx"
  "${_src_root_path}/include/*.cxx"
  "${_src_root_path}/include/Alter/*.inl"
  "${_src_root_path}/include/Alter/*.hpp"
  "${_src_root_path}/include/Alter/*.h"
  "${_src_root_path}/src/Alter/*.inl"
  "${_src_root_path}/src/Alter/*.cpp"
  "${_src_root_path}/src/Alter/*.c"
  "${_src_root_path}/test/test.cpp"
  "${_src_root_path}/test/hdrloader.cpp"
  "${_src_root_path}/test/*.cpp"
  "${_src_root_path}/test/*.hpp"
  "${_src_root_path}/test/*.h"
)

#
SET(PLATFORM_DEF )
set(PLATFORM_HEAD )
IF (WIN32)
  SET (PLATFORM_DEF VK_USE_PLATFORM_WIN32_KHR)
  SET (PLATFORM_HEAD <windows.h>)
ELSE()
  SET (PLATFORM_DEF )
ENDIF()

# 
add_executable(${PROJECT_NAME} ${_source_list})
target_compile_definitions(${PROJECT_NAME} PUBLIC ALT_USE_ROBIN_HOOD USE_CMAKE_PCH VULKAN_HPP_NO_CONSTRUCTORS NOMINMAX ALT_ENABLE_VMA ALT_ENABLE_GLTF CPP21_DEBUG_MEMORY TINYGLTF_USE_CPP14 ANAMED=alter ${PLATFORM_DEF})
target_include_directories(${PROJECT_NAME} PUBLIC 
  ${PROJECT_SOURCE_DIR}/include/Alter 
  ${PROJECT_SOURCE_DIR}/include 
  ${PROJECT_SOURCE_DIR}/../vk-utils/
  ${PROJECT_SOURCE_DIR}/../vk-utils/interval-tree/include
  ${PROJECT_SOURCE_DIR}/../vk-utils/submodules/include 
  ${PROJECT_SOURCE_DIR}/../vk-utils/generator/include 
  ${PROJECT_SOURCE_DIR}/../vk-utils/glm/glm
  ${PROJECT_SOURCE_DIR}/../vk-utils/glm
  ${PROJECT_SOURCE_DIR}/../VulkanMemoryAllocator/include
  ${PROJECT_SOURCE_DIR}/../Vulkan-Headers/include
  ${PROJECT_SOURCE_DIR}/../glm/glm
  ${PROJECT_SOURCE_DIR}/../glm
  ${PROJECT_SOURCE_DIR}/../NRI/_NRI_SDK/Include
  ${PROJECT_SOURCE_DIR}/../RayTracingDenoiser/_NRD_SDK/Integration
  ${PROJECT_SOURCE_DIR}/../RayTracingDenoiser/_NRD_SDK/Include
  ${Vulkan_INCLUDE_DIRS}
)

#
target_link_directories(${PROJECT_NAME} PUBLIC 
  ${PROJECT_SOURCE_DIR}/lib
  ${PROJECT_SOURCE_DIR}/../NRI/_NRI_SDK/Lib/Debug
  ${PROJECT_SOURCE_DIR}/../RayTracingDenoiser/_NRD_SDK/Lib/Debug
)

#
set (HSOURCES "${PROJECT_SOURCE_DIR}/../vk-utils/cpp21/core.hpp" "${PROJECT_SOURCE_DIR}/include/Alter/Core.hpp")

#
ADD_MSVC_PRECOMPILED_HEADER(
  "${PROJECT_SOURCE_DIR}/include/cmake_pch.hxx" 
  "${PROJECT_SOURCE_DIR}/include/cmake_pch.cxx" 
  HSOURCES)

#
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} ${Vulkan_LIBRARIES} glfw NRD NRI)

# 
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d1FastFail /std:c17 /std:c++latest /bigobj /MD /O0 /Od /Ob0 /Zc:threadSafeInit /errorReport /arch:AVX2")
    set(CMAKE_EXE_LINKER_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MD /MAP /incremental /debug:fastlink /experimental /errorReport /LTCG:PGInstrument")
endif()

# 
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()
