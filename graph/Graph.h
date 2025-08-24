// Graph.h
// Simple graph data structure supporting both directed and undirected graphs.
// The graph stores adjacency lists along with optional edge weights.

#pragma once

#include <string>
#include <vector>
#include <utility>

// A simple weighted graph representation.  Vertices are zero indexed
// from 0 to (n-1).  An adjacency list stores for each vertex a list of
// neighbours with corresponding edge weights.  The graph can be
// marked as directed or undirected.  For undirected graphs the
// addEdge() method automatically adds the symmetric counterpart.
class Graph {
public:
	// Construct a graph with a given number of vertices.  By default
	// the graph is undirected.  Passing directed=true will build a
	// directed graph.
	explicit Graph(int vertices = 0, bool directed = false);

	Graph(const Graph &graph);

	// Add an edge between u and v with an optional weight (default
	// weight = 1).  The vertices must be in the range [0, n-1].  If
	// the graph is undirected the function automatically inserts a
	// symmetric edge.
	void addEdge(int u, int v, int weight = 1);

	// Return the number of vertices in the graph.
	int numVertices() const { return m_vertices; }

	// Return true if the graph is directed.
	bool isDirected() const { return m_directed; }

	// Access adjacency list for a given vertex.  Each entry is a
	// pair (neighbour, weight).  The returned reference is const to
	// prevent modification by clients.
	const std::vector<std::pair<int, int> > &neighbours(int u) const {
		return m_adj[u];
	}

	// Compute the degree of a vertex (for undirected graphs) or
	// out-degree for directed graphs.
	int degree(int u) const { return static_cast<int>(m_adj[u].size()); }

	// Reverse the direction of all edges in the graph.  Only
	// meaningful on directed graphs.  Returns a new Graph instance.
	Graph reversed() const;

private:
	int m_vertices;
	bool m_directed;
	std::vector<std::vector<std::pair<int, int> > > m_adj;
};


std::string to_string(const Graph &g);

Graph from_string(const std::string &str);
