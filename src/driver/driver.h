/**
 * @file driver.h
 * @brief Veldanava Compiler Driver - Header
 * @author Dr. Bright
 */

#pragma once

#include <vector>
#include <string_view>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <variant>
#include <functional>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "frontend/lexer/lexer.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/ast.h"
#include "backend/vm/vm.h"
#include "backend/vm/instruction.h"
#include "backend/codegen/codegen.h"
#include "backend/native_codegen/native_codegen.h"
#include "middle/ir/ir.h"
#include "middle/ownership/ownership.h"

namespace veldanava {
namespace lexer {
class TokenStream;
}
namespace ast {
struct Node;
class ASTBuilder;
}
namespace parser {
class Parser;
}
namespace vm {
class Program;
}
namespace ownership {
class OwnershipChecker;
}
}

// Forward declare ownership in veldanava namespace
namespace veldanava {

/**
 * @brief Compiler command modes
 */
enum class Command {
    Build,           // Compile to native binary
    Run,             // Run via VM
    Check,           // Static analysis only
    BareMetal,       // Compile to bare metal executable (no_std)
    JavaScript,      // Transpile to JavaScript for web/Node.js
    WebAssembly,     // Compile to WebAssembly for web performance
    SelfHost,        // Test if VeldoCra can compile its own source files
    SelfHostCompare  // Compare C++ and self-hosted compiler outputs
};

/**
 * @brief Result type for compiler operations
 */
template<typename T>
struct Result {
    bool success;
    T value;
    std::string error;
    
    static Result ok(T val) {
        return {true, std::move(val), ""};
    }
    
    static Result fail(std::string err) {
        return {false, T{}, std::move(err)};
    }
};

/**
 * @brief Compiler driver class
 * 
 * Orchestrates the compilation pipeline:
 * 1. Load source file
 * 2. Lexical analysis (Lexer)
 * 3. Parsing (Parser)
 * 4. Semantic analysis
 * 5. IR generation
 * 6. Code generation (VM or AOT)
 */
class Driver {
public:
    Driver();
    ~Driver();
    
    /**
     * @brief Execute a compiler command
     * @param cmd Command to execute
     * @param input_path Path to input .veldan file
     * @param output_path Path for output (optional)
     * @return Result of the operation
     */
    Result<int> execute(Command cmd, std::string_view input_path, 
                        std::string_view output_path = "");
    
    /**
     * @brief Get the compiler version
     */
    static constexpr std::string_view version() { return "0.0.1"; }
    
    /**
     * @brief Get the compiler name
     */
    static constexpr std::string_view name() { return "Veldanava"; }
    
    /**
     * @brief Enable verbose output
     */
    void set_verbose(bool verbose) { verbose_ = verbose; }
    
    /**
     * @brief Enable debug output
     */
    void set_debug(bool debug) { debug_ = debug; }

private:
    // Pipeline methods
    Result<std::vector<char>> load_source(std::string_view path);
    lexer::TokenStream run_lexer(std::string_view source);
    ast::Node* run_parser_ast(lexer::TokenStream& tokens, std::string_view source, parser::Parser& parser);
    vm::Program generate_code(ast::Node* root);
    Result<std::string> run_parser(const std::string& tokens);
    Result<std::string> run_semantic(const std::string& ast);
    Result<std::string> run_codegen(const std::string& ir);
    bool run_ownership_check(ast::Node* ast_root);
    Result<int> execute_selfhost();
    Result<int> execute_selfhost_compare();
    Result<int> execute_bare_metal(std::string_view input_path, std::string_view output_path);
    Result<int> execute_javascript(std::string_view input_path, std::string_view output_path);
    Result<int> execute_webassembly(std::string_view input_path, std::string_view output_path);
    
    // Internal state - Parser kept as member to prevent AST from being freed
    bool verbose_ = false;
    bool debug_ = false;
    
    // IMPORTANT: Keep parser alive to preserve the AST arena
    // The parser owns the ASTBuilder which allocates the AST nodes
    std::unique_ptr<parser::Parser> parser_;
    
    // Store parsed AST root for ownership checking
    ast::Node* ast_root_ = nullptr;
};

} // namespace veldanava
