project(Tests)

file(
    GLOB SOURCE_FILES
    *.cpp
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCE_FILES}
    )

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${cppext_INCLUDE_DIRS}
)

target_link_libraries(
    ${PROJECT_NAME}
    cppext
    gtest
    gmock_main
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/data
    ${PROJECT_BINARY_DIR}
    )

