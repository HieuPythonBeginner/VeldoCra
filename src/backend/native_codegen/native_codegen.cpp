/**
 * @file native_codegen.cpp
 * @brief Native codegen driver implementation
 */

#include "native_codegen.h"
#include <iostream>

#ifdef USE_LLVM
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCAsmBackend.h>
#include <llvm/MC/MCCodeEmitter.h>
#include <llvm/MC/MCObjectWriter.h>
#endif

namespace veldanava {
namespace native_codegen {

NativeCodeGen::NativeCodeGen(CodeGenTarget target, bool bare_metal)
    : bare_metal_(bare_metal) {
    // Select backend based on target
    switch (target) {
        case CodeGenTarget::WebAssembly:
            #ifdef USE_LLVM
                backend_ = std::make_unique<WebAssemblyBackend>();
            #else
                std::cerr << "WebAssembly target requires LLVM support (build with USE_LLVM)" << std::endl;
                backend_ = std::make_unique<AssemblerBackend>(); // Fallback
            #endif
            break;

        case CodeGenTarget::JavaScript:
            // JavaScript is handled by separate transpiler
            backend_ = nullptr;
            break;

        case CodeGenTarget::Native:
        default:
            // Select backend based on build configuration for native targets
            #ifdef USE_LLVM
                backend_ = std::make_unique<LLVMBackend>();
            #elif defined(USE_CRANELIFT)
                backend_ = std::make_unique<CraneliftBackend>();
            #else
                backend_ = std::make_unique<AssemblerBackend>();
            #endif
            break;
    }
}

std::vector<uint8_t> NativeCodeGen::generate_executable(const middle::ir::IRModule& module) {
    // Generate code using selected backend
    std::vector<uint8_t> code = backend_->generate_code(module);

    std::cout << "[NativeCodeGen] Generated " << code.size() << " bytes of machine code" << std::endl;

    if (code.empty()) {
        std::cerr << "Error: Backend generated no code!" << std::endl;
        return {};
    }

    // Setup ELF generator for executable
    elf_generator_ = ELFGenerator();

    // Set entry point (assume code starts at 0x400000)
    uint64_t entry_addr = 0x400000;
    elf_generator_.set_entry_point(entry_addr);

    // Add code section
    elf_generator_.add_code_section(code, ".text");
    std::cout << "[NativeCodeGen] Added code section to ELF" << std::endl;

    // Generate final ELF binary
    auto elf_binary = elf_generator_.generate();
    std::cout << "[NativeCodeGen] Generated ELF binary with " << elf_binary.size() << " bytes" << std::endl;

    return elf_binary;
}

// AssemblerBackend implementation
std::vector<uint8_t> AssemblerBackend::generate_code(const middle::ir::IRModule& module) {
    AssembledModule assembled = assembler_.assemble(module);

    std::vector<uint8_t> result;

    // Concatenate all function codes
    for (const auto& func : assembled.functions) {
        result.insert(result.end(), func.code.begin(), func.code.end());
    }

    // Add data section
    result.insert(result.end(), assembled.data_section.begin(), assembled.data_section.end());

    return result;
}

// LLVMBackend implementation
LLVMBackend::LLVMBackend() {
#ifdef USE_LLVM
    // Initialize LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
#endif
}

std::vector<uint8_t> LLVMBackend::generate_code(const middle::ir::IRModule& module) {
#ifdef USE_LLVM
    // Create LLVM module
    auto context = std::make_unique<llvm::LLVMContext>();
    auto llvm_module = std::make_unique<llvm::Module>("veldanava_module", *context);

    // TODO: Convert VeldoCra IR to LLVM IR

    // For now, create a simple function
    llvm::IRBuilder<> builder(*context);
    auto func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), false);
    auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "main", llvm_module.get());

    auto entry = llvm::BasicBlock::Create(*context, "entry", func);
    builder.SetInsertPoint(entry);
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 42));

    // Generate machine code
    // TODO: Implement proper code generation

    return {}; // Placeholder
#else
    return {};
#endif
}

// CraneliftBackend implementation
CraneliftBackend::CraneliftBackend() {
    // TODO: Initialize Cranelift
}

std::vector<uint8_t> CraneliftBackend::generate_code(const middle::ir::IRModule& module) {
    // TODO: Implement Cranelift code generation
    // For now, return empty
    return {};
}

// WebAssemblyBackend implementation
WebAssemblyBackend::WebAssemblyBackend() {
#ifdef USE_LLVM
    // Initialize WebAssembly target instead of native
    LLVMInitializeWebAssemblyTarget();
    LLVMInitializeWebAssemblyTargetMC();
    LLVMInitializeWebAssemblyAsmPrinter();
    LLVMInitializeWebAssemblyAsmParser();
#endif
}

std::vector<uint8_t> WebAssemblyBackend::generate_code(const middle::ir::IRModule& module) {
#ifdef USE_LLVM
    // Create LLVM module with WebAssembly target
    auto context = std::make_unique<llvm::LLVMContext>();
    auto llvm_module = std::make_unique<llvm::Module>("veldanava_wasm", *context);

    // Set target triple for WebAssembly
    llvm_module->setTargetTriple("wasm32-unknown-wasi");

    // TODO: Convert VeldoCra IR to LLVM IR
    // For now, create a simple function that returns 42
    llvm::IRBuilder<> builder(*context);
    auto func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), false);
    auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "main", llvm_module.get());

    auto entry = llvm::BasicBlock::Create(*context, "entry", func);
    builder.SetInsertPoint(entry);
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 42));

    // Generate WebAssembly code
    std::string error;
    llvm::raw_string_ostream error_stream(error);

    // Create target machine for WebAssembly
    auto target = llvm::TargetRegistry::lookupTarget("wasm32-unknown-wasi", error_stream);
    if (!target) {
        std::cerr << "WebAssembly target not found: " << error << std::endl;
        return {};
    }

    llvm::TargetOptions options;
    auto target_machine = target->createTargetMachine("wasm32-unknown-wasi", "", "", options,
                                                     llvm::Optional<llvm::Reloc::Model>(),
                                                     llvm::Optional<llvm::CodeModel::Model>(),
                                                     llvm::CodeGenOpt::Default);

    // Emit WebAssembly object file
    llvm::SmallString<4096> code_buffer;
    llvm::raw_svector_ostream code_stream(code_buffer);

    llvm::legacy::PassManager pass_manager;
    if (target_machine->addPassesToEmitFile(pass_manager, code_stream,
                                           nullptr, // DWO file
                                           llvm::CGFT_ObjectFile)) {
        std::cerr << "Cannot emit WebAssembly object file" << std::endl;
        return {};
    }

    pass_manager.run(*llvm_module);

    // Convert to vector<uint8_t>
    std::vector<uint8_t> result(code_buffer.begin(), code_buffer.end());
    return result;

#else
    std::cerr << "WebAssembly backend requires LLVM support (build with USE_LLVM)" << std::endl;
    return {};
#endif
}

std::vector<uint8_t> NativeCodeGen::generate_object(const middle::ir::IRModule& module) {
    // For object files, we would create relocatable ELF
    // This is more complex and requires relocation entries
    // For now, just generate executable
    return generate_executable(module);
}

void NativeCodeGen::setup_executable_layout() {
    // Setup standard executable layout
    // This would include:
    // - Program headers for code and data segments
    // - Proper section alignment
    // - Entry point setup
}

void NativeCodeGen::add_startup_code() {
    if (bare_metal_) {
        // Bare metal startup code - x86_64 assembly
        // _start function that sets up environment and calls main

        // Assembly code (as bytes):
        // push rbp
        // mov rbp, rsp
        // sub rsp, 32  ; allocate stack space

        // Initialize allocator (assume memory starts at 0x100000)
        // mov rdi, 0x100000  ; heap start
        // mov rsi, 0x100000  ; heap size (1MB)
        // call init_allocator

        // Call main
        // call main

        // Exit
        // mov rax, 60  ; sys_exit
        // xor rdi, rdi ; exit code 0
        // syscall

        std::vector<uint8_t> startup_code = {
            0x55,                    // push rbp
            0x48, 0x89, 0xe5,       // mov rbp, rsp
            0x48, 0x83, 0xec, 0x20, // sub rsp, 32

            // Initialize allocator
            0x48, 0xbf, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rdi, 0x100000
            0x48, 0xbe, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rsi, 0x100000
            0xe8, 0x00, 0x00, 0x00, 0x00, // call init_allocator (placeholder offset)

            // Call main
            0xe8, 0x00, 0x00, 0x00, 0x00, // call main (placeholder offset)

            // Exit
            0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, // mov rax, 60
            0x48, 0x31, 0xff,             // xor rdi, rdi
            0x0f, 0x05,                   // syscall
        };

        // Add startup code to ELF generator
        elf_generator_.add_code_section(startup_code, ".startup");
        elf_generator_.set_entry_point(0x400000); // _start entry point
    } else {
        // Standard CRT startup code
        // This would include:
        // - _start function
        // - Stack setup
        // - Calling main()
        // - Exit syscall
    }
}

void NativeCodeGen::add_standard_library() {
    if (bare_metal_) {
        // Include bare metal stdlib implementations
        // These would be compiled and linked into the binary
        // For now, assume they're available as external functions
    } else {
        // Link against standard library functions
        // For minimal executables, we can implement syscalls directly
    }
}

} // namespace native_codegen
} // namespace veldanava