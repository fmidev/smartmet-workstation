cmake_minimum_required(VERSION 3.7.1)
project(CaseStudyExe)

set(CMAKE_MFC_FLAG 2)

link_directories(
    ${BOOST_LIB_DIR}
    ${TOOLKITPRO_LIB_DIR}
    )

file( GLOB CPP_FILES *.cpp )
file( GLOB H_FILES *.h )
file( GLOB RES_FILES *.rc res/* )

add_executable( 
    ${PROJECT_NAME} WIN32
    ${CPP_FILES} 
    ${H_FILES} 
    ${RES_FILES} 
    )

target_link_libraries(
    ${PROJECT_NAME}
    Newbase
    MfcUtils
    CaseStudyLib
    )

target_include_directories( 
    ${PROJECT_NAME}
    PUBLIC ${BOOST_INCLUDE_DIR}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${MfcUtils_INCLUDE_DIRS}
    PUBLIC ${CaseStudyLib_INCLUDE_DIRS}
    )

