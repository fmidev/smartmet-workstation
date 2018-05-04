cmake_minimum_required(VERSION 3.7.1)
project(Imagine)

set(CMAKE_MFC_FLAG 0)

add_definitions( 
    -DIMAGINE_IGNORE_FORMATS
    )

file( GLOB SOURCE_FILES imagine/*.cpp imagine/*.h )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES} 
    )

target_link_libraries(
    ${PROJECT_NAME}
    Newbase
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/imagine
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/imagine
    ${Newbase_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
