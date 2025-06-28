cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

# ================================================
#     Setup project
# ================================================
project(lldb-frontend LANGUAGES C CXX)

# ================================================
#     Setup libraries with FetchContent
# ================================================
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG docking)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  file(COPY cmake/imgui/CMakeLists.txt DESTINATION ${imgui_SOURCE_DIR})
  include_directories(${glfw_SOURCE_DIR}/include/)
  add_subdirectory(${imgui_SOURCE_DIR} ${imgui_BINARY_DIR})
endif()

add_library(glad libs/glad/src/gl.c)
add_library(tfd  libs/tfd/tinyfiledialogs.c)

# ================================================
#     Setup LLVM prefix
# ================================================
if(APPLE)
  execute_process(COMMAND brew --prefix llvm
                  OUTPUT_VARIABLE LLVM_PREFIX
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (LLVM_PREFIX)
    list(APPEND CMAKE_PREFIX_PATH "${LLVM_PREFIX}/lib/cmake")
  endif()
else()
  message(FATAL_ERROR "Your platform is not supported yet.")
endif()

# ================================================
#     Setup LLVM prefix
# ================================================
set(LLDB_INCLUDE_DIR "${LLVM_PREFIX}/include/")
set(LLDB_LIB_DIR "${LLVM_PREFIX}/lib/")


# ================================================
#     Setup main lldb-frontend executable and linker data
# ================================================
set(SOURCES 
  src/main.cpp 
  src/Window.cpp 
  src/Init.cpp 
  src/ImGuiLayer.cpp
  src/LLDBDebugger.cpp
  src/FileHeirarchy.cpp
  src/Util.cpp
  src/Logger.cpp)

add_executable(lldb-frontend ${SOURCES})

message(${CMAKE_SOURCE_DIR})
target_link_libraries(lldb-frontend PRIVATE lldb glfw glad imgui tfd)
target_link_directories(lldb-frontend PRIVATE ${LLDB_LIB_DIR})
target_include_directories(lldb-frontend PRIVATE ${LLDB_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libs/glad/include ${CMAKE_SOURCE_DIR}/libs/tfd)

set(TEST_SOURCES
  test/test.cpp
  test/test_support.cpp)

add_executable(lldb-frontend-test ${TEST_SOURCES})
