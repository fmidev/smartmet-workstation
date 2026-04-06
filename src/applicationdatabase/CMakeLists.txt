cmake_minimum_required(VERSION 3.14.4)

project(ApplicationDataBase)

if(NOT UNIX)
set(CMAKE_MFC_FLAG 2)
endif()

file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)

add_library(
    ${PROJECT_NAME} STATIC
    ${CPP_FILES}
    ${H_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    Newbase
    SmartTools
    Q2ClientLib
    CtrlViewUtils
    )
if(NOT UNIX)
target_link_libraries(
    ${PROJECT_NAME}
    MfcUtils
    )
endif()

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${Q2ClientLib_INCLUDE_DIRS}
    PUBLIC ${CtrlViewUtils_INCLUDE_DIRS}
    )
if(NOT UNIX)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${MfcUtils_INCLUDE_DIRS}
    )
endif()

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${Newbase_INCLUDE_DIRS}
    ${SmartTools_INCLUDE_DIRS}
    ${Q2ClientLib_INCLUDE_DIRS}
    ${CtrlViewUtils_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
