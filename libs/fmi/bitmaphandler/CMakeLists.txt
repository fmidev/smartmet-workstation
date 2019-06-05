cmake_minimum_required(VERSION 3.14.4)
project(BitmapHandler)

set(CMAKE_MFC_FLAG 2)

file(
    GLOB SOURCE_FILES
    bitmaphandler/*
    )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES}
    )

target_link_libraries( 
    ${PROJECT_NAME}
    CtrlViews
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${CtrlViews_INCLUDE_DIRS}
    )

source_group( 
    "Src" 
    FILES ${SOURCE_FILES} 
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${CtrlViews_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
