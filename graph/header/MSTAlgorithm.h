// MSTAlgorithm.h
// Computes the weight of a minimum spanning tree (MST) of an
// undirected weighted graph using Prim's algorithm.  If the graph
// contains more than one connected component the MST does not exist
// and an appropriate message is returned.

#pragma once

#include "Algorithm.h"

class MSTAlgorithm : public Algorithm {
public:
    std::string name() const override { return "MST"; }
    std::string run(const Graph &g) override;
};