/**
 * @file opcode.h
 * @brief x86-64 instruction opcodes for native codegen
 */

#ifndef VELDOCRA_NATIVE_OPCODE_H
#define VELDOCRA_NATIVE_OPCODE_H

#include <cstdint>
#include <vector>
#include <string>

namespace veldanava {
namespace native_codegen {

// x86-64 registers
enum class Register : uint8_t {
    RAX = 0, RCX = 1, RDX = 2, RBX = 3,
    RSP = 4, RBP = 5, RSI = 6, RDI = 7,
    R8  = 8, R9  = 9, R10 = 10, R11 = 11,
    R12 = 12, R13 = 13, R14 = 14, R15 = 15,
};

// Opcodes (TRUE FORM names)
enum class Opcode {
    // Data movement
    MOV_RR, MOV_RI, MOV_RM, MOV_MR,
    // Arithmetic
    ADD_RR, ADD_RI, SUB_RR, SUB_RI,
    MUL_RR, MUL_RI, DIV_RR, NEG, INC, DEC,
    // Bitwise
    AND_RR, AND_RI, OR_RR, OR_RI, XOR_RR, XOR_RI, NOT,
    // Shifts
    SHL_RR, SHL_RI, SHR_RR, SHR_RI, SAR_RR, SAR_RI,
    // Comparison
    CMP_RR, CMP_RI, TEST_RR, TEST_RI,
    // Stack
    PUSH_R, PUSH_I, POP_R, PUSHALL, POPALL,
    // Control flow
    JMP, JZ, JNZ, JL, JLE, JG, JGE, CALL, RET,
    // System
    SYSCALL, INT, NOP, HLT, INT3,
    // String
    MOVS, STOS, CMPS, LODS, SCAS,
    // FPU
    FLD, FST, FADD, FMUL, FSUB, FDIV,
};

// Operand types
enum class OpType { NONE, REG, IMM, MEM };

struct Operand {
    OpType type = OpType::NONE;
    int64_t imm_val = 0;
    uint8_t reg_val = 0;
    int32_t mem_offset = 0;

    static Operand reg(Register r) {
        Operand op;
        op.type = OpType::REG;
        op.reg_val = static_cast<uint8_t>(r);
        return op;
    }
    static Operand imm(int64_t v) {
        Operand op;
        op.type = OpType::IMM;
        op.imm_val = v;
        return op;
    }
    static Operand mem(int32_t off = 0) {
        Operand op;
        op.type = OpType::MEM;
        op.mem_offset = off;
        return op;
    }
    static Operand none() { return Operand{}; }
};

struct EncodedInstruction {
    std::vector<uint8_t> bytes;
};

class InstructionEncoder {
public:
    EncodedInstruction encode(Opcode op, Operand d = {}, Operand s = {}, Operand a = {});
    static std::string opcode_name(Opcode op);
    static std::string reg_name(Register reg);
};

inline uint8_t reg_idx(Register r) { return static_cast<uint8_t>(r); }

} // namespace native_codegen
} // namespace veldanava

#endif // VELDOCRA_NATIVE_OPCODE_H