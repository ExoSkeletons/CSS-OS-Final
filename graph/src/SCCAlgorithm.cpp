// SCCAlgorithm.cpp
// Implementation of Kosaraju's algorithm to compute the number of
// strongly connected components in a directed graph.  For an
// undirected graph this effectively computes connected components.

#include "SCCAlgorithm.h"
#include "Graph.h"
#include <vector>
#include <string>
#include <sstream>

namespace {

void dfsOrder(int u, const Graph &g, std::vector<bool> &visited, std::vector<int> &order) {
    visited[u] = true;
    for (const auto &edge : g.neighbours(u)) {
        int v = edge.first;
        if (!visited[v]) {
            dfsOrder(v, g, visited, order);
        }
    }
    order.push_back(u);
}

void dfsComponent(int u, const Graph &g, std::vector<bool> &visited) {
    visited[u] = true;
    for (const auto &edge : g.neighbours(u)) {
        int v = edge.first;
        if (!visited[v]) {
            dfsComponent(v, g, visited);
        }
    }
}

}

std::string SCCAlgorithm::run(const Graph &g) {
    int n = g.numVertices();
    std::vector<bool> visited(n, false);
    std::vector<int> order;
    order.reserve(n);
    // 1. Build order of vertices by finishing times
    for (int i = 0; i < n; ++i) {
        if (!visited[i]) {
            dfsOrder(i, g, visited, order);
        }
    }
    // 2. Reverse the graph
    Graph rev = g.reversed();
    // 3. DFS in reverse finishing order on reversed graph
    std::fill(visited.begin(), visited.end(), false);
    int count = 0;
    for (int i = static_cast<int>(order.size()) - 1; i >= 0; --i) {
        int u = order[i];
        if (!visited[u]) {
            dfsComponent(u, rev, visited);
            count++;
        }
    }
    std::ostringstream oss;
    oss << "Number of strongly connected components: " << count;
    return oss.str();
}