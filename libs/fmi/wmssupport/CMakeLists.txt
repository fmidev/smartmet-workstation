cmake_minimum_required(VERSION 3.14.4)
project(WmsSupport)

set(CMAKE_MFC_FLAG 2)

add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/wd4503>
    )

file(
    GLOB SOURCE_FILES
    wmssupport/*
    )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES}
    )

target_link_libraries( 
    ${PROJECT_NAME}
    BitmapHandler
    Newbase
    WebClient
    cppext
    cppback
    CatLog
	XmlLiteUtils
	CtrlViewUtils
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}
    PUBLIC ${PROJECT_SOURCE_DIR}/wmssupport
    PUBLIC ${BitmapHandler_INCLUDE_DIRS}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${WebClient_INCLUDE_DIRS}
    PUBLIC ${cppext_INCLUDE_DIRS}
    PUBLIC ${cppback_INCLUDE_DIRS}
    PUBLIC ${CatLog_INCLUDE_DIRS}
	PUBLIC ${XmlLiteUtils_INCLUDE_DIRS}
	PUBLIC ${CtrlViewUtils_INCLUDE_DIRS}
    )

source_group( 
    "Src" 
    FILES ${SOURCE_FILES} 
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/wmssupport
    ${BitmapHandler_INCLUDE_DIRS}
    ${Newbase_INCLUDE_DIRS}
    ${WebClient_INCLUDE_DIRS}
    ${cppext_INCLUDE_DIRS}
    ${cppback_INCLUDE_DIRS}
    ${CatLog_INCLUDE_DIRS}
	${XmlLiteUtils_INCLUDE_DIRS}
	${CtrlViewUtils_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
