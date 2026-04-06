cmake_minimum_required(VERSION 3.14.4)
project(Toolbox)

if(NOT UNIX)
set(CMAKE_MFC_FLAG 2)
endif()

file( GLOB CPP_FILES source/*.cpp )
file( GLOB H_FILES include/*.h )

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
if(NOT UNIX)
target_link_libraries(
    ${PROJECT_NAME}
    Win32Utils
    )
endif()

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${EditingDataUtils_INCLUDE_DIRS}
    PUBLIC ${CatLog_INCLUDE_DIRS}
    )
if(NOT UNIX)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${Win32Utils_INCLUDE_DIRS}
    )
endif()

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${Newbase_INCLUDE_DIRS}
    ${SmartTools_INCLUDE_DIRS}
    ${EditingDataUtils_INCLUDE_DIRS}
    ${CatLog_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
if(NOT UNIX)
set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${${PROJECT_NAME}_INCLUDE_DIRS}
    ${Win32Utils_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
endif()
