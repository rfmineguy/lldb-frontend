# ================================================
#     Platform-independent LLVM + LLDB Setup
# ================================================

# Allow user to specify LLVM_DIR or use CMAKE_PREFIX_PATH
# If not set, try to find LLVM via find_package

find_package(LLVM REQUIRED CONFIG)

if (NOT LLVM_FOUND)
  message(FATAL_ERROR "LLVM not found. Set LLVM_DIR or add to CMAKE_PREFIX_PATH.")
endif()

set(LLVM_INCLUDE_DIR "${LLVM_INCLUDE_DIRS}")
set(LLVM_LIB_DIR "${LLVM_LIBRARY_DIRS}")
set(LLDB_INCLUDE_DIR "${LLDB_INCLUDE_DIRS}")

list(APPEND CMAKE_PREFIX_PATH "${LLVM_DIR}")

message(STATUS "LLVM found at: ${LLVM_DIR}")
message(STATUS "LLVM Include: ${LLVM_INCLUDE_DIR}")
message(STATUS "LLVM Libs: ${LLVM_LIB_DIR}")

if (NOT TARGET LLDB::liblldb)
   # Candidate lists per platform
    set(LLDB_STATIC_CANDIDATES)
    set(LLDB_SHARED_CANDIDATES)

    if (WIN32)
        list(APPEND LLDB_STATIC_CANDIDATES "${LLVM_LIB_DIR}/liblldb.lib")
        list(APPEND LLDB_SHARED_CANDIDATES "${LLVM_LIB_DIR}/liblldb.dll")
    elseif (APPLE)
        list(APPEND LLDB_STATIC_CANDIDATES "${LLVM_LIB_DIR}/liblldb.a")
        list(APPEND LLDB_SHARED_CANDIDATES "${LLVM_LIB_DIR}/liblldb.dylib")
    elseif (UNIX)
        list(APPEND LLDB_STATIC_CANDIDATES "${LLVM_LIB_DIR}/liblldb.a")
        list(APPEND LLDB_SHARED_CANDIDATES
            "${LLVM_LIB_DIR}/liblldb.so"
            "${LLVM_LIB_DIR}/liblldb.so.20.1.7"
            "${LLVM_LIB_DIR}/liblldb.so.20.1"
            "${LLVM_LIB_DIR}/liblldb.so.1"
        )
    endif()

    set(LLDB_LIB_FILE "")
    set(LLDB_LIB_TYPE "")

    foreach(CANDIDATE ${LLDB_STATIC_CANDIDATES})
        if (EXISTS "${CANDIDATE}")
            set(LLDB_LIB_FILE "${CANDIDATE}")
            set(LLDB_LIB_TYPE STATIC)
            break()
        endif()
    endforeach()

    if (NOT LLDB_LIB_FILE)
        foreach(CANDIDATE ${LLDB_SHARED_CANDIDATES})
            if (EXISTS "${CANDIDATE}")
                set(LLDB_LIB_FILE "${CANDIDATE}")
                set(LLDB_LIB_TYPE SHARED)
                break()
            endif()
        endforeach()
    endif()

    if (NOT LLDB_LIB_FILE)
        message(FATAL_ERROR "No LLDB static or shared library found in ${LLVM_LIB_DIR}")
    endif()

    add_library(LLDB::liblldb ${LLDB_LIB_TYPE} IMPORTED GLOBAL)

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

    if (APPLE)
        target_link_libraries(LLDB::liblldb INTERFACE "-framework Foundation" "-framework CoreFoundation")
    elseif (UNIX AND NOT ANDROID)
        target_link_libraries(LLDB::liblldb INTERFACE pthread dl)
    elseif (WIN32)
        target_link_libraries(LLDB::liblldb INTERFACE shlwapi psapi)
    endif()

endif()
