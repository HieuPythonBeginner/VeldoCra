/**
 * @file assembler.cpp
 * @brief IR to x86-64 assembler
 */

#include "assembler.h"
#include <iostream>

namespace veldanava {
namespace middle {
namespace ir {

struct IRValue {
    enum Type { CONSTANT, REGISTER, PARAMETER } type;
    union { int64_t constant; uint32_t reg_id; uint32_t param_index; } value;
};

struct IRInstruction {
    enum Opcode { MOV, ADD, SUB, MUL, DIV, RET, CALL, JMP, CMP, AND, OR, XOR, NEG, NOT, SHL, SHR, PUSH, POP } opcode;
    IRValue dest, src1, src2;
    std::string label;
};

struct IRFunction {
    std::string name;
    std::vector<IRValue> parameters;
    std::vector<IRInstruction> instructions;
};

struct IRModule {
    std::vector<IRFunction> functions;
    std::vector<std::string> strings;
};

} // namespace ir
} // namespace middle
} // namespace veldanava

namespace veldanava {
namespace native_codegen {

Assembler::Assembler() : next_label_id_(1) {}

uint32_t Assembler::create_label() {
    return next_label_id_++;
}

AssembledModule Assembler::assemble(const middle::ir::IRModule& module) {
    AssembledModule result;
    uint64_t current_offset = 0;

    for (const auto& func : module.functions) {
        AssembledFunction af = assemble_function(func);
        result.symbol_table[func.name] = current_offset;
        af.entry_offset = current_offset;
        result.functions.push_back(af);
        current_offset += af.code.size();
    }

    for (const auto& str : module.strings) {
        size_t off = result.data_section.size();
        result.data_section.insert(result.data_section.end(), str.begin(), str.end());
        result.data_section.push_back(0);
    }

    return result;
}

AssembledFunction Assembler::assemble_function(const middle::ir::IRFunction& function) {
    AssembledFunction result;
    result.name = function.name;
    std::unordered_map<uint32_t, Register> reg_map;
    std::unordered_map<uint32_t, uint64_t> labels;

    setup_function_prologue(result.code);

    Register param_regs[] = {Register::RDI, Register::RSI, Register::RDX, Register::RCX, Register::R8, Register::R9};
    for (size_t i = 0; i < function.parameters.size() && i < 6; ++i) {
        reg_map[function.parameters[i].value.reg_id] = param_regs[i];
    }

    for (const auto& instr : function.instructions) {
        assemble_instruction(instr, result.code, labels, reg_map);
    }

    setup_function_epilogue(result.code);
    resolve_labels(result.code, labels);

    return result;
}

void Assembler::assemble_instruction(const middle::ir::IRInstruction& instr,
                                   std::vector<uint8_t>& code,
                                   std::unordered_map<uint32_t, uint64_t>& labels,
                                   std::unordered_map<uint32_t, Register>& reg_map) {
    using namespace middle::ir;

    switch (instr.opcode) {
        case IRInstruction::MOV: {
            Register dst = allocate_register(instr.dest, reg_map);
            if (instr.src1.type == IRValue::CONSTANT) {
                auto e = encoder_.encode(Opcode::MOV_RI, Operand::reg(dst), Operand::imm(instr.src1.value.constant));
                code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            } else {
                Register src = allocate_register(instr.src1, reg_map);
                auto e = encoder_.encode(Opcode::MOV_RR, Operand::reg(dst), Operand::reg(src));
                code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            }
            break;
        }
        case IRInstruction::ADD: {
            Register dst = allocate_register(instr.dest, reg_map);
            Register src = allocate_register(instr.src1, reg_map);
            auto e = encoder_.encode(Opcode::ADD_RR, Operand::reg(dst), Operand::reg(src));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::SUB: {
            Register dst = allocate_register(instr.dest, reg_map);
            Register src = allocate_register(instr.src1, reg_map);
            auto e = encoder_.encode(Opcode::SUB_RR, Operand::reg(dst), Operand::reg(src));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::MUL: {
            Register dst = allocate_register(instr.dest, reg_map);
            Register src = allocate_register(instr.src1, reg_map);
            auto e = encoder_.encode(Opcode::MUL_RR, Operand::reg(dst), Operand::reg(src));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::DIV: {
            Register dst = allocate_register(instr.dest, reg_map);
            auto e = encoder_.encode(Opcode::DIV_RR, Operand::reg(dst));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::CMP: {
            Register dst = allocate_register(instr.dest, reg_map);
            Register src = allocate_register(instr.src1, reg_map);
            auto e = encoder_.encode(Opcode::CMP_RR, Operand::reg(dst), Operand::reg(src));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::AND: {
            Register dst = allocate_register(instr.dest, reg_map);
            Register src = allocate_register(instr.src1, reg_map);
            auto e = encoder_.encode(Opcode::AND_RR, Operand::reg(dst), Operand::reg(src));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::OR: {
            Register dst = allocate_register(instr.dest, reg_map);
            Register src = allocate_register(instr.src1, reg_map);
            auto e = encoder_.encode(Opcode::OR_RR, Operand::reg(dst), Operand::reg(src));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::XOR: {
            Register dst = allocate_register(instr.dest, reg_map);
            Register src = allocate_register(instr.src1, reg_map);
            auto e = encoder_.encode(Opcode::XOR_RR, Operand::reg(dst), Operand::reg(src));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::NEG: {
            Register dst = allocate_register(instr.dest, reg_map);
            auto e = encoder_.encode(Opcode::NEG, Operand::reg(dst));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::NOT: {
            Register dst = allocate_register(instr.dest, reg_map);
            auto e = encoder_.encode(Opcode::NOT, Operand::reg(dst));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::SHL: {
            Register dst = allocate_register(instr.dest, reg_map);
            auto e = encoder_.encode(Opcode::SHL_RR, Operand::reg(dst), Operand::reg(Register::RCX));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::SHR: {
            Register dst = allocate_register(instr.dest, reg_map);
            auto e = encoder_.encode(Opcode::SHR_RR, Operand::reg(dst), Operand::reg(Register::RCX));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::RET: {
            auto e = encoder_.encode(Opcode::RET);
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::PUSH: {
            Register src = allocate_register(instr.src1, reg_map);
            auto e = encoder_.encode(Opcode::PUSH_R, Operand::reg(src));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        case IRInstruction::POP: {
            Register dst = allocate_register(instr.dest, reg_map);
            auto e = encoder_.encode(Opcode::POP_R, Operand::reg(dst));
            code.insert(code.end(), e.bytes.begin(), e.bytes.end());
            break;
        }
        default:
            std::cerr << "Warning: unimplemented IR opcode\n";
            break;
    }
}

Register Assembler::allocate_register(const middle::ir::IRValue& value,
                                     std::unordered_map<uint32_t, Register>& reg_map) {
    if (value.type == middle::ir::IRValue::REGISTER) {
        auto it = reg_map.find(value.value.reg_id);
        if (it != reg_map.end()) return it->second;
        static Register avail[] = {Register::RAX, Register::RBX, Register::RCX, Register::RDX, Register::RSI, Register::RDI};
        static int next = 0;
        Register r = avail[next % 6];
        reg_map[value.value.reg_id] = r;
        next++;
        return r;
    }
    return Register::RAX;
}

void Assembler::setup_function_prologue(std::vector<uint8_t>& code) {
    auto e = encoder_.encode(Opcode::PUSH_R, Operand::reg(Register::RBP));
    code.insert(code.end(), e.bytes.begin(), e.bytes.end());
    e = encoder_.encode(Opcode::MOV_RR, Operand::reg(Register::RBP), Operand::reg(Register::RSP));
    code.insert(code.end(), e.bytes.begin(), e.bytes.end());
}

void Assembler::setup_function_epilogue(std::vector<uint8_t>& code) {
    auto e = encoder_.encode(Opcode::MOV_RR, Operand::reg(Register::RSP), Operand::reg(Register::RBP));
    code.insert(code.end(), e.bytes.begin(), e.bytes.end());
    e = encoder_.encode(Opcode::POP_R, Operand::reg(Register::RBP));
    code.insert(code.end(), e.bytes.begin(), e.bytes.end());
    e = encoder_.encode(Opcode::RET);
    code.insert(code.end(), e.bytes.begin(), e.bytes.end());
}

void Assembler::resolve_labels(std::vector<uint8_t>& code,
                             const std::unordered_map<uint32_t, uint64_t>& labels) {}

} // namespace native_codegen
} // namespace veldanava