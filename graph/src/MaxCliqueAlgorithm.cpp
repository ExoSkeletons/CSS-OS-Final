// MaxCliqueAlgorithm.cpp
// Implementation of the Bron–Kerbosch algorithm with pivot to
// compute the maximum clique size in an undirected graph.

#include "MaxCliqueAlgorithm.h"
#include "Graph.h"

#include <vector>
#include <set>
#include <string>
#include <sstream>

namespace {

// Convert graph to adjacency matrix where adj[i][j] is true if there
// exists an edge between i and j (undirected).  For directed graphs
// adjacency is symmetrised.
std::vector<std::vector<bool>> buildAdjacencyMatrix(const Graph &g) {
    int n = g.numVertices();
    std::vector<std::vector<bool>> adj(n, std::vector<bool>(n, false));
    for (int u = 0; u < n; ++u) {
        for (const auto &e : g.neighbours(u)) {
            int v = e.first;
            adj[u][v] = true;
            if (!g.isDirected()) {
                adj[v][u] = true;
            } else {
                // For directed graphs treat edges as undirected for clique
                adj[v][u] = true;
            }
        }
    }
    return adj;
}

void bronk(std::vector<int> &R, std::set<int> &P, std::set<int> &X,
           const std::vector<std::vector<bool>> &adj,
           int &bestSize, std::vector<int> &bestClique) {
    if (P.empty() && X.empty()) {
        if (static_cast<int>(R.size()) > bestSize) {
            bestSize = static_cast<int>(R.size());
            bestClique = R;
        }
        return;
    }
    // Choose a pivot u from P ∪ X with maximum degree to reduce branching
    int u = -1;
    int maxDeg = -1;
    std::set<int> unionPX;
    unionPX.insert(P.begin(), P.end());
    unionPX.insert(X.begin(), X.end());
    for (int v : unionPX) {
        int deg = 0;
        for (int w : P) {
            if (adj[v][w]) deg++;
        }
        if (deg > maxDeg) {
            maxDeg = deg;
            u = v;
        }
    }
    
    std::set<int> candidates;
    for (int v : P) {
        if (u < 0 || !adj[u][v]) {
            candidates.insert(v);
        }
    }
    for (int v : candidates) {
        R.push_back(v);
        
        std::set<int> newP;
        for (int w : P) {
            if (adj[v][w]) newP.insert(w);
        }
        std::set<int> newX;
        for (int w : X) {
            if (adj[v][w]) newX.insert(w);
        }
        bronk(R, newP, newX, adj, bestSize, bestClique);
        // Move v from P to X
        R.pop_back();
        P.erase(v);
        X.insert(v);
    }
}

}

std::string MaxCliqueAlgorithm::run(const Graph &g) {
    int n = g.numVertices();
    if (n == 0) {
        return "Graph is empty; maximum clique size is 0.";
    }
    auto adj = buildAdjacencyMatrix(g);
    int bestSize = 0;
    std::vector<int> bestClique;
    std::vector<int> R;
    std::set<int> P;
    std::set<int> X;
    for (int i = 0; i < n; ++i) P.insert(i);
    bronk(R, P, X, adj, bestSize, bestClique);
    std::ostringstream oss;
    oss << "Maximum clique size: " << bestSize;
    if (!bestClique.empty()) {
        oss << " (nodes: ";
        for (size_t i = 0; i < bestClique.size(); ++i) {
            oss << bestClique[i];
            if (i + 1 < bestClique.size()) oss << ", ";
        }
        oss << ")";
    }
    return oss.str();
}