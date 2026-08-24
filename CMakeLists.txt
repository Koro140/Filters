cmake_minimum_required(VERSION 3.20)

project(filters C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_target(run_python_script
    COMMAND ${Python3_EXECUTABLE}
            "${CMAKE_SOURCE_DIR}/resources/scripts/get_shaders_to_file.py"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/scripts/"
)

add_library(glad vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

find_package(SDL3 REQUIRED)
add_subdirectory(vendor/cglm)
find_package(PkgConfig REQUIRED)

pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(SWSCALE REQUIRED IMPORTED_TARGET libswscale)
pkg_check_modules(SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)

add_executable(filters
    src/main.c
    src/shader.c
    src/player.c
    src/frame_queue.c
    src/texture.c
    src/video_renderer.c
    src/audio_renderer.c
    src/settings.c
    src/filters.c
)
add_dependencies(filters run_python_script)

target_include_directories(filters PRIVATE
    include
    vendor/flag
)

target_link_libraries(filters PRIVATE
    cglm
    SDL3::SDL3
    glad
    PkgConfig::AVFORMAT
    PkgConfig::AVCODEC
    PkgConfig::AVUTIL
    PkgConfig::SWSCALE
    PkgConfig::SWRESAMPLE
)