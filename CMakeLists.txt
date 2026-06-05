cmake_minimum_required(VERSION 4.2)
project(dev)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.c")

set(IMGUI_DIR "imgui")
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(dev
        ${SRC_FILES}
        ${IMGUI_SOURCES}
)

target_include_directories(dev PRIVATE
        src
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(dev PRIVATE
        OpenGL::GL
        glfw
)