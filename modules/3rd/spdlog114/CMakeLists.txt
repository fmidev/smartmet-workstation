cmake_minimum_required(VERSION 3.10)
project(spdlog114 VERSION 1.14.1)

# Header-only interface library for spdlog 1.14.1 using system fmt
add_library(spdlog INTERFACE)
add_library(spdlog::spdlog ALIAS spdlog)

target_include_directories(spdlog INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Use system fmt (fmt-12 on Linux)
if(UNIX)
    find_package(fmt REQUIRED)
    target_link_libraries(spdlog INTERFACE fmt::fmt)
    target_compile_definitions(spdlog INTERFACE SPDLOG_FMT_EXTERNAL)
endif()

find_package(Threads REQUIRED)
target_link_libraries(spdlog INTERFACE Threads::Threads)

# Expose include dirs via cache variable (matches pattern used by other modules)
set(
    spdlog_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    CACHE INTERNAL "spdlog: Include Directories" FORCE
)
