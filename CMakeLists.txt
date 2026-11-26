cmake_minimum_required(VERSION 3.28)
project(HomeWork1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(HomeWork1 main.cpp
        UniqPtr.h
        ShrdPtr.h
        Benchmark.h)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(SmartPtrTests tests.cpp)
target_link_libraries(SmartPtrTests GTest::gtest_main)