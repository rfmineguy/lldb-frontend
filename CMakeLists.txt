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

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  message(FATAL_ERROR "Build must be set to Debug")
else()
  message(STATUS "CMAKE_BUILD_TYPE is Debug")

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Compiler is Clang")
    if(WIN32)
      set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    else()
      set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    endif()

  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Compiler is GCC")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

  elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG docking GIT_SHALLOW TRUE)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  file(COPY cmake/imgui/CMakeLists.txt DESTINATION ${imgui_SOURCE_DIR})
  include_directories(${glfw_SOURCE_DIR}/include/)
  add_subdirectory(${imgui_SOURCE_DIR} ${imgui_BINARY_DIR})
endif()

FetchContent_Declare(argparse GIT_REPOSITORY https://github.com/p-ranav/argparse.git GIT_TAG v3.2 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(argparse)

add_library(glad libs/glad/src/gl.c)
add_library(tfd  libs/tfd/tinyfiledialogs.c)

FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG master GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(fmt)

include(cmake/LLVM.cmake)

set(SOURCES 
  src/main.cpp 
  src/Window.cpp 
  src/Init.cpp 
  src/ImGuiLayer.cpp
  src/ImGuiCustomWidgets.cpp
  src/LLDBDebugger.cpp
  src/FileHeirarchy.cpp
  src/Util.cpp
  src/Logger.cpp
  src/Args.cpp)

add_executable(lldb-frontend ${SOURCES})
target_compile_features(lldb-frontend PRIVATE cxx_std_23)
target_link_libraries(lldb-frontend PRIVATE glfw glad imgui tfd LLDB::liblldb argparse fmt)
target_link_directories(lldb-frontend PRIVATE ${LLVM_LIB_DIR})
target_include_directories(lldb-frontend PRIVATE 
  ${LLVM_INCLUDE_DIR} 
  ${CMAKE_SOURCE_DIR}/libs/glad/include 
  ${CMAKE_SOURCE_DIR}/libs/tfd
  ${argparse_INCLUDE_DIR}
  ${fmt_SOURCE_DIR}/include)

install(TARGETS lldb-frontend
  RUNTIME DESTINATION bin COMPONENT Runtime)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/ DESTINATION share/lldb-frontend COMPONENT Runtime FILES_MATCHING PATTERN "*.ini")
install(FILES ${CMAKE_BINARY_DIR}/imgui.ini DESTINATION bin COMPONENT Runtime)

set(TEST_SOURCES
  test/test.cpp
  test/test_support.cpp)

add_executable(lldb-frontend-test ${TEST_SOURCES})
target_compile_features(lldb-frontend-test PRIVATE cxx_std_23)

install(TARGETS lldb-frontend-test
  RUNTIME DESTINATION bin COMPONENT Tests)

if(WIN32 AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
  install(FILES "$<TARGET_PDB_FILE:lldb-frontend>" DESTINATION bin COMPONENT Runtime)
  install(FILES "$<TARGET_PDB_FILE:lldb-frontend-test>" DESTINATION bin COMPONENT Tests)
endif()

set(CPACK_PACKAGE_NAME "lldb-frontend")
set(CPACK_PACKAGE_VENDOR "lldb-frontend Contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LLDB Frontend UI")
set(CPACK_PACKAGE_VERSION_MAJOR ${LDBF_V_MAJ})
set(CPACK_PACKAGE_VERSION_MINOR ${LDBF_V_MIN})
set(CPACK_PACKAGE_VERSION_PATCH ${LDBF_V_PAT})
set(CPACK_PACKAGE_VERSION_TWEAK ${LDBF_V_TWE})
# set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README.md)
set(CPACK_PACKAGE_CONTACT "rfmineguy@gmail.com")
set(CPACK_GENERATOR "TGZ")

set(CPACK_COMPONENTS_ALL Runtime Tests)
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "LLDB Frontend Runtime")
set(CPACK_COMPONENT_TESTS_DISPLAY_NAME "LLDB Frontend Tests")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "The core executable and resources for lldb-frontend")
set(CPACK_COMPONENT_TESTS_DESCRIPTION "Unit tests for lldb-frontend")
string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSTEM_NAME_LOWER)
set(CPACK_PACKAGE_FILE_NAME "lldb-frontend-${SYSTEM_NAME_LOWER}")

include(CPack)

get_target_property(target_name lldb-frontend NAME)

# Determine the output base depending on target type
get_target_property(target_type ${target_name} TYPE)

if(target_type STREQUAL "EXECUTABLE" OR target_type STREQUAL "UTILITY")
    set(base_output_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
elseif(target_type STREQUAL "STATIC_LIBRARY" OR target_type STREQUAL "SHARED_LIBRARY")
    set(base_output_dir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
else()
    message(FATAL_ERROR "Unhandled target type: ${target_type}")
endif()

# If not set, use default
if(NOT base_output_dir)
    set(base_output_dir "${CMAKE_BINARY_DIR}")
endif()

# Add configuration subdir if using a multi-config generator
if(CMAKE_CONFIGURATION_TYPES)
    set(config "Debug") # Or your chosen configuration
    set(LLDB_FRONTEND_BUILD_DIRECTORY "${base_output_dir}/${config}")
else()
    set(LLDB_FRONTEND_BUILD_DIRECTORY "${base_output_dir}")
endif()

message(STATUS "Output directory for ${target_name} will be: ${LLDB_FRONTEND_BUILD_DIRECTORY}")

file(COPY imgui.ini DESTINATION ${LLDB_FRONTEND_BUILD_DIRECTORY})
