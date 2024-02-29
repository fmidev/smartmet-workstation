cmake_minimum_required(VERSION 3.14.4)
project(Newbase)

if(WIN32)
    set(CMAKE_MFC_FLAG 0)
endif()

# Set C++ standard to C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file( GLOB SOURCE_FILES newbase/* )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES} 
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/newbase
    PUBLIC ${BOOST_INCLUDE_DIR} 
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/newbase
    ${PROJECT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR} 
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )


