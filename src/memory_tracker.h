#pragma once
#include <cstddef>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

extern size_t currentAllocated;
extern size_t peakAllocated;

class ScopedMemoryTracker {
private:
    size_t startAlloc;
    char* stackStart;

public:
    ScopedMemoryTracker() {
        char marker;
        stackStart = &marker;
        startAlloc = currentAllocated;
        peakAllocated = startAlloc;
    }

    size_t getPeakHeapBytes() const {
        return (peakAllocated >= startAlloc) ? (peakAllocated - startAlloc) : 0;
    }

    size_t getStackBytes() const {
        char marker;
        return (size_t)std::abs(stackStart - &marker);
    }

    size_t getTotalPeakBytes() const {
        return getPeakHeapBytes() + getStackBytes();
    }
};

inline std::string formatMemory(size_t bytes) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (bytes < 1024) {
        ss << bytes << " B";
    } else if (bytes < 1024 * 1024) {
        ss << (bytes / 1024.0) << " KB";
    } else {
        ss << (bytes / (1024.0 * 1024.0)) << " MB";
    }
    return ss.str();
}
