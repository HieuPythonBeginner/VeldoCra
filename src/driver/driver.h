/**
 * @file driver.h
 * @brief VeldoCra Compiler Driver - Header
 * @author Dr. Bright
 * 
 * The driver coordinates all compiler phases:
 * - Lexer
 * - Parser
 * - AST
 * - Semantic Analysis
 * - IR Generation
 * - Code Generation (VM or Native)
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>

// Forward declarations
namespace veldocra {
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

// Forward declare ownership in veldocra namespace
namespace veldocra {

/**
 * @brief Compiler command modes
 */
enum class Command {
    Build,  // Compile to native binary
    Run,    // Run via VM
    Check   // Static analysis only
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
     * @param input_path Path to input .vel file
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
    static constexpr std::string_view name() { return "VeldoCra"; }
    
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
    Result<std::string> run_lexer(const std::vector<char>& source);
    lexer::TokenStream run_lexer(std::string_view source);
    ast::Node* run_parser_ast(lexer::TokenStream& tokens, std::string_view source, parser::Parser& parser);
    vm::Program generate_code(ast::Node* root);
    Result<std::string> run_parser(const std::string& tokens);
    Result<std::string> run_semantic(const std::string& ast);
    Result<std::string> run_codegen(const std::string& ir);
    bool run_ownership_check(ast::Node* ast_root);
    
    // Internal state - Parser kept as member to prevent AST from being freed
    bool verbose_ = false;
    bool debug_ = false;
    
    // IMPORTANT: Keep parser alive to preserve the AST arena
    // The parser owns the ASTBuilder which allocates the AST nodes
    std::unique_ptr<parser::Parser> parser_;
    
    // Store parsed AST root for ownership checking
    ast::Node* ast_root_ = nullptr;
};

} // namespace veldocra
