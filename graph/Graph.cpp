// Graph.cpp
// Implementation of the Graph class defined in Graph.h.  Provides
// basic methods for constructing and manipulating directed and
// undirected graphs.  The graph uses an adjacency list for
// efficiency.

#include "Graph.h"
#include <stdexcept>
#include <strstream>

Graph::Graph(int vertices, bool directed)
    : m_vertices(vertices), m_directed(directed), m_adj(vertices)
{
}

Graph::Graph(const Graph &graph) {
    m_vertices = graph.m_vertices;
    m_directed = graph.m_directed;
    m_adj = graph.m_adj;
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

std::string to_string(const Graph &g) {
    std::ostrstream s;
    s << g.numVertices() << ' ' << g.isDirected() << std::endl;
    for (int u = 0; u < g.numVertices(); ++u) {
        for (const auto &[v, w] : g.neighbours(u))
            s << ' ' << v << ' ' << w;
        s << std::endl;
    }
    return s.str();
}

Graph from_string(const std::string& str) {
    std::istringstream in(str);
    int vertices;
    bool directed;

    if (!(in >> vertices >> directed)) {
        throw std::runtime_error("Invalid graph format: missing header");
    }

    Graph graph(vertices, directed); // assuming constructor Graph(int vertices, bool directed)

    for (int u = 0; u < vertices; ++u) {
        std::string line;
        if (!std::getline(in >> std::ws, line))
            throw std::runtime_error("Invalid graph format: missing adjacency list");

        std::istringstream lin(line);
        int v, w;
        while (lin >> v >> w)
            graph.addEdge(u, v, w);

        if (!lin.eof())
            throw std::runtime_error("Invalid graph format: bad adjacency entry");
    }

    return graph;
}
