cmake_minimum_required(VERSION 3.14.4)
project(Processes)

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
    EditingDataUtils
    CatLog
    )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${BOOST_INCLUDE_DIR}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
	PUBLIC ${CatLog_INCLUDE_DIRS}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${Newbase_INCLUDE_DIRS}
    ${SmartTools_INCLUDE_DIRS}
	${CatLog_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )

