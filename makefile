CXX = g++

LINK_FLAGS = -fopenmp -lOpenCL
CXXFLAGS = -Wall -std=c++20 -g -O3 $(LINK_FLAGS)

all: rt

rt: main.cpp common.hpp objects.o ray.o point.o
	$(CXX) $(CXXFLAGS) -o rt main.cpp objects.o point.o ray.o

objects.o: objects.hpp common.hpp objects.cpp
	$(CXX) $(CXXFLAGS) -c -o objects.o objects.cpp

ray.o: ray.hpp ray.cpp common.hpp
	$(CXX) $(CXXFLAGS) -c -o ray.o ray.cpp

point.o: point.hpp common.hpp point.cpp
	$(CXX) $(CXXFLAGS) -c -o point.o point.cpp

clean:
	rm *.o
	rm *.bmp
	rm rt
