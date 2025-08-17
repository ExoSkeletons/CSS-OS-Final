// Graph.cpp
// Implementation of the Graph class defined in Graph.h.  Provides
// basic methods for constructing and manipulating directed and
// undirected graphs.  The graph uses an adjacency list for
// efficiency.

#include "Graph.h"
#include <stdexcept>

Graph::Graph(int vertices, bool directed)
    : m_vertices(vertices), m_directed(directed), m_adj(vertices)
{
}

void Graph::addEdge(int u, int v, int weight) {
    if (u < 0 || v < 0 || u >= m_vertices || v >= m_vertices) {
        throw std::out_of_range("Vertex index out of bounds");
    }
    m_adj[u].emplace_back(v, weight);
    if (!m_directed) {
        // For undirected graphs insert the reciprocal edge
        m_adj[v].emplace_back(u, weight);
    }
}

Graph Graph::reversed() const {
    Graph rev(m_vertices, m_directed);
    if (m_directed) {
        for (int u = 0; u < m_vertices; ++u) {
            for (const auto &edge : m_adj[u]) {
                int v = edge.first;
                int w = edge.second;
                rev.m_adj[v].emplace_back(u, w);
            }
        }
    } else {
        // For an undirected graph reversing edges yields the same
        // structure.  Copy adjacency list directly.
        rev.m_adj = m_adj;
    }
    return rev;
}