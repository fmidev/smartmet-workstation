cmake_minimum_required(VERSION 3.6.2)

project(WorkerProcessMFC)

set(CMAKE_MFC_FLAG 2)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

add_definitions( -D_UNICODE -DUNICODE )

file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB RES_FILES *.rc res/* )

include_directories(
    ${LIBS_FMI}/MultiProcessTools/include
    ${LIBS_FMI}/newbase/include
    ${BOOST_INCLUDE_DIR}
    ${UNIRAS_INCLUDE_DIR}
    ${SRC}/processpool/MultiProcessToolsMFC
    )

link_directories(
    ${BOOST_LIB_DIR}
    ${UNIRAS_LIB_DIR}
    )

add_executable(
    ${PROJECT_NAME}
    ${CPP_FILES}
    ${H_FILES}
    ${RES_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    newbaseEditor
    MultiProcessTools
    MultiProcessToolsMFC
    )
