// MaxFlowAlgorithm.h
// Computes the maximum flow between vertex 0 (source) and
// vertex (n-1) (sink) in a weighted directed graph using the
// Edmonds–Karp implementation of the Ford–Fulkerson method.  For
// undirected graphs each undirected edge is treated as two opposing
// directed edges with the same capacity.

#pragma once

#include "Algorithm.h"

class MaxFlowAlgorithm : public Algorithm {
public:
    std::string name() const override { return "MAXFLOW"; }
    std::string run(const Graph &g) override;
};