cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
message("${lldb_includes}")

# ================================================
#     Setup test executable
# ================================================
project(lldb-frontend-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(lldb-frontend-test test/test.cpp)


# ================================================
#     Setup project
# ================================================
project(lldb-frontend LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)


# ================================================
#     Setup main lldb-frontend executable and linker data
# ================================================
set(SOURCES src/main.cpp)

add_executable(lldb-frontend ${SOURCES})

target_link_libraries(lldb-frontend PRIVATE lldb)
target_link_directories(lldb-frontend PRIVATE "${LLDB_LIB_DIR}")
target_include_directories(lldb-frontend PRIVATE "${LLDB_INCLUDE_DIR}")
