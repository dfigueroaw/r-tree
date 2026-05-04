cmake_minimum_required(VERSION 3.20)

project(RTreeProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(rtree INTERFACE)
target_include_directories(rtree INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(r_tree_app main.cpp)
target_link_libraries(r_tree_app PRIVATE rtree)

file(GLOB TEST_SOURCES tests/*.cpp)
add_executable(r_tree_tests ${TEST_SOURCES})
target_link_libraries(r_tree_tests PRIVATE rtree)

target_include_directories(r_tree_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rtree INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wconversion
        -Wsign-conversion

        $<$<CONFIG:Release>:-O3>

        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-fsanitize=address>
        $<$<CONFIG:Debug>:-fsanitize=undefined>
    )
endif()

enable_testing()
add_test(NAME RTreeTests COMMAND r_tree_tests)
