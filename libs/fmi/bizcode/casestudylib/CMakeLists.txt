cmake_minimum_required(VERSION 3.14.4)
project(CaseStudyLib)

if(NOT UNIX)
set(CMAKE_MFC_FLAG 2)
endif()

file( GLOB CPP_FILES source/*.cpp )
file( GLOB H_FILES include/*.h )

if(UNIX)
    list(FILTER CPP_FILES EXCLUDE REGEX "CShellFileOp\\.cpp$")
endif()

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
    JsonSpirit
    CtrlViewUtils
    )
if(NOT UNIX)
target_link_libraries(
    ${PROJECT_NAME}
    WinRegistryUtils
    )
endif()

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${EditingDataUtils_INCLUDE_DIRS}
    PUBLIC ${JsonSpirit_INCLUDE_DIRS}
    PUBLIC ${CtrlViewUtils_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    )
if(NOT UNIX)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${WinRegistryUtils_INCLUDE_DIRS}
    )
endif()

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${Newbase_INCLUDE_DIRS}
    ${SmartTools_INCLUDE_DIRS}
    ${EditingDataUtils_INCLUDE_DIRS}
    ${JsonSpirit_INCLUDE_DIRS}
    ${CtrlViewUtils_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )

