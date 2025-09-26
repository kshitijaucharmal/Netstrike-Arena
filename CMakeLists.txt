cmake_minimum_required(VERSION 3.20)
project(Platformer_RL)

set(CMAKE_CXX_STANDARD 20)

# find_package(raylib REQUIRED)
# Raylib
add_subdirectory(external/raylib)

## Build ImGui
add_library(imgui STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_demo.cpp
)

add_subdirectory(external/enet)

target_include_directories(imgui PUBLIC
        external/imgui
)

file(GLOB_RECURSE SOURCE_FILES src/*)
file(GLOB_RECURSE SERVER_FILES server/*)

set(ASSET_PATH "${CMAKE_SOURCE_DIR}/assets")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
add_executable(server ${SERVER_FILES})

target_link_libraries(server PRIVATE enet raylib)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib imgui enet)
target_compile_definitions(${PROJECT_NAME} PRIVATE ASSET_DIR="${ASSET_PATH}")
