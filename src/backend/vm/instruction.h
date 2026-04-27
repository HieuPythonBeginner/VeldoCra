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

namespace veldanava {
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
    CONCAT,       // Concat strings: rd = ra + rb
    TO_STR,       // Convert int to string: rd = str(ra)
    READ,         // Read input to register
    
    // Memory (simple stack)
    PUSH,         // Push register to stack
    POP,          // Pop stack to register
    PEEK,         // Peek stack value
    
    // String operations
    MOVSTR,       // Load string constant: rd = strings[imm]
    STRIDX,       // String index: rd = string[ra]
    
    // Array/Memory operations
    ALLOC,        // Allocate memory: rd = alloc(ra)
    STORE,        // Store: mem[rd + ra] = rb
    LOAD,         // Load: rd = mem[ra + rb]

    // Object-oriented operations
    NEW_CLASS,    // Create new class: rd = new_class(name)
    NEW_OBJECT,   // Create new object: rd = new_object(class)
    GET_FIELD,    // Get object field: rd = object[ra].field
    SET_FIELD,    // Set object field: object[rd].field = ra
    CALL_METHOD,  // Call virtual method: rd = object.call_method(name, args...)
    CALL_VIRT,    // Call virtual method via vtable

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
    Char,
    Pointer,   // Generic pointer (void*)
    Object,    // Instance of a class
    Class      // Class definition with vtable
};

// Forward declarations for OOP support
struct VMClass;
struct VMObject;

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
        void* pointer_val;     // For generic pointers
        VMObject* object_val;  // For objects
        VMClass* class_val;    // For class definitions
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

    static Value from_pointer(void* ptr) {
        Value val;
        val.type = ValueType::Pointer;
        val.pointer_val = ptr;
        return val;
    }

    static Value from_object(VMObject* obj) {
        Value val;
        val.type = ValueType::Object;
        val.object_val = obj;
        return val;
    }

    static Value from_class(VMClass* cls) {
        Value val;
        val.type = ValueType::Class;
        val.class_val = cls;
        return val;
    }

    // Conversion methods
    int64_t to_int() const {
        if (type == ValueType::Int) return int_val;
        if (type == ValueType::Pointer) return reinterpret_cast<int64_t>(pointer_val);
        return 0;
    }

    void* to_pointer() const {
        if (type == ValueType::Pointer) return pointer_val;
        if (type == ValueType::Int) return reinterpret_cast<void*>(int_val);
        return nullptr;
    }
};

/**
 * @brief Virtual method entry
 */
struct VirtualMethod {
    const char* name;        // Method name
    uint32_t function_index; // Index in VM function table
    uint32_t vtable_index;   // Index in vtable
};

/**
 * @brief Class definition with virtual method table
 */
struct VMClass {
    const char* name;                    // Class name
    VMClass* parent;                     // Parent class (for inheritance)
    std::vector<VirtualMethod> vtable;   // Virtual method table
    std::vector<const char*> field_names; // Field names
    std::vector<ValueType> field_types;   // Field types

    // Constructor
    VMClass(const char* class_name, VMClass* parent_class = nullptr)
        : name(class_name), parent(parent_class) {}

    // Add a virtual method
    void add_virtual_method(const char* method_name, uint32_t function_index) {
        VirtualMethod method;
        method.name = method_name;
        method.function_index = function_index;
        method.vtable_index = vtable.size();
        vtable.push_back(method);
    }

    // Find virtual method by name
    const VirtualMethod* find_method(const char* method_name) const {
        for (const auto& method : vtable) {
            if (strcmp(method.name, method_name) == 0) {
                return &method;
            }
        }
        // Check parent class if not found
        if (parent) {
            return parent->find_method(method_name);
        }
        return nullptr;
    }
};

/**
 * @brief Object instance
 */
struct VMObject {
    VMClass* class_def;              // Class definition
    std::vector<Value> fields;       // Field values

    VMObject(VMClass* cls) : class_def(cls) {
        // Initialize fields with default values
        fields.resize(cls->field_names.size());
        for (size_t i = 0; i < fields.size(); ++i) {
            fields[i] = Value(); // Default None value
        }
    }

    // Get field by index
    Value get_field(size_t index) const {
        if (index < fields.size()) {
            return fields[index];
        }
        return Value(); // None
    }

    // Set field by index
    void set_field(size_t index, const Value& value) {
        if (index < fields.size()) {
            fields[index] = value;
        }
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
        std::memcpy(&val, operands + 3, sizeof(uint32_t));
        return val;
    }

    // Instruction size in bytes
    uint32_t size() const {
        switch (op) {
            case Opcode::MOVI:
            case Opcode::PRINTS:
                return 6;
            case Opcode::ADD:
            case Opcode::SUB:
            case Opcode::MUL:
            case Opcode::DIV:
                return 4;
            case Opcode::PRINT:
                return 2;
            case Opcode::CONCAT:
                return 4;
            case Opcode::TO_STR:
                return 3;
            case Opcode::CALL:
            case Opcode::JMP:
                return 5;
            case Opcode::JE:
            case Opcode::JNE:
            case Opcode::JG:
            case Opcode::JL:
            case Opcode::JGE:
            case Opcode::JLE:
                return 7;
            case Opcode::RET:
            case Opcode::HLT:
            default:
                return 1;
        }
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
        std::memcpy(inst.operands + 3, &target, sizeof(uint32_t));
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
    Instruction& get(size_t index) { return instructions_[index]; }
    size_t size() const { return instructions_.size(); }
    
    void set_jump_target(size_t index, uint32_t target) {
        auto& inst = instructions_[index];
        std::memcpy(inst.operands + 3, &target, sizeof(uint32_t));
    }
    
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
        case Opcode::STRIDX: return "STRIDX";
        case Opcode::ALLOC: return "ALLOC";
        case Opcode::STORE: return "STORE";
        case Opcode::LOAD: return "LOAD";
        default: return "INVALID";
    }
}

} // namespace vm
} // namespace veldanava

