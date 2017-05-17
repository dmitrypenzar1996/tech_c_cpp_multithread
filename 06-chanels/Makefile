TEST_FILES = ../thirdparty/gtest/gtest-all.cc ../thirdparty/gtest/gtest_main.cc
HDR = include/coroutine/engine.h
SRC = src/engine.cpp main.cpp
all: chanel

chanel: $(SRC) $(HDR)
	g++ -O1 -g -std=c++11 -o chanel src/engine.cpp main.cpp -lpthread -Iinclude

ping: $(SRC) $(HDR)
	g++ -O1 -g -std=c++11 -o ping src/engine.cpp ping.cpp -lpthread -Iinclude
