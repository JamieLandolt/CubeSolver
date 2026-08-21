#ifndef HASH_H
#define HASH_H

#include <cstdint>
#include <functional>
#include <utility>
#include <memory>

// A hash for a cube state to store visited states while DFSing
struct StateHash {
    size_t operator()(const std::pair<long, long>& p) const {
        size_t h1 = std::hash<long>()(p.first);
        size_t h2 = std::hash<long>()(p.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

struct PathEntry {
    std::pair<long,long> parent_state;
    std::string move;
};

struct DFSEntry {
    DFSEntry* parent_state;
    long corners;
    long edges;
    std::string move;
    int depth;
};

#endif
