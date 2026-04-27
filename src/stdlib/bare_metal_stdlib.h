/**
 * @file bare_metal_stdlib.h
 * @brief Bare metal standard library - no OS dependencies
 */

#pragma once

#include <cstdint>
#include <cstddef>

namespace veldanava {
namespace bare_metal {

// Simple memory allocator interface
class Allocator {
public:
    virtual ~Allocator() = default;
    virtual void* allocate(size_t size) = 0;
    virtual void deallocate(void* ptr) = 0;
};

// Bump allocator for bare metal
class BumpAllocator : public Allocator {
public:
    BumpAllocator(void* start, size_t size);
    void* allocate(size_t size) override;
    void deallocate(void* ptr) override;

private:
    uint8_t* current_;
    uint8_t* end_;
};

// Global allocator instance
extern Allocator* g_allocator;

// Vector type (dynamic array)
class Vec {
public:
    Vec();
    ~Vec();

    void push(void* item);
    void* pop();
    size_t size() const;
    void* get(size_t index) const;
    void set(size_t index, void* item);

private:
    void** data_;
    size_t capacity_;
    size_t size_;

    void resize(size_t new_capacity);
};

// Map type (hash table with simple hashing)
class Map {
public:
    Map();
    ~Map();

    void insert(const char* key, void* value);
    void* get(const char* key) const;
    bool contains(const char* key) const;
    void remove(const char* key);

private:
    struct Entry {
        const char* key;
        void* value;
        bool occupied;
    };

    Entry* entries_;
    size_t capacity_;
    size_t size_;

    size_t hash(const char* key) const;
    void resize(size_t new_capacity);
    void rehash();
};

// String functions
char* string_concat(const char* a, const char* b);
char** string_split(const char* str, const char* delim, size_t* count);
char* string_substring(const char* str, size_t start, size_t len);
size_t string_length(const char* str);
int string_compare(const char* a, const char* b);

// Math functions
double math_abs(double x);
double math_pow(double base, double exp);
double math_sqrt(double x);
int math_random_int(int min, int max);

// I/O functions (bare metal - direct hardware access)
// For now, stub implementations
void io_print(const char* str);
char* io_read_line();
void io_write_file(const char* filename, const char* content);
char* io_read_file(const char* filename);

// System functions
void system_exit(int code);
void* system_malloc(size_t size);
void system_free(void* ptr);

} // namespace bare_metal
} // namespace veldanava