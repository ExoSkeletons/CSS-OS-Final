// EulerAlgorithm.cpp
// Implements detection and construction of an Euler circuit in an
// undirected graph using Hierholzer's algorithm.  If no Euler
// circuit exists the algorithm returns a descriptive message.

#include "EulerAlgorithm.h"
#include "Graph.h"

#include <vector>
#include <string>
#include <stack>
#include <sstream>
#include <algorithm>

namespace {

// Perform a DFS to check connectivity of vertices with non-zero degree.
void dfs(int u, const Graph &g, std::vector<bool> &visited) {
    visited[u] = true;
    for (auto &edge : g.neighbours(u)) {
        int v = edge.first;
        if (!visited[v]) {
            dfs(v, g, visited);
        }
    }
}

}

std::string EulerAlgorithm::run(const Graph &g) {
    // Only works on undirected graphs.
    if (g.isDirected()) {
        return "Error: Euler circuit algorithm expects an undirected graph.";
    }

    int n = g.numVertices();
    // Find a vertex with non-zero degree to start DFS.
    int start = -1;
    int oddCount = 0;
    for (int i = 0; i < n; ++i) {
        int deg = g.degree(i);
        if (deg % 2 != 0) {
            oddCount++;
        }
        if (deg > 0 && start == -1) {
            start = i;
        }
    }
    // Euler circuit requires all vertices to have even degree.
    if (oddCount != 0) {
        return "No Euler circuit: graph contains vertices of odd degree.";
    }
    // If no edges exist, a trivial circuit exists at vertex 0.
    if (start == -1) {
        return "Graph has no edges; trivial Euler circuit: 0";
    }
    // Check connectivity: vertices with degree > 0 must form one connected component.
    std::vector<bool> visited(n, false);
    dfs(start, g, visited);
    for (int i = 0; i < n; ++i) {
        if (g.degree(i) > 0 && !visited[i]) {
            return "No Euler circuit: graph is not connected.";
        }
    }
    // Build adjacency list with edge IDs to support removal.  Each undirected edge
    // is stored with a unique ID.  When an edge is traversed we mark the
    // ID as used so it is not traversed again.
    struct AdjEdge { int to; int id; };
    std::vector<std::vector<AdjEdge>> adj(n);
    // Build list of edges; since graph is undirected we ensure that each
    // undirected edge is given a single ID but appears in both
    // directions in adj.
    int edgeCount = 0;
    {
        // We'll use a map to avoid assigning duplicate IDs.  This
        // structure maps an ordered pair (u,v) to its id.  Note that
        // there may be parallel edges; we include them separately.
        std::vector<std::tuple<int,int,int>> edges; // (u,v,w)
        for (int u = 0; u < n; ++u) {
            for (const auto &p : g.neighbours(u)) {
                int v = p.first;
                int w = p.second;
                if (u <= v) {
                    edges.emplace_back(u, v, w);
                }
            }
        }
        // Build adjacency lists.  Each undirected edge yields two entries.
        for (const auto &e : edges) {
            int u, v, w;
            std::tie(u, v, w) = e;
            // Add edge u->v
            adj[u].push_back({v, edgeCount});
            if (u != v) {
                adj[v].push_back({u, edgeCount});
            }
            edgeCount++;
        }
    }
    // Track which edges have been used.
    std::vector<bool> used(edgeCount, false);
    std::stack<int> st;
    std::vector<int> circuit;
    st.push(start);
    while (!st.empty()) {
        int u = st.top();
        // Remove used edges from the front of adjacency list until we
        // find one that has not been traversed.
        while (!adj[u].empty() && used[adj[u].back().id]) {
            adj[u].pop_back();
        }
        if (!adj[u].empty()) {
            auto [v, id] = adj[u].back();
            // Mark edge as used
            used[id] = true;
            st.push(v);
        } else {
            // No more edges from u; append to circuit and backtrack
            circuit.push_back(u);
            st.pop();
        }
    }
    if (circuit.size() != static_cast<size_t>(edgeCount) + 1) {
        return "No Euler circuit: failed to traverse all edges.";
    }
    // Build result string.
    std::ostringstream oss;
    oss << "Euler circuit: ";
    for (size_t i = 0; i < circuit.size(); ++i) {
        oss << circuit[i];
        if (i + 1 < circuit.size()) {
            oss << " -> ";
        }
    }
    return oss.str();
}