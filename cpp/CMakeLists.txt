cmake_minimum_required(VERSION 3.16)
project(lab3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LAB3_COVERAGE "Build with coverage instrumentation" OFF)
option(LAB3_TESTS "Build unit tests" ON)
option(LAB3_BENCH "Build benchmarks" ON)

if(LAB3_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping -O0 -g)
        add_link_options(-fprofile-instr-generate)
    else()
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
endif()

add_library(lab3_core STATIC
    src/codec.cpp src/serializable.cpp src/sequence.cpp
    src/array.cpp src/singly_list.cpp src/doubly_list.cpp src/stack.cpp src/queue.cpp
    src/rb_tree.cpp src/hash_table.cpp)
target_include_directories(lab3_core PUBLIC include)
target_compile_options(lab3_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(lab3_demo main.cpp)
target_link_libraries(lab3_demo lab3_core)

if(LAB3_TESTS)
    enable_testing()
    find_package(Boost REQUIRED COMPONENTS unit_test_framework)
    add_executable(tests_boost tests/boost_tests.cpp)
    target_link_libraries(tests_boost lab3_core Boost::unit_test_framework)
    target_compile_definitions(tests_boost PRIVATE BOOST_TEST_DYN_LINK)
    add_test(NAME boost COMMAND tests_boost)

    find_package(GTest REQUIRED)
    add_executable(tests_gtest tests/gtest_tests.cpp)
    target_link_libraries(tests_gtest lab3_core GTest::gtest_main)
    add_test(NAME gtest COMMAND tests_gtest)

    find_package(Catch2 3 REQUIRED)
    add_executable(tests_catch2 tests/catch2_tests.cpp)
    target_link_libraries(tests_catch2 lab3_core Catch2::Catch2WithMain)
    add_test(NAME catch2 COMMAND tests_catch2)
endif()

if(LAB3_BENCH)
    find_package(benchmark REQUIRED)
    add_executable(bench benchmarks.cpp)
    target_link_libraries(bench lab3_core benchmark::benchmark)
endif()
