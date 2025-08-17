// SCCAlgorithm.h
// Computes the number of strongly connected components (SCC) in a
// directed graph using Kosaraju's algorithm.  For undirected
// graphs the SCC count corresponds to the number of connected
// components.

#pragma once

#include "Algorithm.h"

class SCCAlgorithm : public Algorithm {
public:
    std::string name() const override { return "SCC"; }
    std::string run(const Graph &g) override;
};