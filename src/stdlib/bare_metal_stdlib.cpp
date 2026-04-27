/**
 * @file bare_metal_stdlib.cpp
 * @brief Bare metal standard library implementation
 */

#include "bare_metal_stdlib.h"
#include <cstring> // For memcpy, memset, strlen

// x86 I/O port functions (inline assembly)
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

namespace veldanava {
namespace bare_metal {

// Global allocator
Allocator* g_allocator = nullptr;

// Bump Allocator Implementation
BumpAllocator::BumpAllocator(void* start, size_t size)
    : current_(static_cast<uint8_t*>(start)), end_(static_cast<uint8_t*>(start) + size) {}

void* BumpAllocator::allocate(size_t size) {
    if (current_ + size > end_) {
        return nullptr; // Out of memory
    }
    void* result = current_;
    current_ += size;
    return result;
}

void BumpAllocator::deallocate(void* ptr) {
    // Bump allocator doesn't support deallocation
    // In a real implementation, we'd need a more sophisticated allocator
}

// Vec Implementation
Vec::Vec() : data_(nullptr), capacity_(0), size_(0) {}

Vec::~Vec() {
    if (data_ && g_allocator) {
        g_allocator->deallocate(data_);
    }
}

void Vec::push(void* item) {
    if (size_ >= capacity_) {
        resize(capacity_ == 0 ? 4 : capacity_ * 2);
    }
    data_[size_++] = item;
}

void* Vec::pop() {
    if (size_ == 0) return nullptr;
    return data_[--size_];
}

size_t Vec::size() const {
    return size_;
}

void* Vec::get(size_t index) const {
    if (index >= size_) return nullptr;
    return data_[index];
}

void Vec::set(size_t index, void* item) {
    if (index >= size_) return;
    data_[index] = item;
}

void Vec::resize(size_t new_capacity) {
    void** new_data = static_cast<void**>(g_allocator->allocate(new_capacity * sizeof(void*)));
    if (data_) {
        memcpy(new_data, data_, size_ * sizeof(void*));
        g_allocator->deallocate(data_);
    }
    data_ = new_data;
    capacity_ = new_capacity;
}

// Map Implementation
Map::Map() : entries_(nullptr), capacity_(0), size_(0) {}

Map::~Map() {
    if (entries_ && g_allocator) {
        g_allocator->deallocate(entries_);
    }
}

size_t Map::hash(const char* key) const {
    size_t hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void Map::insert(const char* key, void* value) {
    if (capacity_ == 0) {
        resize(16);
    }

    size_t index = hash(key) % capacity_;
    size_t start_index = index;

    do {
        if (!entries_[index].occupied) {
            entries_[index].key = key;
            entries_[index].value = value;
            entries_[index].occupied = true;
            size_++;
            return;
        }
        if (strcmp(entries_[index].key, key) == 0) {
            entries_[index].value = value;
            return;
        }
        index = (index + 1) % capacity_;
    } while (index != start_index);

    // Table is full, resize
    resize(capacity_ * 2);
    insert(key, value);
}

void* Map::get(const char* key) const {
    if (capacity_ == 0) return nullptr;

    size_t index = hash(key) % capacity_;
    size_t start_index = index;

    do {
        if (entries_[index].occupied && strcmp(entries_[index].key, key) == 0) {
            return entries_[index].value;
        }
        index = (index + 1) % capacity_;
    } while (index != start_index && entries_[index].occupied);

    return nullptr;
}

bool Map::contains(const char* key) const {
    return get(key) != nullptr;
}

void Map::remove(const char* key) {
    if (capacity_ == 0) return;

    size_t index = hash(key) % capacity_;
    size_t start_index = index;

    do {
        if (entries_[index].occupied && strcmp(entries_[index].key, key) == 0) {
            entries_[index].occupied = false;
            size_--;
            rehash();
            return;
        }
        index = (index + 1) % capacity_;
    } while (index != start_index);
}

void Map::resize(size_t new_capacity) {
    Entry* new_entries = static_cast<Entry*>(g_allocator->allocate(new_capacity * sizeof(Entry)));
    memset(new_entries, 0, new_capacity * sizeof(Entry));

    if (entries_) {
        for (size_t i = 0; i < capacity_; ++i) {
            if (entries_[i].occupied) {
                size_t new_index = hash(entries_[i].key) % new_capacity;
                size_t start_index = new_index;
                while (new_entries[new_index].occupied) {
                    new_index = (new_index + 1) % new_capacity;
                    if (new_index == start_index) break; // Should not happen
                }
                new_entries[new_index] = entries_[i];
            }
        }
        g_allocator->deallocate(entries_);
    }

    entries_ = new_entries;
    capacity_ = new_capacity;
}

void Map::rehash() {
    // For simplicity, we'll skip rehashing after removal
    // In a production implementation, we'd rebuild the table
}

// String functions
char* string_concat(const char* a, const char* b) {
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    char* result = static_cast<char*>(g_allocator->allocate(len_a + len_b + 1));
    memcpy(result, a, len_a);
    memcpy(result + len_a, b, len_b);
    result[len_a + len_b] = '\0';
    return result;
}

char** string_split(const char* str, const char* delim, size_t* count) {
    // Simplified implementation - splits on single character delimiter
    size_t delim_len = strlen(delim);
    if (delim_len != 1) return nullptr; // Only support single char delimiters for now

    // Count occurrences
    *count = 1;
    const char* ptr = str;
    while ((ptr = strstr(ptr, delim))) {
        (*count)++;
        ptr += delim_len;
    }

    char** result = static_cast<char**>(g_allocator->allocate(*count * sizeof(char*)));

    // Split
    size_t index = 0;
    const char* start = str;
    ptr = strstr(str, delim);
    while (ptr) {
        size_t len = ptr - start;
        result[index] = static_cast<char*>(g_allocator->allocate(len + 1));
        memcpy(result[index], start, len);
        result[index][len] = '\0';
        index++;
        start = ptr + delim_len;
        ptr = strstr(start, delim);
    }

    // Last part
    size_t len = strlen(start);
    result[index] = static_cast<char*>(g_allocator->allocate(len + 1));
    memcpy(result[index], start, len);
    result[index][len] = '\0';

    return result;
}

char* string_substring(const char* str, size_t start, size_t len) {
    size_t str_len = strlen(str);
    if (start >= str_len) return nullptr;
    if (start + len > str_len) len = str_len - start;

    char* result = static_cast<char*>(g_allocator->allocate(len + 1));
    memcpy(result, str + start, len);
    result[len] = '\0';
    return result;
}

size_t string_length(const char* str) {
    return strlen(str);
}

int string_compare(const char* a, const char* b) {
    return strcmp(a, b);
}

// Math functions (simplified implementations)
double math_abs(double x) {
    return x < 0 ? -x : x;
}

double math_pow(double base, double exp) {
    // Simple implementation for integer exponents
    if (exp == 0) return 1;
    if (exp < 0) return 1 / math_pow(base, -exp);

    double result = 1;
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

double math_sqrt(double x) {
    if (x < 0) return -1; // Error

    double guess = x / 2;
    for (int i = 0; i < 10; ++i) { // 10 iterations
        guess = (guess + x / guess) / 2;
    }
    return guess;
}

int math_random_int(int min, int max) {
    // Very simple linear congruential generator
    static unsigned int seed = 1;
    seed = seed * 1103515245 + 12345;
    return min + (seed % (max - min + 1));
}

// I/O functions (bare metal - direct hardware access)
// For x86_64, use serial port COM1 (0x3F8) for output

// Serial port I/O functions
void serial_init() {
    // Initialize COM1 serial port
    outb(0x3F8 + 1, 0x00);    // Disable interrupts
    outb(0x3F8 + 3, 0x80);    // Enable DLAB
    outb(0x3F8 + 0, 0x03);    // Set divisor to 3 (38400 baud)
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(0x3F8 + 2, 0xC7);    // Enable FIFO, clear them, 14-byte threshold
    outb(0x3F8 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void serial_putc(char c) {
    while ((inb(0x3F8 + 5) & 0x20) == 0); // Wait for transmitter empty
    outb(0x3F8, c);
}

void io_print(const char* str) {
    static bool initialized = false;
    if (!initialized) {
        serial_init();
        initialized = true;
    }

    while (*str) {
        serial_putc(*str++);
    }
    serial_putc('\n');
}

char* io_read_line() {
    // For bare metal, reading input is complex without interrupts
    // For now, return empty string
    char* result = static_cast<char*>(g_allocator->allocate(2));
    result[0] = '\0';
    return result;
}

void io_write_file(const char* filename, const char* content) {
    // File I/O not available in bare metal
    // Could implement basic RAM disk in future
    (void)filename; (void)content;
}

char* io_read_file(const char* filename) {
    // File I/O not available in bare metal
    (void)filename;
    return nullptr;
}

// System functions
void system_exit(int code) {
    // In bare metal, this would be a syscall or halt instruction
    while (true) {} // Infinite loop
}

void* system_malloc(size_t size) {
    return g_allocator ? g_allocator->allocate(size) : nullptr;
}

void system_free(void* ptr) {
    if (g_allocator) g_allocator->deallocate(ptr);
}

} // namespace bare_metal
} // namespace veldanava