cmake_minimum_required(VERSION 3.1)
project(test_v1_package LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Ruby REQUIRED CONFIG)

add_executable(${PROJECT_NAME} ../test_package/test_package.cpp)
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
target_link_libraries(${PROJECT_NAME} PRIVATE Ruby::Ruby)
if (RUBY_STATIC_RUBY)
  target_compile_definitions(${PROJECT_NAME} PRIVATE RUBY_STATIC_RUBY)
endif()
if (RUBY_STATIC_LINKED_EXT)
  target_compile_definitions(${PROJECT_NAME} PRIVATE RUBY_STATIC_LINKED_EXT)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(TEST_RB_ABS_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin/test.rb")
configure_file(../test_package/test.rb ${TEST_RB_ABS_PATH} COPYONLY)
target_compile_definitions(${PROJECT_NAME} PRIVATE TEST_RB_ABS_PATH="${TEST_RB_ABS_PATH}")
