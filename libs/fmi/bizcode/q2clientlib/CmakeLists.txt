cmake_minimum_required(VERSION 3.14.4)
project(Q2ClientLib)

if(NOT UNIX)
set(CMAKE_MFC_FLAG 0)
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
	CatLog
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${Newbase_INCLUDE_DIRS}
	PUBLIC ${CatLog_INCLUDE_DIRS}
    PUBLIC ${BOOST_INCLUDE_DIR}
    PUBLIC ${OPENSSL_INCLUDE_DIR}
    )

set(
    ${PROJECT_NAME}_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${Newbase_INCLUDE_DIRS}
	${CatLog_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
    )
