cmake_minimum_required(VERSION 3.14.4)

project(MfcUtils)

set(CMAKE_MFC_FLAG 2)

file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB RES_FILES SmartMetMfcUtils.rc res/*)

add_library(
    ${PROJECT_NAME} STATIC
    ${CPP_FILES}
    ${H_FILES}
    ${RES_FILES}
    )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${GridCtrl_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    )

set(${PROJECT_NAME}_RES_FILE
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dir/$(Configuration)/SmartMetMfcUtils.res
    CACHE INTERNAL "${PROJECT_NAME}: Generated .res file" FORCE
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${GridCtrl_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR} 
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )


