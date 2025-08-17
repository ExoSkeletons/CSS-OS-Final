// RandomGraph.h
// Utility for generating random graphs.  Can generate directed or
// undirected graphs with a specified number of vertices and edges.

#pragma once

#include "Graph.h"
#include <random>

// Generate a random graph with a given number of vertices and edges.
// If directed is true the graph is directed; otherwise it is
// undirected.  Edge weights are drawn uniformly from the range
// [minWeight, maxWeight].  A random seed can be supplied to control
// repeatability. 
Graph generateRandomGraph(int vertices, int edges, bool directed,
                          int minWeight, int maxWeight,
                          unsigned int seed);