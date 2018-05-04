cmake_minimum_required(VERSION 3.7.1)
project(HakeMessage)

set(CMAKE_MFC_FLAG 2)

file( GLOB SOURCE_FILES *.cpp *.h )

add_library( 
    ${PROJECT_NAME} STATIC
    ${SOURCE_FILES} 
    )

source_group("Src" FILES ${SOURCE_FILES})

target_link_libraries(
    ${PROJECT_NAME}
    XmlLiteUtils
    Newbase
    SmartTools
    EditingDataUtils
    WebClient
    cppext
    cppback
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/..
    PUBLIC ${XmlLiteUtils_INCLUDE_DIRS}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${SmartTools_INCLUDE_DIRS}
    PUBLIC ${EditingDataUtils_INCLUDE_DIRS}
    PUBLIC ${WebClient_INCLUDE_DIRS}
    PUBLIC ${cppext_INCLUDE_DIRS}
    PUBLIC ${cppback_INCLUDE_DIRS}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/..
    ${XmlLiteUtils_INCLUDE_DIRS}
    ${Newbase_INCLUDE_DIRS}
    ${SmartTools_INCLUDE_DIRS}
    ${EditingDataUtils_INCLUDE_DIRS}
    ${WebClient_INCLUDE_DIRS}
    ${cppext_INCLUDE_DIRS}
    ${cppback_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )

