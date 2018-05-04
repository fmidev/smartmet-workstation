#
# Copyright(c) 2015 Ruslan Baratov.
# Distributed under the MIT License (http://opensource.org/licenses/MIT)
#

cmake_minimum_required(VERSION 3.1)
project(spdlog VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")
endif()

file( GLOB_RECURSE SOURCE_FILES include/*.cpp include/*.h )

add_library(spdlog ${SOURCE_FILES})

target_link_libraries(
    ${PROJECT_NAME}
    fmt
    )

target_include_directories( 
    ${PROJECT_NAME}
	PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${fmt_INCLUDE_DIRS}
    )

option(SPDLOG_BUILD_EXAMPLES "Build examples" OFF)
option(SPDLOG_BUILD_TESTING "Build spdlog tests" ON)

set(HEADER_BASE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(SPDLOG_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(SPDLOG_BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${fmt_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
