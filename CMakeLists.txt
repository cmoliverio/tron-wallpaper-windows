cmake_minimum_required(VERSION 3.16)
project(OpenGLApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
endif()

# GLFW
find_package(glfw3 CONFIG REQUIRED)

# GLAD
find_package(glad CONFIG REQUIRED)

# GLM
find_package(glm CONFIG REQUIRED)

add_executable(tron_wallpaper
    src/main.cpp
    src/shader.cpp
    src/tetrahedron.cpp
    src/wallpaper_utils.cpp
)

target_include_directories(tron_wallpaper
    PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(tron_wallpaper
    PRIVATE
        glad::glad
        glfw
        glm::glm
        opengl32
)

# MSVC niceties
if (MSVC)
    target_compile_options(tron_wallpaper PRIVATE /W4 /permissive-)
endif()

set_target_properties(tron_wallpaper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}"
)