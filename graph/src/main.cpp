// main.cpp
// Entry point for command line program.  Generates a random graph
// based on parameters provided via command line options and executes
// the selected algorithm on that graph.  Results are printed to
// standard output.

#include "AlgorithmFactory.h"
#include "RandomGraph.h"

#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <string>

int main(int argc, char *argv[]) {
    std::string algName = "EULER";
    int vertices = 0;
    int edges = 0;
    unsigned int seed = 1;
    bool directed = false;
    // Weight range for random graph
    int minWeight = 1;
    int maxWeight = 10;
    const struct option longopts[] = {
        {"algorithm", required_argument, nullptr, 'a'},
        {"vertices", required_argument, nullptr, 'v'},
        {"edges", required_argument, nullptr, 'e'},
        {"seed", required_argument, nullptr, 's'},
        {"directed", no_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "a:v:e:s:dh", longopts, nullptr)) != -1) {
        switch (opt) {
        case 'a':
            algName = optarg;
            break;
        case 'v':
            vertices = std::atoi(optarg);
            break;
        case 'e':
            edges = std::atoi(optarg);
            break;
        case 's':
            seed = static_cast<unsigned int>(std::strtoul(optarg, nullptr, 10));
            break;
        case 'd':
            directed = true;
            break;
        case 'h':
            std::cout << "Usage: " << argv[0]
                      << " [--algorithm <alg>] [--vertices <n>] [--edges <m>] [--seed <s>] [--directed]"
                      << std::endl;
            return 0;
        default:
            std::cerr << "Unknown option. Use --help for usage." << std::endl;
            return 1;
        }
    }
    if (vertices <= 0) {
        std::cerr << "Number of vertices must be positive." << std::endl;
        return 1;
    }
    if (edges < 0) {
        std::cerr << "Number of edges cannot be negative." << std::endl;
        return 1;
    }
    // Generate random graph
    Graph g;
    try {
        g = generateRandomGraph(vertices, edges, directed, minWeight, maxWeight, seed);
    } catch (const std::exception &ex) {
        std::cerr << "Error generating graph: " << ex.what() << std::endl;
        return 1;
    }
    // Create algorithm via factory
    auto alg = createAlgorithm(algName);
    if (!alg) {
        std::cerr << "Unknown algorithm: " << algName << std::endl;
        return 1;
    }
    // Execute and print result
    std::string result = alg->run(g);
    std::cout << result << std::endl;
    return 0;
}