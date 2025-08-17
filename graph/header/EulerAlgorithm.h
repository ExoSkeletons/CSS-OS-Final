// EulerAlgorithm.h
// Implementation of an algorithm to find an Euler circuit in an
// undirected graph.  Uses Hierholzer's algorithm to compute the
// circuit if it exists.

#pragma once

#include "Algorithm.h"

// Forward declaration
class Graph;

class EulerAlgorithm : public Algorithm {
public:
    std::string name() const override { return "EULER"; }

    // Execute the Euler circuit algorithm on the provided graph.
    // Returns a description of the circuit if one exists, or a
    // message indicating that no Euler circuit exists.  If the graph
    // is directed the algorithm prints a warning and returns an empty
    // result.
    std::string run(const Graph &g) override;
};