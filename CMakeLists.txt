cmake_minimum_required(VERSION 3.20)
project(NetStrike-Arena)

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
        external/enet/include
        external/json
)

file(GLOB_RECURSE SOURCE_FILES src/*)
file(GLOB_RECURSE SERVER_FILES server/*)
file(GLOB_RECURSE COMMON_FILES common/*)

set(ASSET_PATH "${CMAKE_SOURCE_DIR}/assets")

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${COMMON_FILES})
add_executable(server ${SERVER_FILES} ${COMMON_FILES})

# Server required libraries
target_link_libraries(server PRIVATE enet raylib)
# Game required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE raylib imgui enet)

target_compile_definitions(${PROJECT_NAME} PRIVATE ASSET_DIR="${ASSET_PATH}")

# Fix enet include for server
target_include_directories(server PRIVATE external/enet/include)

# Fix nlohmann json include
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/nlohmann)
# Copy json.hpp to build/include/nlohmann/json.hpp so <nlohmann/json.hpp> works
configure_file(${CMAKE_SOURCE_DIR}/external/json/json.hpp ${CMAKE_BINARY_DIR}/include/nlohmann/json.hpp COPYONLY)
include_directories(${CMAKE_BINARY_DIR}/include)
