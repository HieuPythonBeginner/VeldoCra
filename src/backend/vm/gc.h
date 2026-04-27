/**
 * @file gc.h
 * @brief Garbage Collector for VeldoCra VM
 */

#pragma once

#include <vector>
#include <unordered_set>
#include <memory>

namespace veldanava {
namespace vm {

/**
 * @brief Object header for GC
 */
struct GCObject {
    bool marked;
    size_t size;
    void* data;
};

/**
 * @brief Mark-and-sweep garbage collector
 */
class GarbageCollector {
public:
    GarbageCollector();
    ~GarbageCollector();

    // Allocate object
    void* allocate(size_t size);

    // Mark phase - mark reachable objects
    void mark();

    // Sweep phase - free unmarked objects
    void sweep();

    // Full GC cycle
    void collect();

    // Add root object (register/stack)
    void add_root(void* obj);

    // Remove root
    void remove_root(void* obj);

private:
    std::vector<GCObject*> objects_;
    std::unordered_set<void*> roots_;

    // Helper to mark object
    void mark_object(GCObject* obj);
};

} // namespace vm
} // namespace veldanava