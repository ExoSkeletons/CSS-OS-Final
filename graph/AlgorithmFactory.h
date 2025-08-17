// AlgorithmFactory.h
// Provides a factory for constructing algorithm objects based on a
// requested name.  Encapsulates knowledge of available algorithm
// classes.This
// implements part of the Factory design pattern.

#pragma once

#include "Algorithm.h"
#include <string>

// Create a unique_ptr to an Algorithm based on the provided name.
// Returns nullptr if no algorithm with the given name is known.  The
// names are compared case-insensitively.
AlgorithmPtr createAlgorithm(const std::string &name);