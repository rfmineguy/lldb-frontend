set(FMT_INSTALL OFF)
FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG master GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(fmt)