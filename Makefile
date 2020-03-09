CXX = gcc
CPP = g++
CXXFLAGS = -Wall -Werror -Wextra -g
CPPFLAGS = -std=c++11

all: multi-lookup

multi-lookup: util.o multi-lookup.o FileHandler.o
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) -o multi-lookup multi-lookup.o util.o FileHandler.o -lpthread

multi-lookup.o: multi-lookup.cpp
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) -c multi-lookup.cpp

util.o: util.c
	$(CXX) $(CXXFLAGS) -c util.c

FileHandler.o: FileHandler.cpp
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) -c FileHandler.cpp

clean:
	rm *.o multi-lookup