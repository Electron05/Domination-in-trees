#include "memory_tracker.h"
#include <cstdlib>
#include <new>

size_t currentAllocated = 0;
size_t peakAllocated = 0;

namespace {
    constexpr size_t HEADER_SIZE = 16;
}

void* operator new(size_t size) {
    currentAllocated += size;
    if (currentAllocated > peakAllocated) {
        peakAllocated = currentAllocated;
    }

    size_t* ptr = (size_t*)std::malloc(size + HEADER_SIZE);
    if (!ptr) throw std::bad_alloc();
    *ptr = size;
    return (char*)ptr + HEADER_SIZE;
}

void* operator new[](size_t size) {
    return operator new(size);
}

void operator delete(void* ptr) noexcept {
    if (!ptr) return;
    size_t* origPtr = (size_t*)((char*)ptr - HEADER_SIZE);
    currentAllocated -= *origPtr;
    std::free(origPtr);
}

void operator delete[](void* ptr) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    operator delete(ptr);
}
