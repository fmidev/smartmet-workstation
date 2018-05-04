cmake_minimum_required(VERSION 3.7.1)
project(CatLog)

set(CMAKE_MFC_FLAG 0)

file( GLOB SOURCE_FILES *.cpp *.h )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES} 
    )

source_group("Src" FILES ${SOURCE_FILES})

target_link_libraries(
    ${PROJECT_NAME}
    spdlog
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/..
    PUBLIC ${spdlog_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/..
    ${spdlog_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )

