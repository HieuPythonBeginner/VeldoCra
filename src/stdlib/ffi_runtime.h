/**
 * @file ffi_runtime.h
 * @brief Foreign Function Interface Runtime Support
 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace veldanava {
namespace ffi {

// FFI function signature
using FFIFunction = void*;

// Language types for FFI
enum class FFILanguage {
    C,
    CPP,
    Python,
    Rust,
    // Add more as needed
};

// Function signature information
struct FFISignature {
    std::string name;
    FFILanguage language;
    std::string return_type;
    std::vector<std::string> param_types;
    std::string calling_convention; // "cdecl", "fastcall", etc.
};

// Loaded library handle
struct FFILibrary {
    void* handle;
    std::string path;
    std::unordered_map<std::string, FFIFunction> functions;
};

// Python FFI support
struct PythonContext {
    void* interpreter;  // PyThreadState or similar
    bool initialized = false;
};

// FFI Manager - handles loading libraries and functions
class FFIManager {
public:
    static FFIManager& instance();

    // Load a dynamic library
    bool load_library(const std::string& name, const std::string& path = "");

    // Python FFI support
    bool initialize_python();
    void finalize_python();

    // Universal Python function calling
    template<typename ReturnType, typename... Args>
    ReturnType call_python(const std::string& module, const std::string& func, Args... args);

    // String-based Python calling (for dynamic calls)
    std::string call_python_function(const std::string& module, const std::string& func,
                                   const std::vector<std::string>& args = {});

    // Import Python module
    bool import_python_module(const std::string& module_name);

    // Check if Python function exists
    bool python_function_exists(const std::string& module, const std::string& func);

private:
    // Helper methods for Python FFI
    std::string call_embedded_python(const std::string& module, const std::string& func,
                                   const std::vector<std::string>& args);
    std::string call_system_python(const std::string& module, const std::string& func,
                                 const std::vector<std::string>& args);
    std::string handle_colorama_fallback(const std::string& func);
    std::string escape_json_string(const std::string& str);

    // Get function from loaded library
    FFIFunction get_function(const std::string& lib_name, const std::string& func_name);

    // Register FFI function (for static linking)
    void register_function(const std::string& name, FFIFunction func, const FFISignature& sig);

    // Call FFI function with arguments
    template<typename ReturnType, typename... Args>
    ReturnType call_function(const std::string& lib_name, const std::string& func_name, Args... args);

private:
    FFIManager() = default;
    ~FFIManager();

    std::unordered_map<std::string, std::unique_ptr<FFILibrary>> libraries_;
    std::unordered_map<std::string, std::pair<FFIFunction, FFISignature>> static_functions_;
    PythonContext python_context_;

    // Platform-specific library loading
    void* load_library_platform(const std::string& path);
    void unload_library_platform(void* handle);
    FFIFunction get_symbol_platform(void* handle, const std::string& symbol);
};

// Template implementation for call_function
template<typename ReturnType, typename... Args>
ReturnType FFIManager::call_function(const std::string& lib_name, const std::string& func_name, Args... args) {
    FFIFunction func = get_function(lib_name, func_name);
    if (!func) {
        throw std::runtime_error("FFI function not found: " + lib_name + "::" + func_name);
    }

    // Cast and call based on calling convention
    // This is a simplified implementation - real FFI would need proper calling convention handling
    using FuncType = ReturnType(*)(Args...);
    FuncType typed_func = reinterpret_cast<FuncType>(func);
    return typed_func(args...);
}

} // namespace ffi
} // namespace veldanava