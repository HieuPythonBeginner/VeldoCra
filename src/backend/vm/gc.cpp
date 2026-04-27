/**
 * @file gc.cpp
 * @brief Garbage Collector for VeldoCra VM (stub for no-GC build)
 */

#include "gc.h"

namespace veldanava {
namespace vm {

GarbageCollector::GarbageCollector() {}
GarbageCollector::~GarbageCollector() {}

void* GarbageCollector::allocate(size_t size) {
    return ::operator new(size);
}

void GarbageCollector::mark() {}
void GarbageCollector::sweep() {}
void GarbageCollector::collect() {}
void GarbageCollector::add_root(void* obj) {}
void GarbageCollector::remove_root(void* obj) {}
void GarbageCollector::mark_object(GCObject* obj) {}

} // namespace vm
} // namespace veldanava