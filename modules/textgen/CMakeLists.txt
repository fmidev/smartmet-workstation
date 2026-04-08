cmake_minimum_required(VERSION 3.14.4)
project(Textgen)

set(CMAKE_MFC_FLAG 0)

# Set C++ standard to C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

disable_linux_side_compiler_warnings()

file( 
    GLOB SOURCE_FILES 
    textgen/* 
    )

list(
    REMOVE_ITEM SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/textgen/MySQLDictionary.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/textgen/MySQLDictionary.cpp"
    )

if(UNIX)
    # PostGISDataSource uses deprecated OGR API (pre-GDAL 2.0) that is
    # incompatible with current GDAL. Exclude it on Linux.
    # The authoritative textgen is in a separate repository (~/hub/textgen).
    list(FILTER SOURCE_FILES EXCLUDE REGEX "PostGISDataSource\\.(cpp|h)$")
    message(STATUS "Textgen: excluding PostGISDataSource (uses deprecated OGR API)")
endif()

add_library(
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    MacGyver
    Newbase
    Calculator
    )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/textgen
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${MacGyver_INCLUDE_DIRS}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${Calculator_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/textgen
    ${PROJECT_SOURCE_DIR}
    ${MacGyver_INCLUDE_DIRS}
    ${Newbase_INCLUDE_DIRS}
    ${Calculator_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
