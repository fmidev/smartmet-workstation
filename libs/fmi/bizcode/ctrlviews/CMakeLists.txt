cmake_minimum_required(VERSION 3.14.4)
project(CtrlViews)

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


set(TARGET_LINK_LIBRARIES_LIST
	${PROJECT_NAME}
    SmartTools
    Toolbox
    Newbase
    SmetData
    CtrlViewUtils
    TmUtils
    )
if(NOT UNIX)
    set(TARGET_LINK_LIBRARIES_LIST ${TARGET_LINK_LIBRARIES_LIST} WinRegistryUtils)
endif()
if(NOT DISABLE_CPPRESTSDK)
	set(TARGET_LINK_LIBRARIES_LIST ${TARGET_LINK_LIBRARIES_LIST} WmsSupport)
endif()

if(UNIX AND Qt6_FOUND)
    set(TARGET_LINK_LIBRARIES_LIST ${TARGET_LINK_LIBRARIES_LIST} Qt6::Widgets Qt6::Gui)
endif()

target_link_libraries(${TARGET_LINK_LIBRARIES_LIST})


target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${Toolbox_INCLUDE_DIRS}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${SmetData_INCLUDE_DIRS}
    PUBLIC ${CtrlViewUtils_INCLUDE_DIRS}
    PUBLIC ${StlHelp_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    PUBLIC ${TmUtils_INCLUDE_DIRS}
    PUBLIC ${WmsSupport_INCLUDE_DIRS}
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
    ${SmartTools_INCLUDE_DIRS}
    ${Toolbox_INCLUDE_DIRS}
    ${Newbase_INCLUDE_DIRS}
    ${SmetData_INCLUDE_DIRS}
    ${CtrlViewUtils_INCLUDE_DIRS}
    ${StlHelp_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    ${TmUtils_INCLUDE_DIRS}
    ${WmsSupport_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
if(NOT UNIX)
set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${${PROJECT_NAME}_INCLUDE_DIRS}
    ${WinRegistryUtils_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
endif()

