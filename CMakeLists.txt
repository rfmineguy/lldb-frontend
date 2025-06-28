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
#     Platform-independent LLVM + LLDB Setup
# ================================================

# Allow user to specify LLVM_DIR or use CMAKE_PREFIX_PATH
# If not set, try to find LLVM via find_package

# Optionally allow override
# set(LLVM_MINIMUM_VERSION 15.0 REQUIRED)

# Use CMAKE_PREFIX_PATH hint from environment or prior scripts
find_package(LLVM REQUIRED CONFIG)

# Ensure variables are populated
if (NOT LLVM_FOUND)
  message(FATAL_ERROR "LLVM not found. Set LLVM_DIR or add to CMAKE_PREFIX_PATH.")
endif()

# Set include and lib paths
set(LLVM_INCLUDE_DIR "${LLVM_INCLUDE_DIRS}")
set(LLVM_LIB_DIR "${LLVM_LIBRARY_DIRS}")
set(LLDB_INCLUDE_DIR "${LLDB_INCLUDE_DIRS}")

# Add to CMake path
list(APPEND CMAKE_PREFIX_PATH "${LLVM_DIR}")

# Print useful info
message(STATUS "LLVM found at: ${LLVM_DIR}")
message(STATUS "LLVM Include: ${LLVM_INCLUDE_DIR}")
message(STATUS "LLVM Libs: ${LLVM_LIB_DIR}")
message(STATUS "LLVM Libraries: ${LLVM_LIBRARIES}")

if (NOT TARGET LLDB::liblldb)

    add_library(LLDB::liblldb STATIC IMPORTED GLOBAL)

    # Per-platform lib filename
    if (WIN32)
        set(LLDB_LIB_FILE "${LLVM_LIB_DIR}/liblldb.lib")
    elseif (APPLE)
        set(LLDB_LIB_FILE "${LLVM_LIB_DIR}/liblldb.a")
    elseif (UNIX)
        set(LLDB_LIB_FILE "${LLVM_LIB_DIR}/liblldb.a")
    endif()

    if (NOT EXISTS "${LLDB_LIB_FILE}")
        message(FATAL_ERROR "liblldb not found at: ${LLDB_LIB_FILE}")
    endif()

    # Handle debug/release multi-config setups on Windows
    if (MSVC OR CMAKE_GENERATOR MATCHES "Visual Studio")
        set_target_properties(LLDB::liblldb PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release"
            IMPORTED_LOCATION_DEBUG "${LLDB_LIB_FILE}"
            IMPORTED_LOCATION_RELEASE "${LLDB_LIB_FILE}"
        )
    else()
        set_target_properties(LLDB::liblldb PROPERTIES
            IMPORTED_LOCATION "${LLDB_LIB_FILE}"
        )
    endif()

    set_target_properties(LLDB::liblldb PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LLDB_INCLUDE_DIR}"
    )

    # Platform-specific dependencies if needed
    if (APPLE)
        target_link_libraries(LLDB::liblldb INTERFACE "-framework Foundation" "-framework CoreFoundation")
    elseif (UNIX AND NOT ANDROID)
        target_link_libraries(LLDB::liblldb INTERFACE pthread dl)
    elseif (WIN32)
        target_link_libraries(LLDB::liblldb INTERFACE shlwapi psapi)
    endif()

endif()


# ================================================
#     Setup main lldb-frontend executable and linker data
# ================================================
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

set(TEST_SOURCES
  test/test.cpp
  test/test_support.cpp)

add_executable(lldb-frontend-test ${TEST_SOURCES})
target_compile_features(lldb-frontend-test PRIVATE cxx_std_23)