/**
 * @file source.h
 * @brief VeldoCra Source File Management
 * @author Dr. Bright
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace veldocra {
namespace source {

/**
 * @brief Result type for source operations (defined first)
 */
template<typename T>
struct SourceOpResult {
    bool success;
    T value;
    std::string error;
    std::string filename;
    
    static SourceOpResult<T> ok(T val, std::string_view fname = "") {
        return {true, std::move(val), "", std::string(fname)};
    }
    
    static SourceOpResult<T> fail(std::string err, std::string_view fname = "") {
        return {false, T{}, std::move(err), std::string(fname)};
    }
};

/**
 * @brief Represents a source file in memory
 * 
 * POD structure for cache-friendly access.
 * Maximum size: 32 bytes
 */
struct SourceFile {
    const char* data;      // Pointer to source data
    size_t size;           // Size in bytes
    const char* filename;  // Source filename
    
    // Helper methods
    constexpr bool is_valid() const { return data != nullptr && size > 0; }
    constexpr std::string_view view() const { return {data, size}; }
    constexpr std::string_view name() const { return filename ? filename : "<unknown>"; }
};

/**
 * @brief Source location for error reporting
 */
struct SourceLocation {
    uint32_t offset;  // Byte offset from start
    uint32_t line;    // 1-based line number
    uint32_t column;  // 1-based column number
    
    constexpr SourceLocation(uint32_t o = 0, uint32_t l = 1, uint32_t c = 1)
        : offset(o), line(l), column(c) {}
};

/**
 * @brief Source manager for loading and tracking source files
 */
class SourceManager {
public:
    SourceManager() = default;
    ~SourceManager() = default;
    
    // Disable copying, enable moving
    SourceManager(const SourceManager&) = delete;
    SourceManager& operator=(const SourceManager&) = delete;
    SourceManager(SourceManager&&) = default;
    SourceManager& operator=(SourceManager&&) = default;
    
    /**
     * @brief Load a source file from disk
     * @param path Path to the .vel source file
     * @return Result containing the SourceFile or error
     */
    SourceOpResult<SourceFile> load_file(std::string_view path);
    
    /**
     * @brief Get source location from byte offset
     * @param file Source file to query
     * @param offset Byte offset
     * @return Source location (line, column)
     */
    SourceLocation get_location(const SourceFile& file, uint32_t offset) const;
    
    /**
     * @brief Get a substring of the source
     * @param file Source file
     * @param start Start offset
     * @param length Number of characters
     * @return String_view of the substring
     */
    std::string_view get_span(const SourceFile& file, uint32_t start, uint32_t length) const;
    
private:
    std::vector<SourceFile> loaded_files_;
    
    // Pre-computed line offsets for fast location lookup
    std::vector<uint32_t> line_offsets_;
    
    /**
     * @brief Build line offset table
     */
    void build_line_offsets(const SourceFile& file);
};

} // namespace source
} // namespace veldocra

