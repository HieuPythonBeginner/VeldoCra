/**
 * @file source.cpp
 * @brief VeldoCra Source File Management Implementation
 * @author Dr. Bright
 */

#include "frontend/source/source.h"
#include <fstream>
#include <algorithm>
#include <memory>

namespace veldocra {
namespace source {

/**
 * @brief Load a source file from disk
 */
SourceOpResult<SourceFile> SourceManager::load_file(std::string_view path) {
    std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        return SourceOpResult<SourceFile>::fail(
            std::string("Cannot open file: ") + std::string(path), path);
    }
    
    auto size = file.tellg();
    if (size < 0) {
        return SourceOpResult<SourceFile>::fail(
            "Failed to get file size", path);
    }
    
    file.seekg(0, std::ios::beg);
    
    // Allocate buffer and read
    auto* buffer = new char[static_cast<size_t>(size) + 1];
    if (!file.read(buffer, size)) {
        delete[] buffer;
        return SourceOpResult<SourceFile>::fail(
            std::string("Failed to read file: ") + std::string(path), path);
    }
    
    // Null-terminate
    buffer[static_cast<size_t>(size)] = '\0';
    
    // Copy filename
    std::string filename(path);
    
    // Create source file
    SourceFile sf;
    sf.data = buffer;
    sf.size = static_cast<size_t>(size);
    sf.filename = filename.c_str();
    
    // Build line offset table
    build_line_offsets(sf);
    
    // Store in loaded files
    loaded_files_.push_back(sf);
    
    return SourceOpResult<SourceFile>::ok(sf, path);
}

/**
 * @brief Build line offset table for fast location lookup
 */
void SourceManager::build_line_offsets(const SourceFile& file) {
    line_offsets_.clear();
    line_offsets_.push_back(0);  // First line starts at offset 0
    
    for (size_t i = 0; i < file.size; ++i) {
        if (file.data[i] == '\n') {
            line_offsets_.push_back(static_cast<uint32_t>(i + 1));
        }
    }
}

/**
 * @brief Get source location from byte offset
 */
SourceLocation SourceManager::get_location(const SourceFile& file, uint32_t offset) const {
    if (offset > file.size) {
        offset = static_cast<uint32_t>(file.size);
    }
    
    // Binary search for line
    uint32_t line = 1;
    uint32_t left = 0;
    uint32_t right = static_cast<uint32_t>(line_offsets_.size());
    
    while (left < right) {
        uint32_t mid = left + (right - left) / 2;
        if (line_offsets_[mid] <= offset) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    line = left;
    
    // Calculate column
    uint32_t line_start = (line > 1) ? line_offsets_[line - 2] : 0;
    uint32_t column = offset - line_start + 1;
    
    return SourceLocation(offset, line, column);
}

/**
 * @brief Get a substring of the source
 */
std::string_view SourceManager::get_span(const SourceFile& file, 
                                         uint32_t start, 
                                         uint32_t length) const {
    if (start >= file.size) {
        return {};
    }
    
    uint32_t actual_length = length;
    if (start + length > file.size) {
        actual_length = static_cast<uint32_t>(file.size) - start;
    }
    
    return std::string_view(file.data + start, actual_length);
}

} // namespace source
} // namespace veldocra

