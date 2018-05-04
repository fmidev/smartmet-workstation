cmake_minimum_required(VERSION 3.7.1)
project(WebClient)

file(
    GLOB SOURCE_FILES
    webclient/*
    )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES}
    )

target_link_libraries( 
    ${PROJECT_NAME}
    cppback
    debug ${CPPRESTSDK_DEBUG_LIB}
    optimized ${CPPRESTSDK_OPTIMIZED_LIB}
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${cppback_INCLUDE_DIRS}
    PUBLIC ${CPPRESTSDK_INCLUDE_DIR}
    )

source_group( 
    "Src" 
    FILES ${SOURCE_FILES} 
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${cppback_INCLUDE_DIRS}
    ${CPPRESTSDK_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
