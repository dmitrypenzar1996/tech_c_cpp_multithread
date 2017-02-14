cmake_minimum_required(VERSION 3.6)
project(technosphere_c_multithread)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        01-allocator/allocator.cpp
        01-allocator/allocator.h
        01-allocator/allocator_error.cpp
        01-allocator/allocator_error.h
        01-allocator/allocator_pointer.cpp
        01-allocator/allocator_pointer.h
        01-allocator/allocator_test.cpp
        01-allocator/Makefile
        thirdparty/gtest/gtest-all.cc
        thirdparty/gtest/gtest.h
        thirdparty/gtest/gtest_main.cc 01-allocator/mem_list_elem.cpp 01-allocator/mem_list_elem.h)

add_executable(technosphere_c_multithread ${SOURCE_FILES})