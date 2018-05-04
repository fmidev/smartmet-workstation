cmake_minimum_required(VERSION 3.6.2)

project(AtlUtils)

set(CMAKE_MFC_FLAG 2)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(
    -D_USRDLL
    -DSMARTMETATLUTILS_BUILD
    )

file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)

file(GLOB RES_FILES *.rc *.rgs res/*)
file(GLOB IDL_FILES *.idl)

add_library(
    ${PROJECT_NAME} SHARED
    ${CPP_FILES}
    ${H_FILES}
    ${RES_FILES}
    ${IDL_FILES}
    )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:windows")
endif()

set(${PROJECT_NAME}_RES_FILE
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dir/$(Configuration)/SmartMetAtlUtils.res
    CACHE INTERNAL "${PROJECT_NAME}: Generated .res file" FORCE
    )
