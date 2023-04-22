CC = gcc
CXX = g++

LINK_FLAGS =
CXXFLAGS = -Wall -std=c++20 -O3
CCFLAGS = -Wall -std=c17 -O3

all: asmt2

asmt2: main.cpp common.hpp objects.o ray.o point.o
	$(CXX) $(CXXFLAGS) -o asmt2 main.cpp objects.o point.o ray.o

objects.o: objects.hpp common.hpp objects.cpp
	$(CXX) $(CXXFLAGS) -c -o objects.o objects.cpp

ray.o: ray.hpp ray.cpp common.hpp
	$(CXX) $(CXXFLAGS) -c -o ray.o ray.cpp

point.o: point.hpp common.hpp point.cpp
	$(CXX) $(CXXFLAGS) -c -o point.o point.cpp

clean:
	rm *.o
	rm *.bmp
