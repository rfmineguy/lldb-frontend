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