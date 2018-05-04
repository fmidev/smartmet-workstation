cmake_minimum_required(VERSION 3.7.1)
project(DataUtilities)

set(CMAKE_MFC_FLAG 0)

add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/wd4503>
    )

file(
    GLOB ENTRY_FILES
    datautilities/*
    )

file(
    GLOB CACHE_FILES
    source/cache/*
    )

file(
    GLOB COMMON_FILES
    source/common/*
    )

file(
    GLOB DATAUTIL_FILES
    source/dataUtil/*
    )

file(
    GLOB NEWBASEUTIL_FILES
    source/newbaseUtil/*
    )

file(
    GLOB INTERPOLATION_FILES
    source/interpolation/*
    )

file(
    GLOB INTERPOLATEDDATA_FILES
    source/interpolatedData/*
    )

file(
    GLOB ERROR_FILES
    source/error/*
    )

file(
    GLOB ROOTDATA_FILES
    source/rootData/*
    )

add_library( 
    ${PROJECT_NAME} STATIC
    ${ENTRY_FILES} 
    ${ERROR_FILES}
    ${ROOTDATA_FILES}
    ${INTERPOLATEDDATA_FILES}
    ${INTERPOLATION_FILES}
    ${NEWBASEUTIL_FILES}
    ${DATAUTIL_FILES}
    ${COMMON_FILES}
    ${CACHE_FILES}
    )

target_link_libraries( 
    ${PROJECT_NAME}
    Newbase
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${PROJECT_SOURCE_DIR}
    )

source_group( 
    "Cache" 
    FILES ${CACHE_FILES}
    )

source_group( 
    "Common" 
    FILES ${COMMON_FILES}
    )

source_group( 
    "DataUtil" 
    FILES ${DATAUTIL_FILES}
    )

source_group( 
    "NewbaseUtil" 
    FILES ${NEWBASEUTIL_FILES}
    )

source_group( 
    "Interpolation" 
    FILES ${INTERPOLATION_FILES}
    )

source_group( 
    "InterpolatedData" 
    FILES ${INTERPOLATEDDATA_FILES}
    )

source_group( 
    "RootData" 
    FILES ${ROOTDATA_FILES}
    )

source_group( 
    "Error" 
    FILES ${ERROR_FILES}
    )

source_group( 
    "Entry" 
    FILES ${ENTRY_FILES} 
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${Newbase_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
