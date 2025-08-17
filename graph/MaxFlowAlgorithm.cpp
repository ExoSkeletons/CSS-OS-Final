// MaxFlowAlgorithm.cpp
// Implementation of the Edmonds–Karp algorithm to compute the maximum
// flow between the source (vertex 0) and the sink (vertex n-1) in a
// directed or undirected graph. 
#include "MaxFlowAlgorithm.h"
#include "Graph.h"

#include <vector>
#include <queue>
#include <limits>
#include <string>
#include <sstream>

std::string MaxFlowAlgorithm::run(const Graph &g) {
    int n = g.numVertices();
    if (n < 2) {
        return "Graph must contain at least two vertices to compute max flow.";
    }
    int source = 0;
    int sink = n - 1;
    // Build capacity matrix
    std::vector<std::vector<long long>> capacity(n, std::vector<long long>(n, 0));
    for (int u = 0; u < n; ++u) {
        for (const auto &e : g.neighbours(u)) {
            int v = e.first;
            long long w = e.second;
            capacity[u][v] += w;
            if (!g.isDirected()) {
                capacity[v][u] += w;
            }
        }
    }
    //  capacities initially equal to capacity
    std::vector<std::vector<long long>> residual = capacity;
    long long maxFlow = 0;
    while (true) {
        // BFS to find shortest augmenting path
        std::vector<int> parent(n, -1);
        parent[source] = -2; // mark source
        std::queue<std::pair<int,long long>> q;
        q.push({source, std::numeric_limits<long long>::max()});
        long long pathFlow = 0;
        while (!q.empty()) {
            int u = q.front().first;
            long long flow = q.front().second;
            q.pop();
            for (int v = 0; v < n; ++v) {
                if (parent[v] == -1 && residual[u][v] > 0) {
                    parent[v] = u;
                    long long newFlow = std::min(flow, residual[u][v]);
                    if (v == sink) {
                        pathFlow = newFlow;
                        break;
                    }
                    q.push({v, newFlow});
                }
            }
            if (pathFlow > 0) break;
        }
        if (pathFlow == 0) {
            // No augmenting path found
            break;
        }
        maxFlow += pathFlow;
        // Update residual capacities along path
        int v = sink;
        while (v != source) {
            int u = parent[v];
            residual[u][v] -= pathFlow;
            residual[v][u] += pathFlow;
            v = u;
        }
    }
    std::ostringstream oss;
    oss << "Max flow from 0 to " << sink << ": " << maxFlow;
    return oss.str();
}