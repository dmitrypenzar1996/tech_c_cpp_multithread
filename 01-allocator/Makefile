TEST_FILES = ../thirdparty/gtest/gtest-all.cc ../thirdparty/gtest/gtest_main.cc
SRC = allocator.cpp allocator_error.cpp allocator_pointer.cpp allocator_test.cpp mem_list_elem.cpp
HDR = allocator.h allocator_error.h allocator_pointer.h	mem_list_elem.cpp


all: tests.done

allocator_test: $(SRC) $(HDR)
	g++ -O1 -g -std=c++11 -o allocator_test $(SRC) -I../thirdparty $(TEST_FILES) -lpthread

tests.done: allocator_test
	./allocator_test
	touch tests.done
