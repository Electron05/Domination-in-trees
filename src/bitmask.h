#pragma once

#include <bitset>

#ifndef MAX_K
#define MAX_K 10
#endif

using BitMask = std::bitset<MAX_K>;

inline bool isFull(const BitMask& mask, int k) {
    return mask.count() == (size_t)(k);
}
