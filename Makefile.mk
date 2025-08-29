# ---- Compiler setup ----
CXX      := g++
CXXFLAGS := -std=c++20 -Wall -fPIC
COVERAGE_FLAGS := -fprofile-arcs -ftest-coverage

# ---- Linker setup ----
# linker search paths + runtime search paths
LDFLAGS  := -L. -Lgraph \
            -Wl,-rpath,'$$ORIGIN' \
            -Wl,-rpath,'$$ORIGIN/graph'

# libraries to link against
LDLIBS   := -lpthread

# ---- Project structure ----
LIBS     := libfd_polling.so libpthread_patterns.so graph/libgraph.so
EXES     := pthp_demo graph_demo server client

# ---- Default build ----
all: $(LIBS) $(EXES)

# ---- Libraries ----
libfd_polling.so: fd_polling.cpp
	$(CXX) $(CXXFLAGS) -shared -o $@ $<

libpthread_patterns.so: pthread_patterns.cpp
	$(CXX) $(CXXFLAGS) -shared -o $@ $<

graph/libgraph.so:
	$(MAKE) -C graph

# ---- Executables ----
pthp_demo: pthp_demo.cpp $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) -lgraph -lpthread_patterns -lfd_polling

graph_demo: graph/graph_demo.cpp graph/libgraph.so
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) -lgraph

server: server.cpp $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) -lgraph -lpthread_patterns -lfd_polling

client: client.cpp $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS) -lpthread_patterns -lfd_polling

# ---- Analysis / debugging ----
valgrind: server
	valgrind ./server

helgrind: server
	valgrind --tool=helgrind ./server

callgrind: server
	valgrind --tool=callgrind ./server


# ---- Coverage ----
server_cov:
	# Inject coverage flags into making of server
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(COVERAGE_FLAGS)" LDFLAGS="$(LDFLAGS) $(COVERAGE_FLAGS)" server
	 # Start server. Ideally we then start some clients and connect to it to do some work.
	./server
	lcov --capture --directory ./ --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report

# ---- Cleanup ----
clean:
	rm -f $(EXES) graph_demo *.o *.so
	$(MAKE) -C graph clean

.PHONY: all clean valgrind helgrind callgrind graph_demo