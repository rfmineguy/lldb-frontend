cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(WIN32)
  message(STATUS "WIN32 Detected: Using Static CRT")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
endif()

set(FETCHCONTENT_QUIET OFF)
include(FetchContent)

include(cmake/Version.cmake)

project(lldb-frontend LANGUAGES C CXX VERSION ${LDBF_V_MAJ}.${LDBF_V_MIN}.${LDBF_V_PAT}.${LDBF_V_TWE})

include(cmake/CompilerFlags.cmake)

set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glfw)
include_directories(${glfw_SOURCE_DIR}/include/)

include(cmake/ImGui.cmake)

FetchContent_Declare(argparse GIT_REPOSITORY https://github.com/p-ranav/argparse.git GIT_TAG v3.2 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(stb)

add_library(glad libs/glad/src/gl.c)
add_library(tfd  libs/tfd/tinyfiledialogs.c)

include(cmake/fmt.cmake)

include(cmake/LLVM.cmake)

set(SOURCES src/lldb-frontend.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad imgui tfd LLDB::liblldb argparse fmt-header-only)
target_link_directories(${PROJECT_NAME} PRIVATE ${LLVM_LIB_DIR})
message("stb source dir: ${stb_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${LLVM_INCLUDE_DIR} 
  ${CMAKE_SOURCE_DIR}/libs/glad/include 
  ${CMAKE_SOURCE_DIR}/libs/tfd
  ${CMAKE_SOURCE_DIR}/libs/stb
  ${imgui_SOURCE_DIR}
  ${stb_SOURCE_DIR}
  ${argparse_INCLUDE_DIR}
  ${fmt_SOURCE_DIR}/include)

# Unicode
if (NOT MSVC)
  # Unicode is always supported on compilers other than MSVC.
elseif (FMT_UNICODE)
  # Unicode support requires compiling with /utf-8.
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/utf-8>)
else ()
  target_compile_definitions(${PROJECT_NAME} PRIVATE FMT_UNICODE=0)
endif ()

set(TEST_SOURCES
  test/test.cpp
  test/test_support.cpp
  test/nested/nested.cpp)

add_executable(${PROJECT_NAME}-test ${TEST_SOURCES})
target_compile_features(${PROJECT_NAME}-test PRIVATE cxx_std_23)

include(cmake/Install.cmake)

include(cmake/CPack.cmake)

include(cmake/imgui.ini.cmake)
include(cmake/Resources.cmake)
