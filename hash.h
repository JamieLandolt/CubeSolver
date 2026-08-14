#ifndef HASH_H
#define HASH_H

#include <cstdint>
#include <vector>
#include <utility>

// A hash for a cube state to store visited states while DFSing
struct StateHash {
    size_t operator()(const std::pair<uint32_t, uint64_t>& p) const {
        size_t h1 = std::hash<uint32_t>()(p.first);
        size_t h2 = std::hash<uint64_t>()(p.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

uint64_t permRank(const std::vector<int>& perm) {
    int n = perm.size();
    std::vector<bool> used(n, false);
    uint64_t rank = 0;
    uint64_t fact = 1;
    for (int i = n - 2; i >= 0; i--) fact *= (i + 1); // (n-1)!

    for (int i = 0; i < n; i++) {
        int smaller = 0;
        for (int j = perm[i] - 1; j >= 0; j--) {
            if (!used[j]) smaller++;
        }
        rank += smaller * fact;
        used[perm[i]] = true;
        if (i < n - 1) fact /= (n - 1 - i);
    }
    return rank;
}

// vector 1: 8 pairs, first value 0-2, second value unique 0-7, order matters
uint32_t encodeVec1(const std::vector<std::pair<int,int>>& v) {
    std::vector<int> order(8);
    int arr[8];
    for (int i = 0; i < 8; i++) {
        order[i] = v[i].second;   // sequence of labels
        arr[v[i].second] = v[i].first;
    }
    uint32_t permPart = (uint32_t)permRank(order);   // 0 to 40319
    uint32_t valuePart = 0;
    for (int i = 0; i < 8; i++) valuePart = valuePart * 3 + arr[i]; // 0 to 6560

    return permPart * 6561 + valuePart;
}

// vector 2: 12 pairs, first value 0-1, second value unique 0-11, order matters
uint64_t encodeVec2(const std::vector<std::pair<int,int>>& v) {
    std::vector<int> order(12);
    uint32_t bitmask = 0;
    for (int i = 0; i < 12; i++) {
        order[i] = v[i].second;
        if (v[i].first == 1) bitmask |= (1u << v[i].second);
    }
    uint64_t permPart = permRank(order);   // 0 to 479001599
    uint64_t valuePart = bitmask;          // 0 to 4095

    return permPart * 4096ull + valuePart;
}

#endif
