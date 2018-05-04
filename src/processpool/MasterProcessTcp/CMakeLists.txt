cmake_minimum_required(VERSION 3.6.2)

project(MasterProcessTcp)

set(CMAKE_MFC_FLAG 0)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

add_definitions( -D_UNICODE -DUNICODE )

file(GLOB CPP_FILES *.cpp)

include_directories(
    ${LIBS_FMI}/MultiProcessTools/include
    ${LIBS_FMI}/newbase/include
    ${BOOST_INCLUDE_DIR}
    )

link_directories(
    ${BOOST_LIB_DIR}
    )

add_executable(
    ${PROJECT_NAME}
    ${CPP_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    newbaseEditor
    MultiProcessTools
    )
