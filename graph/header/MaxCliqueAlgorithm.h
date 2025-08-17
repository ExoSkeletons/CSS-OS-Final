// MaxCliqueAlgorithm.h
// Implementation of an algorithm to compute the size of the largest
// clique in an undirected graph.  Uses the Bron–Kerbosch algorithm
// with pivoting
#pragma once

#include "Algorithm.h"

class MaxCliqueAlgorithm : public Algorithm {
public:
    std::string name() const override { return "MAXCLIQUE"; }
    std::string run(const Graph &g) override;
};