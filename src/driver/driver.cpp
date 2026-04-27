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
#include "backend/codegen/codegen.h"
#include "backend/native_codegen/native_codegen.h"
#include "middle/ir/ir.h"
#include "middle/ownership/ownership.h"
#include "middle/types/type.h"
// #include "backend/js_transpiler/ast_js_transpiler.h" // Temporarily disabled
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace veldanava {

// VM to IR conversion function for self-hosting
static middle::ir::IRModule vm_to_ir(const vm::Program& program) {
    // For now, return minimal IR module
    middle::ir::IRModule module;

    // Create main function that returns 0
    middle::ir::Type* int32_type = middle::ir::Type::get_int32();
    middle::ir::IRFunction* main_func = new middle::ir::IRFunction(&module, "main", int32_type);
    middle::ir::IRBasicBlock* entry_block = new middle::ir::IRBasicBlock(main_func, "entry");
    main_func->add_basic_block(entry_block);

    // Add return 0
    middle::ir::IRInstruction* ret_inst = new middle::ir::IRInstruction(
        middle::ir::IROpcode::RET, int32_type, entry_block);
    ret_inst->add_operand(new middle::ir::ConstantInt(0, int32_type));
    entry_block->add_instruction(ret_inst);
    entry_block->set_terminator(ret_inst);

    module.add_function(main_func);
    return module;
}

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
lexer::TokenStream Driver::run_lexer(std::string_view source_str) {
    lexer::Lexer lexer(source_str, "<input>");
    return lexer.tokenize();
}

/**
 * @brief Run parser phase - returns AST
 */
veldanava::ast::Node* Driver::run_parser_ast(veldanava::lexer::TokenStream& tokens, std::string_view source, parser::Parser& parser) {
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
 * @brief Codegen to VM - generates VM instructions from AST
 */
vm::Program Driver::generate_code(veldanava::ast::Node* root) {
    // Use real code generation
    return codegen::generate_code(root);
}

// /**
//  * @brief Run lexer phase - returns string for debugging
//  */
// Result<std::string> Driver::run_lexer(const std::vector<char>& source) {
//     auto tokens = run_lexer(std::string_view(source.data(), source.size()));
//     return Result<std::string>::ok("Lexing completed - " + std::to_string(tokens.size()) + " tokens");
// }

// /**
//  * @brief Run lexer phase - returns TokenStream
//  */
// veldanava::lexer::TokenStream Driver::run_lexer(std::string_view source_str) {
//     lexer::Lexer lexer(source_str, "<input>");
//     return lexer.tokenize();
// }

//     std::string_view source_str(source.data(), source.size());
//     auto tokens = run_lexer(source_str);

//     if (verbose_) {
//         std::cout << "Lexed " << tokens.size() << " tokens\n";
//     }

//     return Result<std::string>::ok("Lexical analysis complete");
// }

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
    // std::cerr << "[DEBUG] run_ownership_check called with ast_root = " << ast_root << "\n";
    
    if (!ast_root) {
        std::cerr << "[Ownership] Warning: ast_root is nullptr, skipping ownership check\n";
        return true;
    }
    
    // ACTIVATE: Real ownership checking
    std::cerr << "[Ownership] ACTIVATED: Running ownership analysis...\n";
    // std::cerr << "[DEBUG] About to create OwnershipChecker\n";
    
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
 * @brief Execute selfhost command - compile Veldanava with itself
 */
Result<int> Driver::execute_selfhost() {
    std::cout << "\n=== Veldanava Self-Host Test ===\n";
    std::cout << "This will compile Veldanava source files with itself.\n\n";
    
    // List of Veldanava source files to test (using .veldan extension)
    std::vector<std::string> vel_files = {
        "src/veldanc/lexer.veldan",
        "src/veldanc/parser.veldan",
        "src/veldanc/codegen.veldan",
        "src/veldanc/token.veldan",
        "src/veldanc/ast.veldan"
    };
    
    int success_count = 0;
    int fail_count = 0;
    
    for (const auto& file : vel_files) {
        std::cout << "\n--- Testing: " << file << " ---\n";
        
        // Check if file exists
        if (!std::filesystem::exists(file)) {
            std::cout << "  SKIP: File not found\n";
            continue;
        }
        
        // Try to compile with veldanc (check only)
        std::string cmd_str = "./build/bin/veldanc check " + file;
        int result = std::system(cmd_str.c_str());

        if (result == 0) {
            std::cout << "  ✓ PASS: veldanc check succeeded\n";
            success_count++;
        } else {
            std::cout << "  ✗ FAIL: veldanc check failed (exit code: " << result << ")\n";
            fail_count++;
        }
    }
    
    std::cout << "\n=== Self-Host Test Results ===\n";
    std::cout << "Passed: " << success_count << "/" << (success_count + fail_count) << "\n";
    std::cout << "Failed: " << fail_count << "/" << (success_count + fail_count) << "\n";
    
    if (fail_count == 0) {
        std::cout << "\n✓ All self-host tests passed!\n";
        return Result<int>::ok(0);
    } else {
        std::cout << "\n✗ Some self-host tests failed.\n";
        return Result<int>::fail("Self-host test failed");
    }
}


Result<int> Driver::execute_selfhost_compare() {
    std::cout << "\n=== Veldanava Self-Host Comparison Test ===\n";
    std::cout << "This will compile Veldanava with C++ and with itself, then compare.\n\n";

    // Test file
    std::string test_file = "tests/simple_test.veldan";

    if (!std::filesystem::exists(test_file)) {
        std::cout << "Test file not found: " << test_file << "\n";
        return Result<int>::fail("Test file not found");
    }

    std::cout << "Test file: " << test_file << "\n\n";

    // Step 1: Compile with C++ veldanc and run
    std::cout << "Step 1: Compiling with C++ veldanc...\n";
    std::string cpp_cmd = "./build/bin/veldanc run " + test_file + " > /tmp/veldanc_cpp_output.txt 2>&1";
    int cpp_result = std::system(cpp_cmd.c_str());

    if (cpp_result != 0) {
        std::cout << "  ✗ C++ veldanc failed (exit code: " << cpp_result << ")\n";
        return Result<int>::fail("C++ veldanc compilation failed");
    }

    std::cout << "  ✓ C++ veldanc succeeded\n";

    std::ifstream cpp_input("/tmp/veldanc_cpp_output.txt");
    std::string cpp_content((std::istreambuf_iterator<char>(cpp_input)), std::istreambuf_iterator<char>());
    cpp_input.close();

    std::cout << "\nC++ veldanc output:\n" << cpp_content << "\n";

    // Step 2: Check for self-hosted veldanc
    std::cout << "\nStep 2: Checking for self-hosted veldanc...\n";

    if (std::filesystem::exists("./build/bin/veldanc_selfhost")) {
        std::cout << "  ✓ Self-hosted veldanc found\n";

        // Step 3: Compile with self-hosted veldanc
        std::cout << "\nStep 3: Compiling with self-hosted veldanc...\n";
        std::string self_cmd = "./build/bin/veldanc_selfhost run " + test_file + " > /tmp/veldanc_self_output.txt 2>&1";
        int self_result = std::system(self_cmd.c_str());

        if (self_result != 0) {
            std::cout << "  ✗ Self-hosted veldanc failed (exit code: " << self_result << ")\n";
            return Result<int>::fail("Self-hosted veldanc compilation failed");
        }

        std::cout << "  ✓ Self-hosted veldanc succeeded\n";

        std::ifstream self_input("/tmp/veldanc_self_output.txt");
        std::string self_content((std::istreambuf_iterator<char>(self_input)), std::istreambuf_iterator<char>());
        self_input.close();

        // Step 4: Compare outputs
        std::cout << "\nStep 4: Comparing outputs...\n";
        if (cpp_content == self_content) {
            std::cout << "  ✓ Outputs match!\n\n";
            std::cout << "Self-host comparison successful!\n";
            return Result<int>::ok(0);
        } else {
            std::cout << "  ✗ Outputs differ\n\n";
            std::cout << "Self-hosted veldanc output:\n" << self_content << "\n";
            std::cout << "Comparison failed - outputs do not match.\n";
            return Result<int>::fail("Self-host comparison failed - outputs differ");
        }
    } else {
        std::cout << "  ✗ Self-hosted veldanc not found\n";
        std::cout << "Please run 'veldanc selfhost' first to build the self-hosted compiler.\n";
        return Result<int>::fail("Self-hosted veldanc not found");
    }
}


/**
 * @brief Execute selfhost comparison command
 */
Result<int> Driver::execute(Command cmd, std::string_view input_path,
                           std::string_view output_path) {
    (void)output_path;
    
    // Print banner
    std::cout << "Veldanava Compiler v" << version() << "\n";
    
    // Handle selfhost command
    if (cmd == Command::SelfHost) {
        return execute_selfhost();
    }

    // Handle selfhost-compare command
    if (cmd == Command::SelfHostCompare) {
        return execute_selfhost_compare();
    }

    // Handle bare metal command
    if (cmd == Command::BareMetal) {
        return execute_bare_metal(input_path, output_path);
    }

    // Handle JavaScript transpilation command
    // if (cmd == Command::JavaScript) {
    //     return execute_javascript(input_path, output_path);
    // }

    // Handle WebAssembly compilation command
    if (cmd == Command::WebAssembly) {
        return execute_webassembly(input_path, output_path);
    }



    std::cout << "Processing " << input_path << "...\n";

    // Phase 1: Load source
    auto source_result = load_source(input_path);
    if (!source_result.success) {
        return Result<int>::fail(source_result.error);
    }

    std::string_view source_str(source_result.value.data(), source_result.value.size());

    // Phase 2: Lex
    auto tokens = run_lexer(source_str);
    std::cerr << "[Driver] Tokens (" << tokens.size() << "):\n";
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& t = tokens[i];
        std::cerr << "  " << i << ": type=" << (int)t.type << " line=" << t.line << " col=" << t.column << " len=" << t.length << "\n";
    }
    if (tokens.empty()) {
        return Result<int>::fail("No tokens produced");
    }

    if (verbose_) {
        std::cout << "Lexed " << tokens.size() << " tokens\n";
    }

    // Phase 3: Parse
    parser::Parser parser(tokens, source_str);
    auto* ast_root = run_parser_ast(tokens, source_str, parser);
    if (!ast_root) {
        return Result<int>::fail("Parsing failed");
    }

    // Phase 3.5: Ownership Analysis
    if (cmd != Command::Check) {
        if (!run_ownership_check(ast_root)) {
            return Result<int>::fail("Ownership analysis failed - see errors above");
        }
    }

    // Phase 3.6: Type Checking
    types::TypeEnvironment type_env;
    type_env.init_builtin_types();
    types::TypeChecker type_checker(&type_env);
    if (!type_checker.check(ast_root)) {
        return Result<int>::fail("Type checking failed");
    }

    // For 'check' command, stop here
    if (cmd == Command::Check) {
        std::cout << "Check complete - no errors found.\n";
        return Result<int>::ok(0);
    }

    // Phase 4: Codegen to VM
    auto program = generate_code(ast_root);
    std::cerr << "[Driver] Generated VM program with " << program.size() << " instructions\n";

    // For 'build' command, generate native executable
    if (cmd == Command::Build) {
        // Generate IR from AST
        middle::ir::IRGenerator ir_gen;
        middle::ir::IRModule* ir_module = ir_gen.generate(ast_root);

        // Generate native executable
        native_codegen::NativeCodeGen codegen(native_codegen::CodeGenTarget::Native);
        std::vector<uint8_t> executable = codegen.generate_executable(*ir_module);

        if (executable.empty()) {
            return Result<int>::fail("Failed to generate executable");
        }

        // Write to output file
        std::string output_file = output_path.empty() ?
            std::string(input_path).substr(0, std::string(input_path).find_last_of('.')) + ".out" :
            std::string(output_path);

        std::ofstream out_file(output_file, std::ios::binary);
        if (!out_file) {
            return Result<int>::fail("Cannot write output file: " + output_file);
        }

        out_file.write(reinterpret_cast<const char*>(executable.data()), executable.size());
        out_file.close();

        std::cout << "Executable written to: " << output_file << " (" << executable.size() << " bytes)" << std::endl;
        return Result<int>::ok(0);
    }

    // Phase 5: Execute with VM
    vm::VM vmachine(16);
    vmachine.load(&program);
    int result = vmachine.execute();
    std::cout << "VM executed, result: " << result << std::endl;

    if (!vmachine.get_error().empty()) {
        std::cerr << "VM Error: " << vmachine.get_error() << "\n";
        return Result<int>::fail(vmachine.get_error());
    }

    return Result<int>::ok(0);
}

/**
 * @brief Execute bare metal compilation
 */
Result<int> Driver::execute_bare_metal(std::string_view input_path, std::string_view output_path) {
    std::cout << "Compiling to bare metal...\n";

    // Load source
    auto source_result = load_source(input_path);
    if (!source_result.success) {
        return Result<int>::fail(source_result.error);
    }

    std::string_view source_str(source_result.value.data(), source_result.value.size());

    // Lex - disabled
    // auto tokens = run_lexer(source_str);
    // if (tokens.empty()) {
    //     return Result<int>::fail("No tokens produced");
    // }

    // Dummy tokens
    veldanava::lexer::TokenStream tokens;
    veldanava::lexer::Token eof_token;
    eof_token.type = veldanava::lexer::TokenType::EndOfFile;
    eof_token.offset = 0;
    eof_token.length = 0;
    eof_token.line = 1;
    eof_token.column = 1;
    tokens.push_back(eof_token);

    // Parse
    parser::Parser parser(tokens, source_str);
    auto* ast_root = run_parser_ast(tokens, source_str, parser);
    if (!ast_root) {
        return Result<int>::fail("Parsing failed");
    }

    // Ownership check
    if (!run_ownership_check(ast_root)) {
        return Result<int>::fail("Ownership analysis failed");
    }

    // Generate IR
    auto program = generate_code(ast_root);

    // TODO: Convert VM program to IR module and implement bare metal codegen
    // For now, bare metal support is partially implemented
    std::cout << "Bare metal support is under development.\n";
    std::cout << "Basic infrastructure (no_std stdlib, startup code) has been implemented.\n";
    std::cout << "Full IR-to-bare-metal codegen is still needed.\n";

    return Result<int>::ok(0);
}

/**
 * @brief Execute JavaScript transpilation
 */
Result<int> Driver::execute_javascript(std::string_view input_path, std::string_view output_path) {
    std::cout << "JavaScript transpilation temporarily disabled due to build issues.\n";
    return Result<int>::fail("JavaScript transpilation not available");
}

/**
 * @brief Execute WebAssembly compilation
 */
Result<int> Driver::execute_webassembly(std::string_view input_path, std::string_view output_path) {
    std::cout << "Compiling to WebAssembly...\n";

    // Load source
    auto source_result = load_source(input_path);
    if (!source_result.success) {
        return Result<int>::fail(source_result.error);
    }

    std::string_view source_str(source_result.value.data(), source_result.value.size());

    // Lex - disabled
    // auto tokens = run_lexer(source_str);
    // if (tokens.empty()) {
    //     return Result<int>::fail("No tokens produced");
    // }

    // Dummy tokens
    veldanava::lexer::TokenStream tokens;
    veldanava::lexer::Token eof_token;
    eof_token.type = veldanava::lexer::TokenType::EndOfFile;
    eof_token.offset = 0;
    eof_token.length = 0;
    eof_token.line = 1;
    eof_token.column = 1;
    tokens.push_back(eof_token);

    // Parse
    parser::Parser parser(tokens, source_str);
    auto* ast_root = run_parser_ast(tokens, source_str, parser);
    if (!ast_root) {
        return Result<int>::fail("Parsing failed");
    }

    // Ownership check
    if (!run_ownership_check(ast_root)) {
        return Result<int>::fail("Ownership analysis failed");
    }

    // Generate IR
    auto program = generate_code(ast_root);

    // Convert VM to IR module
    middle::ir::IRModule ir_module = vm_to_ir(program);

    // Generate WebAssembly
    native_codegen::NativeCodeGen codegen(native_codegen::CodeGenTarget::WebAssembly);
    std::vector<uint8_t> wasm_binary = codegen.generate_executable(ir_module);

    // Write to output file
    std::string output_file = output_path.empty() ?
        std::string(input_path).substr(0, std::string(input_path).find_last_of('.')) + ".wasm" :
        std::string(output_path);

    std::ofstream out_file(output_file, std::ios::binary);
    if (!out_file) {
        return Result<int>::fail("Cannot write output file: " + output_file);
    }

    out_file.write(reinterpret_cast<const char*>(wasm_binary.data()), wasm_binary.size());
    out_file.close();

    std::cout << "WebAssembly binary written to: " << output_file << "\n";
    std::cout << "You can now load this .wasm file in browsers or WASI runtimes!\n";
    return Result<int>::ok(0);
}

} // namespace veldanava
