/**
 * @file stdlib.h
 * @brief VeldoCra Standard Library Header
 * @description Soul/Magic Theme Naming for unique identity
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <random>

namespace veldanava {
namespace stdlib {

// ============================================================================
// 📦 Collections - The Nexus System
// ============================================================================

// Sequence (Vector/Array)
class Sequence {
public:
    Sequence() = default;
    void push(void* item);                      // Add item
    void* pop();                                // Remove last, return it
    size_t extent() const;                      // Get size
    void* peek(size_t index) const;             // Get item at index
    void mutate(size_t index, void* new_val);   // Set item at index
    void graft(size_t index, void* item);       // Insert at index

private:
    std::vector<void*> data_;
};

// Registry (Map/Dict)
class Registry {
public:
    Registry() = default;
    void bind(const std::string& key, void* value);   // Put key-value
    void* fetch(const std::string& key) const;        // Get value
    bool exists(const std::string& key) const;        // Check if key exists
    void sever(const std::string& key);               // Remove key
    size_t extent() const;                            // Get size

private:
    std::unordered_map<std::string, void*> data_;
};

// Collective (Set)
class Collective {
public:
    Collective() = default;
    void enroll(void* item);              // Add item
    void dismiss(void* item);             // Remove item
    bool has(void* item) const;           // Check membership
    void merge(const Collective& other);  // Union
    void mesh(const Collective& other);   // Intersect
    size_t extent() const;                // Get size
    void clear();                         // Clear all

private:
    std::vector<void*> data_;
};

// Memory allocation functions (for C++ stdlib, not language keywords)
namespace memory {
    void* forge(size_t size);             // alloc
    void* clm(size_t size);               // malloc
    void* pur(size_t num, size_t size);   // calloc
    void* rsz(void* ptr, size_t size);    // realloc
    void ee(void* ptr);                   // free
    void* touch(void* ptr);               // deref (placeholder)
};

// ============================================================================
// 📥 I/O - The Echo System
// ============================================================================

void scribe(const std::string& str);                      // Print string
void scribe(int num);                                     // Print number
void scribe(double num);                                  // Print float
void scribe(bool val);                                    // Print bool
void emit(double val);                                    // Print value (alternative)

// Input (Future)
std::string listen_str();                                 // Read string
int listen_int();                                         // Read int
double listen_val();                                      // Read float
bool listen_logic();                                      // Read bool

// File I/O
void inscribe(const std::string& filename, const std::string& content);
std::string perceive(const std::string& filename);

// ============================================================================
// 🔢 Math - The Calculus Core
// ============================================================================

// Basic Math
double abs_val(double x);                                 // Absolute value
double root(double x);                                    // Square root
double exp_val(double base, double exp);                  // Power
int floor_val(double x);                                  // Floor
int ceiling_val(double x);                                // Ceiling

// Trigonometric
double sin_val(double x);
double cos_val(double x);
double tan_val(double x);
double asin_val(double x);
double acos_val(double x);

// Constants
constexpr double PI = 3.14159265358979323846;
constexpr double TAU = 6.28318530717958647692;
constexpr double EULER = 2.71828182845904523536;

// Utilities
double confine(double val, double min_val, double max_val);  // Clamp value
double bridge(double a, double b, double t);                 // Linear interpolation

// Random
double chaos_float();                                      // 0.0 to 1.0
int chaos_int(int min_val, int max_val);                   // min to max
void* chaos_pick(void* array, size_t size);                // Random array element

// ============================================================================
// 📝 Strings - The Lexicon Operations
// ============================================================================

// Info
size_t lex_span(const std::string& str);                  // Length
bool lex_void(const std::string& str);                    // Is empty
char lex_glyph_at(const std::string& str, size_t index); // Char at index

// Modification
std::string lex_elevate(const std::string& str);          // Uppercase
std::string lex_depress(const std::string& str);          // Lowercase
std::string lex_sanitize(const std::string& str);         // Trim
std::string lex_swap(const std::string& str, const std::string& from, const std::string& to);

// Search
bool lex_find(const std::string& str, const std::string& sub);           // Contains
bool lex_anchors_with(const std::string& str, const std::string& pref); // Starts with
bool lex_trails_with(const std::string& str, const std::string& suff);  // Ends with

// Conversion
int lex_parse_int(const std::string& str);                // Parse int
double lex_parse_float(const std::string& str);           // Parse float
bool lex_parse_logic(const std::string& str);             // Parse bool

// Advanced
std::vector<std::string> lex_shatter(const std::string& str, const std::string& delim);
std::string lex_weave(const std::vector<std::string>& parts, const std::string& sep);

// ============================================================================
// 🛠️ Type Conversion
// ============================================================================

std::string to_text(int val);
std::string to_text(double val);
std::string to_text(bool val);
int to_integer(const std::string& str);
double to_number(const std::string& str);

} // namespace stdlib
} // namespace veldanava