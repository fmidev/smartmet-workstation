cmake_minimum_required(VERSION 3.7.1)
project(SmartTools)

set(CMAKE_MFC_FLAG 2)

add_definitions(
    -DFMI_SUPPORT_STATION_DATA_SMARTTOOL
    )

file( GLOB SOURCE_FILES smarttools/* )

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
    PUBLIC ${PROJECT_SOURCE_DIR}/smarttools
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/smarttools
    ${PROJECT_SOURCE_DIR}
    ${Newbase_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
