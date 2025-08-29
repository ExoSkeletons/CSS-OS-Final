CXX := g++
CXXFLAGS := -std=c++20 -Wall -fPIC

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
LIBS := libgraph.so
TARGETS := graph_demo.exe

all: $(LIBS) $(TARGETS)

$(LIBS): $(OBJECTS)
	$(CXX) -shared -o $@ $^
	rm -f $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(LIBS)

.PHONY: all clean