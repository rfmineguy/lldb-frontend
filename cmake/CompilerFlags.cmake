if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  message(WARNING "Build is not set to Debug")
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