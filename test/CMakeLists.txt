cmake_minimum_required(VERSION 3.14.4)

project(Tests)

set(CMAKE_MFC_FLAG 2)

link_directories(
    ${BOOST_LIB_DIR}
    ${CPPRESTSDK_LIB_DIR}
    )

file(
    GLOB DATAUTILITIES_FILES
    datautilities/*.cpp
    datautilities/*.h
    )

file(
    GLOB WMSSUPPORT_FILES
    wmssupport/*.cpp
    wmssupport/*.h
    )

add_executable(
    ${PROJECT_NAME}
    ${DATAUTILITIES_FILES}
    ${WMSSUPPORT_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    gtest
    gmock_main
    DataUtilities
    WmsSupport
    debug cpprest140d_2_9
    optimized cpprest140_2_9
    )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${DataUtilities_INCLUDE_DIRS}
    PUBLIC ${WmsSupport_INCLUDE_DIRS}
    )

source_group("DataUtilitiesTest" FILES ${DATAUTILITIES_FILES})
source_group("WmsSupportTest" FILES ${WMSSUPPORT_FILES})

