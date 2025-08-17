// AlgorithmFactory.cpp
// Implements a simple factory function to construct concrete
// algorithm objects based on a string name.  If the name does not
// correspond to a known algorithm a nullptr is returned.

#include "AlgorithmFactory.h"
#include "EulerAlgorithm.h"
#include "MSTAlgorithm.h"
#include "SCCAlgorithm.h"
#include "MaxFlowAlgorithm.h"
#include "MaxCliqueAlgorithm.h"

#include <algorithm>

AlgorithmPtr createAlgorithm(const std::string &name) {
    // Convert name to uppercase for case-insensitive comparison
    std::string upper;
    upper.reserve(name.size());
    for (char c : name) {
        upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    if (upper == "EULER") {
        return std::make_unique<EulerAlgorithm>();
    } else if (upper == "MST") {
        return std::make_unique<MSTAlgorithm>();
    } else if (upper == "SCC") {
        return std::make_unique<SCCAlgorithm>();
    } else if (upper == "MAXFLOW") {
        return std::make_unique<MaxFlowAlgorithm>();
    } else if (upper == "MAXCLIQUE") {
        return std::make_unique<MaxCliqueAlgorithm>();
    } else {
        return nullptr;
    }
}