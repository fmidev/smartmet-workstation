cmake_minimum_required(VERSION 3.7.1)
project(Calculator)

set(CMAKE_MFC_FLAG 0)

file( 
    GLOB SOURCE_FILES 
    calculator/* 
    )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES} 
    )

target_link_libraries(
    ${PROJECT_NAME}
    Newbase
    MacGyver
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/calculator
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${MacGyver_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/calculator
    ${PROJECT_SOURCE_DIR}
    ${Newbase_INCLUDE_DIRS}
    ${MacGyver_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
