cmake_minimum_required(VERSION 3.14.4)

project(cppback)

file(
    GLOB SOURCE_FILES
    *.h
    *.cpp
    )

add_library(
    ${PROJECT_NAME}
    ${SOURCE_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    CatLog
    )

target_include_directories(
	${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/..
    PUBLIC ${CatLog_INCLUDE_DIRS}
)

source_group("Src" FILES ${SOURCE_FILES})

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/..
    ${CatLog_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )

