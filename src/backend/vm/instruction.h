/**
 * @file instruction.h
 * @brief VeldoCra VM Instruction Definitions
 * @author Dr. Bright
 * 
 * Register-based VM instructions for VeldoCra.
 * Uses computed goto for fast dispatch.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <array>

namespace veldocra {
namespace vm {

/**
 * @brief VM Opcodes
 * 
 * Register-based instruction set.
 * All arithmetic/logical ops use register operands.
 */
enum class Opcode : uint8_t {
    // Special
    HLT = 0,      // Halt execution
    NOP,          // No operation
    MOV,          // Move constant or register to register
    MOVI,         // Move immediate (constant) to register
    
    // Arithmetic
    ADD,          // Add: rd = ra + rb
    SUB,          // Subtract: rd = ra - rb
    MUL,          // Multiply: rd = ra * rb
    DIV,          // Divide: rd = ra / rb
    MOD,          // Modulo: rd = ra % rb
    NEG,          // Negate: rd = -ra
    
    // Bitwise
    AND,          // Bitwise AND
    OR,           // Bitwise OR
    XOR,          // Bitwise XOR
    NOT,          // Bitwise NOT
    SHL,          // Shift left
    SHR,          // Shift right
    
    // Comparison
    CMP,          // Compare: set flags
    
    // Control flow
    JMP,          // Unconditional jump
    JZ,           // Jump if zero
    JNZ,          // Jump if not zero
    JE,           // Jump if equal
    JNE,          // Jump if not equal
    JG,           // Jump if greater
    JGE,          // Jump if greater or equal
    JL,           // Jump if less
    JLE,          // Jump if less or equal
    CALL,         // Call function
    RET,          // Return from function
    
    // Function/IO
    PRINT,        // Print register value
    PRINTS,       // Print string constant
    READ,         // Read input to register
    
    // Memory (simple stack)
    PUSH,         // Push register to stack
    POP,          // Pop stack to register
    PEEK,         // Peek stack value
    
    // End marker
    INVALID
};

/**
 * @brief VM Registers
 * 
 * Fixed-size register file.
 */
enum class Register : uint8_t {
    R0 = 0,
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
    R8 = 8,
    R9 = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    R14 = 14,
    R15 = 15,
    
    // Special registers
    IP = 16,      // Instruction pointer
    SP = 17,      // Stack pointer
    BP = 18,      // Base pointer
    FLAGS = 19,   // Flags register
    
    // Number of registers
    COUNT = 20
};

/**
 * @brief VM Value types
 */
enum class ValueType : uint8_t {
    None = 0,
    Int,
    Float,
    Bool,
    String,
    Char
};

/**
 * @brief VM Value (tagged union)
 */
struct Value {
    ValueType type;
    
    union {
        int64_t int_val;
        double float_val;
        bool bool_val;
        char char_val;
    };
    
    const char* string_val;  // For strings (stored separately)
    
    Value() : type(ValueType::None), int_val(0), string_val(nullptr) {}
    
    static Value from_int(int64_t v) {
        Value val;
        val.type = ValueType::Int;
        val.int_val = v;
        return val;
    }
    
    static Value from_float(double v) {
        Value val;
        val.type = ValueType::Float;
        val.float_val = v;
        return val;
    }
    
    static Value from_bool(bool v) {
        Value val;
        val.type = ValueType::Bool;
        val.bool_val = v;
        return val;
    }
};

/**
 * @brief VM Instruction
 * 
 * Variable-length instruction format:
 * - opcode (1 byte)
 * - operand count (1 byte)
 * - operands (variable)
 * 
 * Most instructions: op rd, ra, rb
 * Some: op rd, immediate
 */
struct Instruction {
    Opcode op;
    uint8_t operands[12];  // Up to 3 registers or register + immediate
    
    // Quick accessors
    uint8_t rd() const { return operands[0]; }
    uint8_t ra() const { return operands[1]; }
    uint8_t rb() const { return operands[2]; }
    
    // Immediate value access (encoded in operands as two's complement)
    int32_t imm32() const {
        int32_t val;
        std::memcpy(&val, operands + 4, sizeof(int32_t));
        return val;
    }
    
    // Jump target access
    uint32_t target() const {
        uint32_t val;
        std::memcpy(&val, operands + 4, sizeof(uint32_t));
        return val;
    }
};

/**
 * @brief VM Program
 * 
 * Contiguous array of instructions.
 */
class Program {
public:
    Program() = default;
    
    /**
     * @brief Add instruction to program
     * @return Index of added instruction
     */
    size_t add(Opcode op) {
        Instruction inst;
        inst.op = op;
        std::memset(inst.operands, 0, sizeof(inst.operands));
        instructions_.push_back(inst);
        return instructions_.size() - 1;
    }
    
    size_t add(Opcode op, uint8_t rd, uint8_t ra, uint8_t rb) {
        Instruction inst;
        inst.op = op;
        std::memset(inst.operands, 0, sizeof(inst.operands));
        inst.operands[0] = rd;
        inst.operands[1] = ra;
        inst.operands[2] = rb;
        instructions_.push_back(inst);
        return instructions_.size() - 1;
    }
    
    size_t add(Opcode op, uint8_t rd, uint8_t ra, int32_t imm) {
        Instruction inst;
        inst.op = op;
        std::memset(inst.operands, 0, sizeof(inst.operands));
        inst.operands[0] = rd;
        inst.operands[1] = ra;
        std::memcpy(inst.operands + 4, &imm, sizeof(int32_t));
        instructions_.push_back(inst);
        return instructions_.size() - 1;
    }
    
    size_t add(Opcode op, uint8_t rd, int32_t imm) {
        Instruction inst;
        inst.op = op;
        std::memset(inst.operands, 0, sizeof(inst.operands));
        inst.operands[0] = rd;
        std::memcpy(inst.operands + 4, &imm, sizeof(int32_t));
        instructions_.push_back(inst);
        return instructions_.size() - 1;
    }
    
    size_t add(Opcode op, uint8_t rd, uint8_t ra) {
        Instruction inst;
        inst.op = op;
        std::memset(inst.operands, 0, sizeof(inst.operands));
        inst.operands[0] = rd;
        inst.operands[1] = ra;
        instructions_.push_back(inst);
        return instructions_.size() - 1;
    }
    
    size_t add(Opcode op, uint32_t target) {
        Instruction inst;
        inst.op = op;
        std::memset(inst.operands, 0, sizeof(inst.operands));
        std::memcpy(inst.operands + 4, &target, sizeof(uint32_t));
        instructions_.push_back(inst);
        return instructions_.size() - 1;
    }
    
    size_t add(Opcode op, uint8_t rd, const char* str_id) {
        // For string constants, store the string id in operands
        Instruction inst;
        inst.op = op;
        std::memset(inst.operands, 0, sizeof(inst.operands));
        inst.operands[0] = rd;
        // Store string index (we'll handle this separately)
        instructions_.push_back(inst);
        return instructions_.size() - 1;
    }
    
    const Instruction& get(size_t index) const { return instructions_[index]; }
    size_t size() const { return instructions_.size(); }
    
    /**
     * @brief Add string constant
     */
    size_t add_string(std::string_view str) {
        strings_.push_back(std::string(str));
        return strings_.size() - 1;
    }
    
    const std::string& get_string(size_t index) const { return strings_[index]; }
    size_t string_count() const { return strings_.size(); }
    
private:
    std::vector<Instruction> instructions_;
    std::vector<std::string> strings_;
};

/**
 * @brief Instruction opcode to string
 */
inline const char* opcode_to_string(Opcode op) {
    switch (op) {
        case Opcode::HLT: return "HLT";
        case Opcode::NOP: return "NOP";
        case Opcode::MOV: return "MOV";
        case Opcode::MOVI: return "MOVI";
        case Opcode::ADD: return "ADD";
        case Opcode::SUB: return "SUB";
        case Opcode::MUL: return "MUL";
        case Opcode::DIV: return "DIV";
        case Opcode::MOD: return "MOD";
        case Opcode::NEG: return "NEG";
        case Opcode::AND: return "AND";
        case Opcode::OR: return "OR";
        case Opcode::XOR: return "XOR";
        case Opcode::NOT: return "NOT";
        case Opcode::SHL: return "SHL";
        case Opcode::SHR: return "SHR";
        case Opcode::CMP: return "CMP";
        case Opcode::JMP: return "JMP";
        case Opcode::JZ: return "JZ";
        case Opcode::JNZ: return "JNZ";
        case Opcode::JE: return "JE";
        case Opcode::JNE: return "JNE";
        case Opcode::JG: return "JG";
        case Opcode::JGE: return "JGE";
        case Opcode::JL: return "JL";
        case Opcode::JLE: return "JLE";
        case Opcode::CALL: return "CALL";
        case Opcode::RET: return "RET";
        case Opcode::PRINT: return "PRINT";
        case Opcode::PRINTS: return "PRINTS";
        case Opcode::READ: return "READ";
        case Opcode::PUSH: return "PUSH";
        case Opcode::POP: return "POP";
        case Opcode::PEEK: return "PEEK";
        default: return "INVALID";
    }
}

} // namespace vm
} // namespace veldocra

