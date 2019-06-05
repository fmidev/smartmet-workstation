cmake_minimum_required(VERSION 3.14.4)
project(WorkerProcessTcpMFC)

set(CMAKE_MFC_FLAG 2)

link_directories(
    ${BOOST_LIB_DIR}
    )

file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB RES_FILES *.rc res/* )

add_executable(
    ${PROJECT_NAME} WIN32
    ${CPP_FILES}
    ${H_FILES}
    ${RES_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    Newbase
    MultiProcessTools
    MultiProcessToolsMFC
	${UNIRAS_LIB}
	version
    )
	
include_directories(
    PUBLIC ${BOOST_INCLUDE_DIR}
    PUBLIC ${UNIRAS_INCLUDE_DIR}
    PUBLIC ${Newbase_INCLUDE_DIRS}
    PUBLIC ${MultiProcessTools_INCLUDE_DIRS}
    PUBLIC ${MultiProcessToolsMFC_INCLUDE_DIRS}
    )

# POST
# xcopy /y $(OutDir)$(ProjectName).exe $(BUILD_DIR)\$(Platform)\SmartMet\SmartMet\$(Configuration)\
