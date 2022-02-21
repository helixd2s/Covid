cmake_minimum_required(VERSION 3.20)
project(LXVC VERSION 0.1.0)
set(PROJECT_NAME LXVC)

# 
find_package(Vulkan REQUIRED FATAL_ERROR)

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
add_executable(${PROJECT_NAME} ${_source_list})
target_compile_definitions(${PROJECT_NAME} PUBLIC )
target_include_directories(${PROJECT_NAME} PUBLIC 
  ${PROJECT_SOURCE_DIR}/project/include 
  ${PROJECT_SOURCE_DIR}/prebuild/include 
  ${PROJECT_SOURCE_DIR}/external/include 
  ${PROJECT_SOURCE_DIR}/submodules/*/include 
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
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} ${Vulkan_LIBRARIES} glfw3)

# 
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /bigobj /MP /incremental /debug:fastlink /experimental /errorReport /Og /Oi /Oa")
endif()

# 
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()
