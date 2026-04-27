/**
 * @file opcode.cpp
 * @brief x86-64 instruction encoding
 */

#include "opcode.h"
#include <iostream>

namespace veldanava {
namespace native_codegen {

std::string InstructionEncoder::opcode_name(Opcode op) {
    switch (op) {
        case Opcode::MOV_RR: return "MOV_RR"; case Opcode::MOV_RI: return "MOV_RI";
        case Opcode::ADD_RR: return "ADD_RR"; case Opcode::ADD_RI: return "ADD_RI";
        case Opcode::SUB_RR: return "SUB_RR"; case Opcode::SUB_RI: return "SUB_RI";
        case Opcode::MUL_RR: return "MUL_RR"; case Opcode::DIV_RR: return "DIV_RR";
        case Opcode::NEG: return "NEG"; case Opcode::INC: return "INC"; case Opcode::DEC: return "DEC";
        case Opcode::AND_RR: return "AND_RR"; case Opcode::OR_RR: return "OR_RR";
        case Opcode::XOR_RR: return "XOR_RR"; case Opcode::NOT: return "NOT";
        case Opcode::SHL_RR: return "SHL_RR"; case Opcode::SHR_RR: return "SHR_RR";
        case Opcode::SAR_RR: return "SAR_RR";
        case Opcode::CMP_RR: return "CMP_RR"; case Opcode::CMP_RI: return "CMP_RI";
        case Opcode::TEST_RR: return "TEST_RR";
        case Opcode::PUSH_R: return "PUSH_R"; case Opcode::PUSH_I: return "PUSH_I";
        case Opcode::POP_R: return "POP_R"; case Opcode::PUSHALL: return "PUSHALL";
        case Opcode::POPALL: return "POPALL";
        case Opcode::JMP: return "JMP"; case Opcode::JZ: return "JZ";
        case Opcode::JNZ: return "JNZ"; case Opcode::JL: return "JL";
        case Opcode::JLE: return "JLE"; case Opcode::JG: return "JG";
        case Opcode::JGE: return "JGE"; case Opcode::CALL: return "CALL";
        case Opcode::RET: return "RET"; case Opcode::SYSCALL: return "SYSCALL";
        case Opcode::INT: return "INT"; case Opcode::NOP: return "NOP";
        case Opcode::HLT: return "HLT"; case Opcode::INT3: return "INT3";
        default: return "UNKNOWN";
    }
}

std::string InstructionEncoder::reg_name(Register reg) {
    static const char* n[] = {
        "rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi",
        "r8","r9","r10","r11","r12","r13","r14","r15"
    };
    return n[reg_idx(reg)];
}

static void encode_rex(uint8_t bits, std::vector<uint8_t>& out) {
    uint8_t rex = 0x40 | bits;
    if (rex != 0x40) out.push_back(rex);
}

static void encode_modrm(uint8_t mod, uint8_t reg, uint8_t rm, std::vector<uint8_t>& out) {
    out.push_back((mod << 6) | ((reg & 7) << 3) | (rm & 7));
}

static void encode_imm(int64_t val, int size, std::vector<uint8_t>& out) {
    for (int i = 0; i < size; ++i) {
        out.push_back(static_cast<uint8_t>(val & 0xFF));
        val >>= 8;
    }
}

EncodedInstruction InstructionEncoder::encode(Opcode op, Operand d, Operand s, Operand a) {
    EncodedInstruction result;
    std::vector<uint8_t>& b = result.bytes;

    uint8_t dst = (d.type == OpType::REG) ? d.reg_val : 0;
    uint8_t src = (s.type == OpType::REG) ? s.reg_val : 0;

    switch (op) {
        // ===== DATA MOVEMENT =====
        case Opcode::MOV_RR: {
            encode_rex(0x08, b);
            b.push_back(0x89);
            encode_modrm(0x03, src, dst, b);
            break;
        }
        case Opcode::MOV_RI: {
            encode_rex(0x08, b);
            b.push_back(0xB8 + (dst & 7));
            encode_imm(s.imm_val, 8, b);
            break;
        }

        // ===== ARITHMETIC =====
        case Opcode::ADD_RR: {
            encode_rex(0x08, b);
            b.push_back(0x01);
            encode_modrm(0x03, src, dst, b);
            break;
        }
        case Opcode::ADD_RI: {
            encode_rex(0x08, b);
            b.push_back(0x81);
            encode_modrm(0x03, 0, dst, b);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::SUB_RR: {
            encode_rex(0x08, b);
            b.push_back(0x29);
            encode_modrm(0x03, src, dst, b);
            break;
        }
        case Opcode::SUB_RI: {
            encode_rex(0x08, b);
            b.push_back(0x81);
            encode_modrm(0x03, 5, dst, b);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::MUL_RR: {
            encode_rex(0x08, b);
            b.push_back(0x0F);
            b.push_back(0xAF);
            encode_modrm(0x03, src, dst, b);
            break;
        }
        case Opcode::DIV_RR: {
            encode_rex(0x08, b);
            b.push_back(0x99);
            b.push_back(0xF7);
            b.push_back(0xF8 | dst);
            break;
        }
        case Opcode::NEG: {
            encode_rex(0x08, b);
            b.push_back(0xF7);
            b.push_back(0xD8 | dst);
            break;
        }
        case Opcode::INC: {
            if (dst >= 8) encode_rex(0x08, b);
            b.push_back(0x40 + (dst & 7));
            break;
        }
        case Opcode::DEC: {
            if (dst >= 8) encode_rex(0x08, b);
            b.push_back(0x48 + (dst & 7));
            break;
        }

        // ===== BITWISE =====
        case Opcode::AND_RR: {
            encode_rex(0x08, b);
            b.push_back(0x21);
            encode_modrm(0x03, src, dst, b);
            break;
        }
        case Opcode::AND_RI: {
            encode_rex(0x08, b);
            b.push_back(0x81);
            encode_modrm(0x03, 4, dst, b);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::OR_RR: {
            encode_rex(0x08, b);
            b.push_back(0x09);
            encode_modrm(0x03, src, dst, b);
            break;
        }
        case Opcode::OR_RI: {
            encode_rex(0x08, b);
            b.push_back(0x81);
            encode_modrm(0x03, 1, dst, b);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::XOR_RR: {
            encode_rex(0x08, b);
            b.push_back(0x31);
            encode_modrm(0x03, src, dst, b);
            break;
        }
        case Opcode::XOR_RI: {
            encode_rex(0x08, b);
            b.push_back(0x81);
            encode_modrm(0x03, 6, dst, b);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::NOT: {
            encode_rex(0x08, b);
            b.push_back(0xF7);
            b.push_back(0xD0 | dst);
            break;
        }

        // ===== SHIFTS =====
        case Opcode::SHL_RR: {
            encode_rex(0x08, b);
            b.push_back(0xD3);
            encode_modrm(0x03, 4, dst, b);
            break;
        }
        case Opcode::SHL_RI: {
            encode_rex(0x08, b);
            b.push_back(0xC1);
            encode_modrm(0x03, 4, dst, b);
            b.push_back(static_cast<uint8_t>(s.imm_val & 0x3F));
            break;
        }
        case Opcode::SHR_RR: {
            encode_rex(0x08, b);
            b.push_back(0xD3);
            encode_modrm(0x03, 5, dst, b);
            break;
        }
        case Opcode::SHR_RI: {
            encode_rex(0x08, b);
            b.push_back(0xC1);
            encode_modrm(0x03, 5, dst, b);
            b.push_back(static_cast<uint8_t>(s.imm_val & 0x3F));
            break;
        }
        case Opcode::SAR_RR: {
            encode_rex(0x08, b);
            b.push_back(0xD3);
            encode_modrm(0x03, 7, dst, b);
            break;
        }
        case Opcode::SAR_RI: {
            encode_rex(0x08, b);
            b.push_back(0xC1);
            encode_modrm(0x03, 7, dst, b);
            b.push_back(static_cast<uint8_t>(s.imm_val & 0x3F));
            break;
        }

        // ===== COMPARISON =====
        case Opcode::CMP_RR: {
            encode_rex(0x08, b);
            b.push_back(0x3B);
            encode_modrm(0x03, src, dst, b);
            break;
        }
        case Opcode::CMP_RI: {
            encode_rex(0x08, b);
            b.push_back(0x81);
            encode_modrm(0x03, 7, dst, b);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::TEST_RR: {
            encode_rex(0x08, b);
            b.push_back(0x85);
            encode_modrm(0x03, src, dst, b);
            break;
        }
        case Opcode::TEST_RI: {
            encode_rex(0x08, b);
            b.push_back(0xF7);
            encode_modrm(0x03, 0, dst, b);
            encode_imm(s.imm_val, 4, b);
            break;
        }

        // ===== STACK =====
        case Opcode::PUSH_R: {
            if (dst >= 8) encode_rex(0x01, b);
            b.push_back(0x50 + (dst & 7));
            break;
        }
        case Opcode::PUSH_I: {
            b.push_back(0x68);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::POP_R: {
            if (dst >= 8) encode_rex(0x01, b);
            b.push_back(0x58 + (dst & 7));
            break;
        }
        case Opcode::PUSHALL: {
            b.push_back(0x60);
            break;
        }
        case Opcode::POPALL: {
            b.push_back(0x61);
            break;
        }

        // ===== CONTROL FLOW =====
        case Opcode::JMP: {
            b.push_back(0xE9);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::JZ: {
            b.push_back(0x0F);
            b.push_back(0x84);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::JNZ: {
            b.push_back(0x0F);
            b.push_back(0x85);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::JL: {
            b.push_back(0x0F);
            b.push_back(0x8C);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::JLE: {
            b.push_back(0x0F);
            b.push_back(0x8E);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::JG: {
            b.push_back(0x0F);
            b.push_back(0x8F);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::JGE: {
            b.push_back(0x0F);
            b.push_back(0x8D);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::CALL: {
            b.push_back(0xE8);
            encode_imm(s.imm_val, 4, b);
            break;
        }
        case Opcode::RET: {
            b.push_back(0xC3);
            break;
        }

        // ===== SYSTEM =====
        case Opcode::SYSCALL: {
            b.push_back(0x0F);
            b.push_back(0x05);
            break;
        }
        case Opcode::INT: {
            b.push_back(0xCD);
            b.push_back(static_cast<uint8_t>(s.imm_val & 0xFF));
            break;
        }
        case Opcode::NOP: {
            b.push_back(0x90);
            break;
        }
        case Opcode::HLT: {
            b.push_back(0xF4);
            break;
        }
        case Opcode::INT3: {
            b.push_back(0xCC);
            break;
        }

        default:
            std::cerr << "Warning: unimplemented opcode: " << opcode_name(op) << "\n";
            b.push_back(0x90);
            break;
    }

    return result;
}

} // namespace native_codegen
} // namespace veldanava