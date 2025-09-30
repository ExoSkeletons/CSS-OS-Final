CXX := g++
CXXFLAGS ?= -std=c++20 -Wall -fPIC

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

LIB := libgraph.so
TARGET := graph_demo.exe

all: $(LIB) $(TARGET)

$(LIB): $(OBJECTS)
	$(CXX) -shared -o $@ $^
	rm -f $(OBJECTS)

$(TARGET): graph_demo.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< -L. -lgraph

# Compile normal objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so *.gcno $(TARGET)

.PHONY: all clean
