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