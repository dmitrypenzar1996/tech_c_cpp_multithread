C=gcc
CXX=g++
CCFLAGS=
CXXFLAGS=--std=c++11
all: extsort check

clean:
	rm -f external_sort.exe

extsort:
	$(CXX) $(CXXFLAGS) external_sort.cpp -o external_sort.exe

check:
	python check.py
