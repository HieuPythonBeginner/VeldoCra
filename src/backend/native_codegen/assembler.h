/**
 * @file assembler.h
 * @brief IR to x86-64 assembler
 */

#ifndef VELDOCRA_NATIVE_ASSEMBLER_H
#define VELDOCRA_NATIVE_ASSEMBLER_H

#include "opcode.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>

namespace veldanava {
namespace middle {
namespace ir {
    struct IRModule;
    struct IRFunction;
    struct IRInstruction;
    struct IRValue;
} // namespace ir
} // namespace middle

namespace native_codegen {

// Assembled function
struct AssembledFunction {
    std::string name;
    std::vector<uint8_t> code;
    uint64_t entry_offset = 0;  // offset of function entry in code
    std::unordered_map<uint32_t, uint64_t> label_offsets;  // label_id -> offset
};

// Assembled module (collection of functions)
struct AssembledModule {
    std::vector<AssembledFunction> functions;
    std::vector<uint8_t> data_section;  // strings, constants
    std::unordered_map<std::string, uint64_t> symbol_table;  // function_name -> offset
};

// x86-64 Assembler - converts IR to machine code
class Assembler {
public:
    Assembler();

    // Main assembly function
    AssembledModule assemble(const middle::ir::IRModule& module);

private:
    InstructionEncoder encoder_;
    uint32_t next_label_id_ = 0;

    // Assembly helpers
    AssembledFunction assemble_function(const middle::ir::IRFunction& function);
    void assemble_instruction(const middle::ir::IRInstruction& instr,
                            std::vector<uint8_t>& code,
                            std::unordered_map<uint32_t, uint64_t>& labels,
                            std::unordered_map<uint32_t, Register>& reg_map);
    void assemble_block(const std::vector<middle::ir::IRInstruction>& block,
                       std::vector<uint8_t>& code,
                       std::unordered_map<uint32_t, uint64_t>& labels);

    // Register allocation (simple for now)
    Register allocate_register(const middle::ir::IRValue& value,
                             std::unordered_map<uint32_t, Register>& reg_map);

    // Label management
    uint32_t create_label();
    void resolve_labels(std::vector<uint8_t>& code,
                       const std::unordered_map<uint32_t, uint64_t>& labels);

    // Calling convention helpers
    void setup_function_prologue(std::vector<uint8_t>& code);
    void setup_function_epilogue(std::vector<uint8_t>& code);
};

} // namespace native_codegen
} // namespace veldanava

#endif // VELDOCRA_NATIVE_ASSEMBLER_H