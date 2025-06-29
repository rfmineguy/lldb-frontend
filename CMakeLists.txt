cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FETCHCONTENT_QUIET OFF)
include(FetchContent)

include(cmake/Version.cmake)
project(lldb-frontend LANGUAGES C CXX VERSION ${LDBF_V_MAJ}.${LDBF_V_MIN}.${LDBF_V_PAT}.${LDBF_V_TWE})

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
  src/Logger.cpp)

add_executable(lldb-frontend ${SOURCES})
target_compile_features(lldb-frontend PRIVATE cxx_std_23)
target_link_libraries(lldb-frontend PRIVATE glfw glad imgui tfd LLDB::liblldb)
target_link_directories(lldb-frontend PRIVATE ${LLVM_LIB_DIR})
target_include_directories(lldb-frontend PRIVATE ${LLVM_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libs/glad/include ${CMAKE_SOURCE_DIR}/libs/tfd)

install(TARGETS lldb-frontend
  RUNTIME DESTINATION bin COMPONENT Runtime)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/ DESTINATION share/lldb-frontend COMPONENT Runtime FILES_MATCHING PATTERN "*.ini")

set(TEST_SOURCES
  test/test.cpp
  test/test_support.cpp)

add_executable(lldb-frontend-test ${TEST_SOURCES})
target_compile_features(lldb-frontend-test PRIVATE cxx_std_23)

install(TARGETS lldb-frontend-test
  RUNTIME DESTINATION bin COMPONENT Tests)

# install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION share/doc COMPONENT Runtime OPTIONAL)

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

file(COPY imgui.ini DESTINATION ${CMAKE_BINARY_DIR})
