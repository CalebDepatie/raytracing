CXX = g++

LINK_FLAGS = -fopenmp -lOpenCL
CXXFLAGS = -Wall -std=c++20 -g -O3 $(LINK_FLAGS)

all: rt

rt: main.cpp common.hpp objects.o ray.o point.o trace.o
	$(CXX) $(CXXFLAGS) -o rt main.cpp objects.o point.o ray.o trace.o

objects.o: Structures/objects.hpp common.hpp Structures/objects.cpp
	$(CXX) $(CXXFLAGS) -c -o objects.o Structures/objects.cpp

trace.o: trace.hpp trace.cpp Structures/ray.hpp Structures/objects.hpp common.hpp
	$(CXX) $(CXXFLAGS) -c -o trace.o trace.cpp

ray.o: Structures/ray.hpp Structures/ray.cpp common.hpp
	$(CXX) $(CXXFLAGS) -c -o ray.o Structures/ray.cpp

point.o: Structures/point.hpp common.hpp Structures/point.cpp
	$(CXX) $(CXXFLAGS) -c -o point.o Structures/point.cpp

clean:
	rm *.o
	rm *.bmp
	rm rt
