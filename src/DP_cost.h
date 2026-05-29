#ifndef DP_COST_H
#define DP_COST_H

#include <limits>

static const int INF = std::numeric_limits<int>::max();

struct Cost {
    int value;

    Cost(int v = 0) : value(v) {}

    bool is_impossible() const { return value == INF; }

    static Cost impossible() { return Cost(INF); }

    Cost operator+(const Cost& other) const {
        if (is_impossible() || other.is_impossible()) {
            return Cost::impossible();
        }
        return Cost(value + other.value);
    }

    Cost& operator+=(const Cost& other) {
        *this = *this + other;
        return *this;
    }

    bool operator<(const Cost& other) const { return value < other.value; }
};

#endif