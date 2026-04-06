cmake_minimum_required(VERSION 3.14.4)
project(TmUtils)

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
	CtrlViews
    SmartTools
    Newbase
	CatLog
    )

if(UNIX)
target_link_libraries(${PROJECT_NAME}
    ${TRAX_LIB}
    ${GIZA_LIB}
    ${IMAGINE2_LIB}
    ${GIS_LIB}
    )
endif()

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${CtrlViews_INCLUDE_DIRS}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${StlHelp_INCLUDE_DIRS}
    PUBLIC ${UNIRAS_INCLUDE_DIR}
    PUBLIC ${BOOST_INCLUDE_DIR}
	PUBLIC ${CatLog_INCLUDE_DIRS}
    )

if(UNIX)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${TRAX_INCLUDE_DIR}
    PUBLIC ${GIZA_INCLUDE_DIR}
    PUBLIC ${IMAGINE2_INCLUDE_DIR}
    PUBLIC ${GIS_INCLUDE_DIR}
    )
endif()

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${CtrlViews_INCLUDE_DIRS}
    ${SmartTools_INCLUDE_DIRS}
    ${Newbase_INCLUDE_DIRS}
    ${StlHelp_INCLUDE_DIRS}
    ${UNIRAS_INCLUDE_DIR}
    ${BOOST_INCLUDE_DIR}
	${CatLog_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
