// RandomGraph.cpp
// Implements generation of random graphs with specified numbers of
// vertices and edges.  The graph can be directed or undirected and
// supports weighted edges.

#include "RandomGraph.h"
#include "Graph.h"

#include <random>
#include <stdexcept>
#include <set>

Graph generateRandomGraph(int vertices, int edges, bool directed,
                          int minWeight, int maxWeight,
                          unsigned int seed) {
    if (vertices < 0 || edges < 0) {
        throw std::invalid_argument("Number of vertices and edges must be non-negative");
    }
    // Maximum number of distinct edges for directed/undirected graphs
    long long maxEdges = 0;
    if (directed) {
        // For directed graphs, loops are allowed
        maxEdges = static_cast<long long>(vertices) * vertices;
    } else {
        // For undirected graphs, loops count as self-loops and are
        // considered distinct; edges (u,v) and (v,u) are the same
        // undirected edge
        maxEdges = static_cast<long long>(vertices) * (vertices + 1) / 2;
    }
    if (edges > maxEdges) {
        throw std::invalid_argument("Too many edges for given number of vertices");
    }
    Graph g(vertices, directed);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> weightDist(minWeight, maxWeight);
    std::uniform_int_distribution<int> vertexDist(0, vertices - 1);
    // Keep track of existing edges to avoid duplicates
    std::set<std::pair<int,int>> used;
    while (used.size() < edges) {
        int u = vertexDist(rng);
        int v = vertexDist(rng);
        if (u == v) continue;
        // For undirected graphs ensure (u,v) and (v,u) considered same
        std::pair<int,int> key;
        if (directed) {
            key = {u, v};
        } else {
            if (u <= v) key = {u, v}; else key = {v, u};
        }
        if (used.contains(key))
            continue;
        used.insert(key);
        int w = weightDist(rng);
        g.addEdge(u, v, w);
    }
    return g;
}