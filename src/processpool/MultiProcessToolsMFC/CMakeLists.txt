cmake_minimum_required(VERSION 3.6.2)

project(MultiProcessToolsMFC)

set(CMAKE_MFC_FLAG 2)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

add_definitions( -D_UNICODE -DUNICODE )

file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)

include_directories(
    ${LIBS_FMI}/MultiProcessTools/include
    ${LIBS_FMI}/newbase/include
    ${BOOST_INCLUDE_DIR}
    ${UNIRAS_INCLUDE_DIR}
    )

add_library(
    ${PROJECT_NAME} STATIC
    ${CPP_FILES}
    ${H_FILES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    newbaseEditor
    MultiProcessTools
    )
