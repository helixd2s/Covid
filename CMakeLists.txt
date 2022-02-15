cmake_minimum_required(VERSION 3.17)
project(LXVC VERSION 0.1.0)
set(PROJECT_NAME LXVC)

include(CTest)
enable_testing()
set(CMAKE_CXX_STANDARD 20)
set(_src_root_path "${PROJECT_SOURCE_DIR}/project")
file(GLOB_RECURSE _source_list 
  LIST_DIRECTORIES false
  "${_src_root_path}/include/neo/*.hpp"
  "${_src_root_path}/include/neo/*.h"
  "${_src_root_path}/src/neo/*.cpp"
  "${_src_root_path}/src/neo/*.c"
	"${_src_root_path}/test/test.cpp"
)

add_executable(${PROJECT_NAME} ${_source_list})
target_compile_definitions(${PROJECT_NAME} PUBLIC )
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/prebuild/include ${PROJECT_SOURCE_DIR}/../external/include)
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/prebuild/lib ${PROJECT_SOURCE_DIR}/../external/lib)
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} glfw3)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP /incremental /debug:fastlink")
endif()

foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()
