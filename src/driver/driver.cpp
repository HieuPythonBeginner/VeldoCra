/**
 * @file driver.cpp
 * @brief VeldoCra Compiler Driver - Implementation
 * @author Dr. Bright
 */

#include "driver/driver.h"
#include "frontend/lexer/lexer.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/ast.h"
#include "backend/vm/vm.h"
#include "backend/vm/instruction.h"
#include "middle/ownership/ownership.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

namespace veldocra {

Driver::Driver() = default;
Driver::~Driver() = default;

/**
 * @brief Load source file into memory
 */
Result<std::vector<char>> Driver::load_source(std::string_view path) {
    std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        return Result<std::vector<char>>::fail(
            std::string("Cannot open file: ") + std::string(path));
    }
    
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        return Result<std::vector<char>>::fail(
            std::string("Failed to read file: ") + std::string(path));
    }
    
    return Result<std::vector<char>>::ok(std::move(buffer));
}

/**
 * @brief Run lexer phase - returns TokenStream
 */
veldocra::lexer::TokenStream Driver::run_lexer(std::string_view source_str) {
    lexer::Lexer lexer(source_str, "<input>");
    return lexer.tokenize();
}

/**
 * @brief Run parser phase - returns AST
 */
veldocra::ast::Node* Driver::run_parser_ast(veldocra::lexer::TokenStream& tokens, std::string_view source, parser::Parser& parser) {
    // Parser is passed in from caller - keep it alive through ownership check
    auto result = parser.parse();
    
    if (!result.success) {
        std::cerr << "Parse error: " << result.error_message 
                  << " at line " << result.error_line 
                  << ", col " << result.error_column << "\n";
        return nullptr;
    }
    
    return result.root;
}

/**
 * @brief Simple codegen to VM - generates VM instructions from AST
 */
vm::Program Driver::generate_code(veldocra::ast::Node* root) {
    vm::Program program;
    
    (void)root;
    
    // Simple code generation - print "Hello, VeldoCra!"
    size_t str_idx = program.add_string("Hello, VeldoCra!\n");
    program.add(vm::Opcode::MOVI, 0, static_cast<int32_t>(str_idx));
    program.add(vm::Opcode::PRINTS, 0);
    program.add(vm::Opcode::HLT);
    
    return program;
}

/**
 * @brief Run lexer phase (legacy - returns string)
 */
Result<std::string> Driver::run_lexer(const std::vector<char>& source) {
    if (verbose_) {
        std::cout << "Running lexer...\n";
    }
    
    std::string_view source_str(source.data(), source.size());
    auto tokens = run_lexer(source_str);
    
    if (verbose_) {
        std::cout << "Lexed " << tokens.size() << " tokens\n";
    }
    
    return Result<std::string>::ok("Lexical analysis complete");
}

/**
 * @brief Run parser phase
 */
Result<std::string> Driver::run_parser(const std::string& tokens) {
    (void)tokens;
    return Result<std::string>::ok("Parsing complete");
}

/**
 * @brief Run semantic analysis phase (placeholder)
 */
Result<std::string> Driver::run_semantic(const std::string& ast) {
    (void)ast;
    if (verbose_) {
        std::cout << "Running semantic analysis...\n";
    }
    return Result<std::string>::ok("Semantic analysis complete");
}

/**
 * @brief Run code generation phase
 */
Result<std::string> Driver::run_codegen(const std::string& ir) {
    (void)ir;
    if (verbose_) {
        std::cout << "Running code generation...\n";
    }
    return Result<std::string>::ok("Code generation complete");
}

/**
 * @brief Run ownership analysis phase
 * ACTIVE: Real ownership checking enabled
 */
bool Driver::run_ownership_check(ast::Node* ast_root) {
    std::cerr << "[DEBUG] run_ownership_check called with ast_root = " << ast_root << "\n";
    
    if (!ast_root) {
        std::cerr << "[Ownership] Warning: ast_root is nullptr, skipping ownership check\n";
        return true;
    }
    
    // ACTIVATE: Real ownership checking
    std::cerr << "[Ownership] ACTIVATED: Running ownership analysis...\n";
    std::cerr << "[DEBUG] About to create OwnershipChecker\n";
    
    ownership::OwnershipChecker checker;
    checker.set_verbose(verbose_);
    checker.check(ast_root);
    
    if (checker.has_errors()) {
        std::cerr << "[Ownership] ERRORS FOUND:\n";
        for (const auto& err : checker.get_errors()) {
            std::cerr << "  - " << err.to_string() << "\n";
        }
        return false;
    }
    
    std::cerr << "[Ownership] PASSED: No ownership violations found\n";
    return true;
}

/**
 * @brief Execute a compiler command
 */
Result<int> Driver::execute(Command cmd, std::string_view input_path,
                           std::string_view output_path) {
    (void)output_path;
    
    // Print banner
    std::cout << "VeldoCra Compiler v" << version() << "\n";
    
    // Check file extension
    std::string path_str(input_path);
    if (path_str.size() < 4 || path_str.substr(path_str.size() - 4) != ".vel") {
        return Result<int>::fail(
            "Input file must have .vel extension");
    }
    
    std::cout << "Processing " << input_path << "...\n";
    
    // Phase 1: Load source
    auto source_result = load_source(input_path);
    if (!source_result.success) {
        return Result<int>::fail(source_result.error);
    }
    
    std::string_view source_str(source_result.value.data(), source_result.value.size());
    
    // Phase 2: Lex
    std::cerr << "[DEBUG] About to run lexer...\n";
    if (verbose_) {
        std::cout << "Running lexer...\n";
    }
    auto tokens = run_lexer(source_str);
    std::cerr << "[DEBUG] Lexer done, token count = " << tokens.size() << "\n";
    if (tokens.empty()) {
        return Result<int>::fail("No tokens produced");
    }
    if (verbose_) {
        std::cout << "Lexed " << tokens.size() << " tokens\n";
    }
    
    // Phase 3: Parse
    std::cerr << "[DEBUG] About to run parser...\n";
    if (verbose_) {
        std::cout << "Running parser...\n";
    }
    // Create parser here - must stay alive through ownership check
    parser::Parser parser(tokens, source_str);
    auto* ast_root = run_parser_ast(tokens, source_str, parser);
    std::cerr << "[DEBUG] Parser done, ast_root = " << ast_root << "\n";
    if (!ast_root) {
        return Result<int>::fail("Parsing failed");
    }
    if (verbose_) {
        std::cout << "Parsed AST successfully\n";
    }
    
    // Phase 3.5: Ownership Analysis
    std::cerr << "[DEBUG] About to run ownership check...\n";
    if (!run_ownership_check(ast_root)) {
        return Result<int>::fail("Ownership analysis failed - see errors above");
    }
    
    // For 'check' command, stop here
    if (cmd == Command::Check) {
        std::cout << "Check complete - no errors found.\n";
        return Result<int>::ok(0);
    }
    
    // Phase 4: Codegen to VM
    if (verbose_) {
        std::cout << "Generating VM code...\n";
    }
    auto program = generate_code(ast_root);
    if (verbose_) {
        std::cout << "Generated " << program.size() << " instructions\n";
    }
    
    // Phase 5: Execute with VM
    if (verbose_ || cmd == Command::Run) {
        std::cout << "Executing...\n";
    }
    
    vm::VM vmachine(16);
    vmachine.set_debug(debug_);
    vmachine.load(&program);
    int result = vmachine.execute();
    
    if (!vmachine.get_error().empty()) {
        std::cerr << "VM Error: " << vmachine.get_error() << "\n";
        return Result<int>::fail(vmachine.get_error());
    }
    
    if (verbose_) {
        std::cout << "Execution complete (exit code: " << result << ")\n";
    }
    
    std::cout << "\nSuccess.\n";
    return Result<int>::ok(0);
}

} // namespace veldocra

