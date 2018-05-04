cmake_minimum_required(VERSION 3.6.2)

project(ApplicationDataBase)

set(CMAKE_MFC_FLAG 2)

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
    MfcUtils
    Q2ClientLib
    CtrlViewUtils
    )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${MfcuUtils_INCLUDE_DIRS}
    PUBLIC ${Q2ClientLib_INCLUDE_DIRS}
    PUBLIC ${CtrlViewUtils_INCLUDE_DIRS}
    PUBLIC ${UNIRAS_INCLUDE_DIR}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${Newbase_INCLUDE_DIRS}
    ${SmartTools_INCLUDE_DIRS}
    ${MfcuUtils_INCLUDE_DIRS}
    ${Q2ClientLib_INCLUDE_DIRS}
    ${CtrlViewUtils_INCLUDE_DIRS}
    ${UNIRAS_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
