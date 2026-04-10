cmake_minimum_required(VERSION 3.14.4)
project(WebClient)

if(UNIX)
    # Linux: use libcurl instead of cpprestsdk
    set(SOURCE_FILES
        webclient/Client.h
        webclient/CurlClient.h
        webclient/CurlClient.cpp
    )

    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CURL REQUIRED libcurl)

    target_link_libraries(${PROJECT_NAME}
        cppback
        CatLog
        ${CURL_LIBRARIES}
    )

    target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}
        PUBLIC ${cppback_INCLUDE_DIRS}
        PUBLIC ${CatLog_INCLUDE_DIRS}
        SYSTEM PUBLIC ${CURL_INCLUDE_DIRS}
    )
else()
    # Windows: use cpprestsdk
    file(GLOB SOURCE_FILES webclient/*)

    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME}
        cppback
        debug ${CPPRESTSDK_DEBUG_LIB}
        optimized ${CPPRESTSDK_OPTIMIZED_LIB}
    )

    target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}
        PUBLIC ${cppback_INCLUDE_DIRS}
        PUBLIC ${CPPRESTSDK_INCLUDE_DIR}
    )
endif()

source_group("Src" FILES ${SOURCE_FILES})

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}
    ${cppback_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
)
