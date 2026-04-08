cmake_minimum_required(VERSION 3.14.4)
project(ModifyEditedData)

if(NOT UNIX)
set(CMAKE_MFC_FLAG 0)
endif()

add_definitions( 
    -DFMI_SUPPORT_STATION_DATA_SMARTTOOL 
    -D_SCL_SECURE_NO_WARNINGS
    )

file( GLOB CPP_FILES source/*.cpp )
file( GLOB H_FILES include/*.h )

if(UNIX)
    list(FILTER CPP_FILES EXCLUDE REGEX "MultiProcessClientData\\.cpp$")
    list(FILTER CPP_FILES EXCLUDE REGEX "NFmiMultiProcessPoolOptions\\.cpp$")
    list(FILTER CPP_FILES EXCLUDE REGEX "NFmiDataParamModifier\\.cpp$")
    list(FILTER CPP_FILES EXCLUDE REGEX "TimeSerialModification\\.cpp$")
endif()

add_library(
    ${PROJECT_NAME} STATIC
    ${CPP_FILES}
    ${H_FILES}
    )

if(NOT UNIX)
    set(MULTIPROCESSTOOLS_LIBS MultiProcessTools)
    set(MULTIPROCESSTOOLS_INCLUDES ${MultiProcessTools_INCLUDE_DIRS})
else()
    set(MULTIPROCESSTOOLS_LIBS "")
    set(MULTIPROCESSTOOLS_INCLUDES "")
endif()

target_link_libraries(
    ${PROJECT_NAME}
    Newbase
    SmartTools
    ${MULTIPROCESSTOOLS_LIBS}
    EditingDataUtils
    CtrlViewUtils
	CatLog
    TmUtils
    )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${MULTIPROCESSTOOLS_INCLUDES}
    PUBLIC ${EditingDataUtils_INCLUDE_DIRS}
    PUBLIC ${CtrlViewUtils_INCLUDE_DIRS}
	PUBLIC ${CatLog_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    PUBLIC ${TmUtils_INCLUDE_DIRS}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${Newbase_INCLUDE_DIRS}
    ${SmartTools_INCLUDE_DIRS}
    ${MULTIPROCESSTOOLS_INCLUDES}
    ${EditingDataUtils_INCLUDE_DIRS}
    ${CtrlViewUtils_INCLUDE_DIRS}
	${CatLog_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    ${TmUtils_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
