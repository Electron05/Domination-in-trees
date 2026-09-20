#pragma once

#include <boost/dynamic_bitset.hpp>

using BitMask = boost::dynamic_bitset<>;

inline bool isFull(const BitMask& mask) {
    return mask.count() == mask.size();
}
