/**
 * @file vm.h
 * @brief VeldoCra Register-Based Virtual Machine
 * @author Dr. Bright
 * 
 * Register-based VM with computed goto dispatch.
 * Supports basic arithmetic, control flow, and I/O.
 */

#pragma once

#include "instruction.h"
#include <stack>
#include <functional>
#include <string>

namespace veldocra {
namespace vm {

/**
 * @brief VeldoCra Register-Based Virtual Machine
 * 
 * Features:
 * - 16 general-purpose registers + 4 special registers
 * - Computed goto for fast instruction dispatch
 * - Function call support with call stack
 * - Stack for local variables
 */
class VM {
public:
    /**
     * @brief Construct VM
     * @param register_count Number of registers (default 16)
     */
    explicit VM(size_t register_count = 16);
    ~VM();
    
    // Disable copying
    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
    
    // Allow moving
    VM(VM&&) noexcept;
    VM& operator=(VM&&) noexcept;
    
    /**
     * @brief Load program into VM
     */
    void load(Program* program);
    
    /**
     * @brief Execute loaded program
     * @return Exit code
     */
    int execute();
    
    /**
     * @brief Execute from AST (convenience method)
     */
    int execute_from_ast(void* ast_root);
    
    /**
     * @brief Reset VM state
     */
    void reset();
    
    /**
     * @brief Get register value
     */
    Value get_register(uint8_t reg) const;
    
    /**
     * @brief Set register value
     */
    void set_register(uint8_t reg, const Value& val);
    
    /**
     * @brief Get program counter
     */
    size_t get_pc() const { return pc_; }
    
    /**
     * @brief Check if VM is running
     */
    bool is_running() const { return running_; }
    
    /**
     * @brief Get last error message
     */
    const std::string& get_error() const { return error_; }
    
    /**
     * @brief Set debug mode
     */
    void set_debug(bool debug) { debug_ = debug; }
    
private:
    // Register file
    std::vector<Value> registers_;
    
    // Special registers
    static constexpr uint8_t IP_REG = 16;  // Instruction pointer
    static constexpr uint8_t SP_REG = 17;  // Stack pointer
    static constexpr uint8_t BP_REG = 18;  // Base pointer
    static constexpr uint8_t FLAGS_REG = 19;  // Flags
    
    // Stack
    std::stack<Value> stack_;
    
    // Program
    Program* program_;
    size_t pc_;  // Program counter
    
    // Execution state
    bool running_;
    bool debug_;
    std::string error_;
    
    // Call stack for return addresses
    std::stack<size_t> call_stack_;
    
    // Flags
    bool zero_flag_;
    bool sign_flag_;
    bool overflow_flag_;
    
    // Dispatch to instruction handler
    void dispatch();
};

/**
 * @brief VM Result
 */
struct VMResult {
    bool success;
    int exit_code;
    std::string error;
    
    static VMResult ok(int code = 0) {
        return {true, code, ""};
    }
    
    static VMResult fail(std::string err) {
        return {false, -1, std::move(err)};
    }
};

} // namespace vm
} // namespace veldocra

