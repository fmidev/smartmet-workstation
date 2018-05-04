cmake_minimum_required(VERSION 3.7.1)
project(Textgen)

set(CMAKE_MFC_FLAG 0)

file( 
    GLOB SOURCE_FILES 
    textgen/* 
    )

list(
    REMOVE_ITEM SOURCE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/textgen/MySQLDictionary.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/textgen/MySQLDictionary.cpp"
    )

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
