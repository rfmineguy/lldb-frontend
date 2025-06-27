cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
add_library(imgui
    imconfig.h
    imgui.cpp
    imgui.h

    misc/cpp/imgui_stdlib.cpp
    misc/cpp/imgui_stdlib.h

    imgui_demo.cpp
    imgui_draw.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_glfw.h
    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_opengl3.h
    backends/imgui_impl_opengl3_loader.h
    imgui_internal.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    )

target_include_directories(imgui PUBLIC ../imgui)
target_include_directories(imgui PUBLIC . .. ../backends)
