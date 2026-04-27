/**
 * @file stdlib.cpp
 * @brief VeldoCra Standard Library Implementation
 * @description Soul/Magic Theme Naming for unique identity
 */

#include "stdlib.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace veldanava {
namespace stdlib {

// ============================================================================
// 📦 Collections - The Nexus System
// ============================================================================

// Sequence (Vector/Array)
void Sequence::push(void* item) {
    data_.push_back(item);
}

void* Sequence::pop() {
    if (data_.empty()) return nullptr;
    void* item = data_.back();
    data_.pop_back();
    return item;
}

size_t Sequence::extent() const {
    return data_.size();
}

void* Sequence::peek(size_t index) const {
    if (index >= data_.size()) return nullptr;
    return data_[index];
}

void Sequence::mutate(size_t index, void* new_val) {
    if (index >= data_.size()) return;
    data_[index] = new_val;
}

void Sequence::graft(size_t index, void* item) {
    if (index > data_.size()) return;
    data_.insert(data_.begin() + index, item);
}

// Registry (Map/Dict)
void Registry::bind(const std::string& key, void* value) {
    data_[key] = value;
}

void* Registry::fetch(const std::string& key) const {
    auto it = data_.find(key);
    return it != data_.end() ? it->second : nullptr;
}

bool Registry::exists(const std::string& key) const {
    return data_.count(key) > 0;
}

void Registry::sever(const std::string& key) {
    data_.erase(key);
}

size_t Registry::extent() const {
    return data_.size();
}

// Collective (Set)
void Collective::enroll(void* item) {
    if (!has(item)) {
        data_.push_back(item);
    }
}

void Collective::dismiss(void* item) {
    data_.erase(std::remove(data_.begin(), data_.end(), item), data_.end());
}

bool Collective::has(void* item) const {
    return std::find(data_.begin(), data_.end(), item) != data_.end();
}

void Collective::merge(const Collective& other) {
    for (size_t i = 0; i < other.data_.size(); i++) {
        enroll(other.data_[i]);
    }
}

void Collective::mesh(const Collective& other) {
    std::vector<void*> result;
    for (size_t i = 0; i < data_.size(); i++) {
        if (other.has(data_[i])) {
            result.push_back(data_[i]);
        }
    }
    data_ = result;
}

size_t Collective::extent() const {
    return data_.size();
}

void Collective::clear() {
    data_.clear();
}

// ============================================================================
// Memory Operations (C++ implementations, not language keywords)
// ============================================================================

namespace memory {

void* forge(size_t size) {
    return std::malloc(size);
}

void* clm(size_t size) {
    return std::malloc(size);
}

void* pur(size_t num, size_t size) {
    return std::calloc(num, size);
}

void* rsz(void* ptr, size_t size) {
    return std::realloc(ptr, size);
}

void ee(void* ptr) {
    std::free(ptr);
}

void* touch(void* ptr) {
    return ptr;  // Placeholder for deref
}

} // namespace memory

// ============================================================================
// 📥 I/O - The Echo System
// ============================================================================

void scribe(const std::string& str) {
    std::cout << str;
}

void scribe(int num) {
    std::cout << num;
}

void scribe(double num) {
    std::cout << num;
}

void scribe(bool val) {
    std::cout << (val ? "true" : "false");
}

void emit(double val) {
    std::cout << val << std::endl;
}

// Input (Future)
std::string listen_str() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int listen_int() {
    int val;
    std::cin >> val;
    return val;
}

double listen_val() {
    double val;
    std::cin >> val;
    return val;
}

bool listen_logic() {
    bool val;
    std::cin >> val;
    return val;
}

// File I/O
void inscribe(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (file) {
        file << content;
    }
}

std::string perceive(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ============================================================================
// 🔢 Math - The Calculus Core
// ============================================================================

// Basic Math
double abs_val(double x) {
    return std::abs(x);
}

double root(double x) {
    return std::sqrt(x);
}

double exp_val(double base, double exp) {
    return std::pow(base, exp);
}

int floor_val(double x) {
    return static_cast<int>(std::floor(x));
}

int ceiling_val(double x) {
    return static_cast<int>(std::ceil(x));
}

// Trigonometric
double sin_val(double x) {
    return std::sin(x);
}

double cos_val(double x) {
    return std::cos(x);
}

double tan_val(double x) {
    return std::tan(x);
}

double asin_val(double x) {
    return std::asin(x);
}

double acos_val(double x) {
    return std::acos(x);
}

// Utilities
double confine(double val, double min_val, double max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

double bridge(double a, double b, double t) {
    return a + (b - a) * t;
}

// Random
static std::random_device rd;
static std::mt19937 gen(rd());

double chaos_float() {
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen);
}

int chaos_int(int min_val, int max_val) {
    std::uniform_int_distribution<> dis(min_val, max_val);
    return dis(gen);
}

void* chaos_pick(void* array, size_t size) {
    if (size == 0) return nullptr;
    void** arr = static_cast<void**>(array);
    return arr[chaos_int(0, size - 1)];
}

// ============================================================================
// 📝 Strings - The Lexicon Operations
// ============================================================================

// Info
size_t lex_span(const std::string& str) {
    return str.length();
}

bool lex_void(const std::string& str) {
    return str.empty();
}

char lex_glyph_at(const std::string& str, size_t index) {
    if (index >= str.length()) return '\0';
    return str[index];
}

// Modification
std::string lex_elevate(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = static_cast<char>(std::toupper(c));
    }
    return result;
}

std::string lex_depress(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(c));
    }
    return result;
}

std::string lex_sanitize(const std::string& str) {
    size_t start = 0;
    size_t end = str.length();
    while (start < end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n')) {
        start++;
    }
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\n')) {
        end--;
    }
    return str.substr(start, end - start);
}

std::string lex_swap(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = result.find(from);
    if (pos != std::string::npos) {
        result.replace(pos, from.length(), to);
    }
    return result;
}

// Search
bool lex_find(const std::string& str, const std::string& sub) {
    return str.find(sub) != std::string::npos;
}

bool lex_anchors_with(const std::string& str, const std::string& pref) {
    return str.rfind(pref, 0) == 0;
}

bool lex_trails_with(const std::string& str, const std::string& suff) {
    if (suff.length() > str.length()) return false;
    return str.compare(str.length() - suff.length(), suff.length(), suff) == 0;
}

// Conversion
int lex_parse_int(const std::string& str) {
    try {
        return std::stoi(str);
    } catch (...) {
        return 0;
    }
}

double lex_parse_float(const std::string& str) {
    try {
        return std::stod(str);
    } catch (...) {
        return 0.0;
    }
}

bool lex_parse_logic(const std::string& str) {
    std::string lower = lex_depress(str);
    return lower == "true" || lower == "1" || lower == "yes";
}

// Advanced
std::vector<std::string> lex_shatter(const std::string& str, const std::string& delim) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delim);
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    result.push_back(str.substr(start));
    return result;
}

std::string lex_weave(const std::vector<std::string>& parts, const std::string& sep) {
    std::string result;
    for (size_t i = 0; i < parts.size(); i++) {
        result += parts[i];
        if (i < parts.size() - 1) {
            result += sep;
        }
    }
    return result;
}

// ============================================================================
// 🛠️ Type Conversion
// ============================================================================

std::string to_text(int val) {
    return std::to_string(val);
}

std::string to_text(double val) {
    return std::to_string(val);
}

std::string to_text(bool val) {
    return val ? "true" : "false";
}

int to_integer(const std::string& str) {
    return lex_parse_int(str);
}

double to_number(const std::string& str) {
    return lex_parse_float(str);
}

} // namespace stdlib
} // namespace veldanava