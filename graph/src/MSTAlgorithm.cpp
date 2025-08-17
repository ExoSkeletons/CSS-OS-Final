// MSTAlgorithm.cpp
// Implementation of a minimum spanning tree algorithm using Prim's
// algorithm.  The algorithm assumes an undirected weighted graph and
// computes the total weight of the MST.If the graph is not
// connected the algorithm returns an explanatory message.

#include "MSTAlgorithm.h"
#include "Graph.h"

#include <queue>
#include <vector>
#include <string>
#include <sstream>

std::string MSTAlgorithm::run(const Graph &g) {
    if (g.isDirected()) {
        return "Error: MST algorithm expects an undirected graph.";
    }
    int n = g.numVertices();
    if (n ==  0) {
        return "Graph is empty; MST weight is 0.";
    }
    // Use a min-heap keyed by edge weight.  Each entry holds
    // (weight, vertex)
    struct Elem {
        int u;
        int w ;
        bool operator>(const Elem &other) const { return w > other.w; }
    };
    std::vector<bool> inTree(n, false);
    std::priority_queue<Elem, std::vector<Elem>, std::greater<Elem>> pq;
    // Start from vertex 0 (if isolated vertices exist they will
    // prevent spanning tree from covering all vertices).
    inTree[0] = true;
    for (auto &e : g.neighbours(0)) {
        pq.push({e.first, e.second});
    }
    long long totalWeight = 0;
    int visitedCount = 1;
    while (!pq.empty() && visitedCount < n) {
        auto current = pq.top();
        pq.pop();
        int u = current.u;
        int w = current.w;
        if (inTree[u]) continue;
        inTree[u] = true;
        visitedCount++;
        totalWeight += w;
        for (auto &e : g.neighbours(u)) {
            int v = e.first;
            int weight = e.second;
            if (!inTree[v]) {
                pq.push({v, weight});
            }
        }
    }
    if (visitedCount < n) {
        return "Graph is not connected; no spanning tree exists.";
    }
    std::ostringstream oss;
    oss << "MST total weight: " << totalWeight;
    return oss.str();
}