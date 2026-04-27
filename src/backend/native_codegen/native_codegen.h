/**
 * @file native_codegen.h
 * @brief Native codegen driver - converts IR to ELF executable
 */

#ifndef VELDOCRA_NATIVE_CODEGEN_H
#define VELDOCRA_NATIVE_CODEGEN_H

#include "assembler.h"
#include "elf.h"
#include <vector>
#include <string>
#include <memory>

namespace veldanava {
namespace middle {
namespace ir {
    struct IRModule;
} // namespace ir
} // namespace middle

namespace native_codegen {

// Backend interface for code generation
class CodeGenBackend {
public:
    virtual ~CodeGenBackend() = default;
    virtual std::vector<uint8_t> generate_code(const middle::ir::IRModule& module) = 0;
};

// Assembler backend (current implementation)
class AssemblerBackend : public CodeGenBackend {
public:
    std::vector<uint8_t> generate_code(const middle::ir::IRModule& module) override;
private:
    Assembler assembler_;
};

// LLVM backend
class LLVMBackend : public CodeGenBackend {
public:
    LLVMBackend();
    std::vector<uint8_t> generate_code(const middle::ir::IRModule& module) override;
};

// Cranelift backend (JIT-oriented)
class CraneliftBackend : public CodeGenBackend {
public:
    CraneliftBackend();
    std::vector<uint8_t> generate_code(const middle::ir::IRModule& module) override;
};

// WebAssembly backend using LLVM
class WebAssemblyBackend : public CodeGenBackend {
public:
    WebAssemblyBackend();
    std::vector<uint8_t> generate_code(const middle::ir::IRModule& module) override;
};

// Code generation target
enum class CodeGenTarget {
    Native,      // Native machine code (ELF)
    WebAssembly, // WebAssembly for web
    JavaScript   // JavaScript transpilation
};

// Native Code Generator - produces executables for different targets
class NativeCodeGen {
public:
    NativeCodeGen(CodeGenTarget target = CodeGenTarget::Native, bool bare_metal = false);

    // Main codegen function
    std::vector<uint8_t> generate_executable(const middle::ir::IRModule& module);

    // Generate object file (for linking)
    std::vector<uint8_t> generate_object(const middle::ir::IRModule& module);

private:
    std::unique_ptr<CodeGenBackend> backend_;
    ELFGenerator elf_generator_;
    bool bare_metal_;

    // Helper functions
    void setup_executable_layout();
    void add_startup_code();
    void add_standard_library();
};

} // namespace native_codegen
} // namespace veldanava

#endif // VELDOCRA_NATIVE_CODEGEN_H