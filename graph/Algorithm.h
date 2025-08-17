// Algorithm.h
// Defines the base class for all graph algorithms.  Each derived
// algorithm should implement the run() method to compute its result
// on a given graph.  The result is returned as a human readable
// string.  Derived classes also provide a unique name to identify
// them.

#pragma once

#include <string>
#include <memory>

class Graph;

// Abstract base class for graph algorithms.  Algorithms operate on a
// Graph and return their result as a string.  The name() method
// provides a unique identifier used by the factory.
class Algorithm {
public:
    virtual ~Algorithm() = default;

    // Return the name used to reference this algorithm.  Derived
    // classes must implement this to provide a unique identifier.
    virtual std::string name() const = 0;

    // Execute the algorithm on the provided graph and return a
    // descriptive string with the result.  The graph is passed by
    // reference; algorithms should not modify it.
    virtual std::string run(const Graph &g) = 0;
};

using AlgorithmPtr = std::unique_ptr<Algorithm>;