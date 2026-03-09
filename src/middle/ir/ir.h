/**
 * @file ir.h
 * @brief VeldoCra SSA Intermediate Representation (IR)
 * @author Dr. Bright
 * 
 * SSA IR for VeldoCra with support for:
 * - Soul-based types (Custom Species)
 * - System command intrinsics (origin, flow)
 * - Full optimization pipeline
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>

namespace veldocra {
namespace ir {

// Forward declarations
class IRBuilder;
class IRModule;
class IRFunction;
class IRBasicBlock;
class IRInstruction;
class IRValue;

/**
 * @brief IR Opcodes - Operations in SSA IR
 */
enum class IROpcode : uint16_t {
    // Special
    INVALID = 0,
    NOP,
    
    // Control flow
    BR,         // Unconditional branch
    BR_COND,    // Conditional branch
    RET,        // Return
    PHI,        // PHI node for SSA
    
    // Memory
    ALLOCA,     // Stack allocation
    LOAD,       // Load from memory
    STORE,      // Store to memory
    GEP,        // Get element pointer
    
    // Casts
    ZEXT,       // Zero extend
    SEXT,       // Sign extend
    TRUNC,      // Truncate
    FPTOINT,    // Float to int
    INTTOFP,    // Int to float
    BITCAST,    // Bit cast
    
    // Arithmetic
    ADD,        // Add
    SUB,        // Subtract
    MUL,        // Multiply
    DIV,        // Divide
    MOD,        // Modulo
    NEG,        // Negate
    
    // Bitwise
    AND,        // Bitwise AND
    OR,         // Bitwise OR
    XOR,        // Bitwise XOR
    NOT,        // Bitwise NOT
    SHL,        // Shift left
    SHR,        // Shift right
    
    // Comparison
    CMP_EQ,     // Equal
    CMP_NE,     // Not equal
    CMP_SGT,   // Signed greater than
    CMP_SGE,   // Signed greater or equal
    CMP_SLT,   // Signed less than
    CMP_SLE,   // Signed less or equal
    CMP_UGT,   // Unsigned greater than
    CMP_UGE,   // Unsigned greater or equal
    CMP_ULT,   // Unsigned less than
    CMP_ULE,   // Unsigned less or equal
    
    // Call
    CALL,       // Function call
    INVOKE,     // Function call with exception handling
    
    // System Commands (Soul-based intrinsics)
    // These map to VeldoCra's unique system commands
    SYSCMD_ORIGIN,   // @origin - Create soul origin
    SYSCMD_FLOW,     // @flow - Control soul flow
    SYSCMD_ABSORB,   // @absorb - Absorb entity
    SYSCMD_MANIFEST, // @manifest - Manifest soul
    
    // Debug/Print
    PRINT,      // Print value
    DBG_TRAP,   // Debug trap
};

/**
 * @brief Comparison predicate types
 */
enum class CmpPredicate {
    EQ, NE, SGT, SGE, SLT, SLE, UGT, UGE, ULT, ULE
};

/**
 * @brief Type kinds for VeldoCra's type system
 * Includes Soul-based types for Custom Species
 */
enum class TypeKind : uint8_t {
    // Primitive types
    Void = 0,
    Bool,
    Char,
    Int8, Int16, Int32, Int64,
    UInt8, UInt16, UInt32, UInt64,
    Float32, Float64,
    String,
    
    // Soul-based types (Custom Species)
    Soul,           // Base soul type
    Entity,         // Entity with soul
    Species,        // Custom species class
    SoulFragment,   // Fragment of a soul
    Essence,        // Essence type
    
    // Compound types
    Array,
    Tuple,
    Struct,
    Enum,
    Function,
    Pointer,
    Reference,
};

/**
 * @brief Type representation
 */
class Type {
public:
    explicit Type(TypeKind kind) : kind_(kind), size_(0), align_(1) {}
    virtual ~Type() = default;
    
    TypeKind get_kind() const { return kind_; }
    size_t get_size() const { return size_; }
    size_t get_alignment() const { return align_; }
    
    bool is_void() const { return kind_ == TypeKind::Void; }
    bool is_bool() const { return kind_ == TypeKind::Bool; }
    bool is_integer() const { 
        return kind_ >= TypeKind::Int8 && kind_ <= TypeKind::UInt64; 
    }
    bool is_float() const { 
        return kind_ == TypeKind::Float32 || kind_ == TypeKind::Float64; 
    }
    bool is_numeric() const { return is_integer() || is_float(); }
    bool is_pointer() const { return kind_ == TypeKind::Pointer; }
    bool is_soul_type() const {
        return kind_ >= TypeKind::Soul && kind_ <= TypeKind::Essence;
    }
    
    virtual std::string to_string() const = 0;
    
    static Type* get_void();
    static Type* get_bool();
    static Type* get_char();
    static Type* get_int32();
    static Type* get_int64();
    static Type* get_float32();
    static Type* get_float64();
    static Type* get_string();
    static Type* get_soul();
    static Type* get_entity();
    static Type* get_pointer_to(Type* element_type);
    
protected:
    TypeKind kind_;
    size_t size_;
    size_t align_;
};

/**
 * @brief IR Value - can be constant, register, or instruction result
 */
class IRValue {
public:
    enum class ValueKind {
        Undef,
        ConstantInt,
        ConstantFloat,
        ConstantBool,
        ConstantString,
        ConstantNull,
        Argument,
        Instruction,
    };
    
    IRValue(ValueKind kind, Type* type) : kind_(kind), type_(type) {}
    virtual ~IRValue() = default;
    
    ValueKind get_kind() const { return kind_; }
    Type* get_type() const { return type_; }
    
    virtual std::string to_string() const = 0;
    
    // For constants
    virtual int64_t get_constant_int() const { return 0; }
    virtual double get_constant_float() const { return 0.0; }
    virtual bool get_constant_bool() const { return false; }
    virtual std::string_view get_constant_string() const { return ""; }
    
protected:
    ValueKind kind_;
    Type* type_;
};

/**
 * @brief IR Constant Integer
 */
class ConstantInt : public IRValue {
public:
    ConstantInt(int64_t value, Type* type) 
        : IRValue(ValueKind::ConstantInt, type), value_(value) {}
    
    int64_t get_value() const { return value_; }
    int64_t get_constant_int() const override { return value_; }
    std::string to_string() const override;
    
private:
    int64_t value_;
};

/**
 * @brief IR Constant Float
 */
class ConstantFloat : public IRValue {
public:
    ConstantFloat(double value, Type* type)
        : IRValue(ValueKind::ConstantFloat, type), value_(value) {}
    
    double get_value() const { return value_; }
    double get_constant_float() const override { return value_; }
    std::string to_string() const override;
    
private:
    double value_;
};

/**
 * @brief IR Constant Bool
 */
class ConstantBool : public IRValue {
public:
    ConstantBool(bool value, Type* type)
        : IRValue(ValueKind::ConstantBool, type), value_(value) {}
    
    bool get_value() const { return value_; }
    bool get_constant_bool() const override { return value_; }
    std::string to_string() const override;
    
private:
    bool value_;
};

/**
 * @brief IR Constant String
 */
class ConstantString : public IRValue {
public:
    ConstantString(std::string value, Type* type)
        : IRValue(ValueKind::ConstantString, type), value_(std::move(value)) {}
    
    const std::string& get_value() const { return value_; }
    std::string_view get_constant_string() const override { return value_; }
    std::string to_string() const override;
    
private:
    std::string value_;
};

/**
 * @brief Function argument
 */
class Argument : public IRValue {
public:
    Argument(std::string name, Type* type, size_t index)
        : IRValue(ValueKind::Argument, type), name_(std::move(name)), index_(index) {}
    
    const std::string& get_name() const { return name_; }
    size_t get_index() const { return index_; }
    std::string to_string() const override;
    
private:
    std::string name_;
    size_t index_;
};

/**
 * @brief IR Instruction
 */
class IRInstruction : public IRValue {
public:
    IRInstruction(IROpcode opcode, Type* type, IRBasicBlock* parent);
    virtual ~IRInstruction() = default;
    
    IROpcode get_opcode() const { return opcode_; }
    IRBasicBlock* get_parent() const { return parent_; }
    
    void add_operand(IRValue* value) { operands_.push_back(value); }
    const std::vector<IRValue*>& get_operands() const { return operands_; }
    size_t get_num_operands() const { return operands_.size(); }
    IRValue* get_operand(size_t index) const { return operands_[index]; }
    
    size_t get_id() const { return id_; }
    void set_id(size_t id) { id_ = id; }
    
    virtual std::string to_string() const override;
    
protected:
    IROpcode opcode_;
    IRBasicBlock* parent_;
    std::vector<IRValue*> operands_;
    size_t id_;
};

/**
 * @brief PHI node for SSA form
 */
class PhiInst : public IRInstruction {
public:
    PhiInst(Type* type, IRBasicBlock* parent);
    
    void add_incoming(IRValue* value, IRBasicBlock* block);
    std::vector<std::pair<IRValue*, IRBasicBlock*>> get_incomings() const;
    
    std::string to_string() const override;
    
private:
    std::vector<std::pair<IRValue*, IRBasicBlock*>> incomings_;
};

/**
 * @brief Basic block - list of instructions with a label
 */
class IRBasicBlock {
public:
    explicit IRBasicBlock(IRFunction* parent, std::string name = "");
    ~IRBasicBlock();
    
    IRFunction* get_parent() const { return parent_; }
    const std::string& get_name() const { return name_; }
    size_t get_id() const { return id_; }
    
    void add_instruction(IRInstruction* inst);
    const std::vector<IRInstruction*>& get_instructions() const { return instructions_; }
    
    // Control flow
    void add_successor(IRBasicBlock* block) { successors_.push_back(block); }
    void add_predecessor(IRBasicBlock* block) { predecessors_.push_back(block); }
    const std::vector<IRBasicBlock*>& get_successors() const { return successors_; }
    const std::vector<IRBasicBlock*>& get_predecessors() const { return predecessors_; }
    
    // Terminator
    IRInstruction* get_terminator() const { 
        return instructions_.empty() ? nullptr : instructions_.back(); 
    }
    void set_terminator(IRInstruction* term) { terminator_ = term; }
    
private:
    IRFunction* parent_;
    std::string name_;
    size_t id_;
    std::vector<IRInstruction*> instructions_;
    IRInstruction* terminator_;
    std::vector<IRBasicBlock*> successors_;
    std::vector<IRBasicBlock*> predecessors_;
    static size_t block_counter_;
};

/**
 * @brief IR Function
 */
class IRFunction {
public:
    explicit IRFunction(IRModule* parent, std::string name, Type* return_type);
    ~IRFunction();
    
    IRModule* get_parent() const { return parent_; }
    const std::string& get_name() const { return name_; }
    Type* get_return_type() const { return return_type_; }
    
    void add_basic_block(IRBasicBlock* block);
    const std::vector<IRBasicBlock*>& get_basic_blocks() const { return blocks_; }
    IRBasicBlock* get_entry_block() const { return blocks_.empty() ? nullptr : blocks_.front(); }
    
    void add_argument(Argument* arg) { arguments_.push_back(arg); }
    const std::vector<Argument*>& get_arguments() const { return arguments_; }
    
    // Symbol table
    IRValue* find_local(std::string_view name) const;
    void add_local(std::string name, IRValue* value);
    
private:
    IRModule* parent_;
    std::string name_;
    Type* return_type_;
    std::vector<IRBasicBlock*> blocks_;
    std::vector<Argument*> arguments_;
    std::unordered_map<std::string, IRValue*> locals_;
    static size_t function_counter_;
};

/**
 * @brief IR Module - collection of functions
 */
class IRModule {
public:
    IRModule();
    ~IRModule();
    
    void add_function(IRFunction* func);
    const std::vector<IRFunction*>& get_functions() const { return functions_; }
    IRFunction* get_function(std::string_view name) const;
    
    // Global variables
    void add_global(std::string name, IRValue* value);
    IRValue* find_global(std::string_view name) const;
    
    // String constants
    size_t add_string_constant(std::string value);
    const std::vector<std::string>& get_string_constants() const { return string_constants_; }
    
    // Name mangling
    std::string get_unique_name(std::string base);
    
private:
    std::vector<IRFunction*> functions_;
    std::unordered_map<std::string, IRFunction*> function_map_;
    std::unordered_map<std::string, IRValue*> globals_;
    std::vector<std::string> string_constants_;
    size_t name_counter_;
};

/**
 * @brief IR Builder - constructs IR from AST
 */
class IRBuilder {
public:
    explicit IRBuilder(IRModule* module);
    ~IRBuilder();
    
    // Module access
    IRModule* get_module() { return module_; }
    
    // Current builder position
    IRBasicBlock* get_current_block() const { return current_block_; }
    void set_current_block(IRBasicBlock* block) { current_block_ = block; }
    
    // Create instructions
    IRInstruction* create_nop(Type* type);
    IRInstruction* create_binary(IROpcode op, IRValue* left, IRValue* right, Type* result_type);
    IRInstruction* create_unary(IROpcode op, IRValue* operand, Type* result_type);
    IRInstruction* create_cmp(CmpPredicate pred, IRValue* left, IRValue* right);
    IRInstruction* create_call(IRFunction* callee, const std::vector<IRValue*>& args, Type* result_type);
    IRInstruction* create_alloca(Type* alloc_type);
    IRInstruction* create_load(IRValue* addr, Type* load_type);
    IRInstruction* create_store(IRValue* value, IRValue* addr);
    IRInstruction* create_br(IRBasicBlock* target);
    IRInstruction* create_br_cond(IRValue* cond, IRBasicBlock* true_block, IRBasicBlock* false_block);
    IRInstruction* create_ret(IRValue* value);
    IRInstruction* create_ret_void();
    
    // System commands (soul intrinsics)
    IRInstruction* create_syscmd_origin(IRValue* entity_type, Type* result_type);
    IRInstruction* create_syscmd_flow(IRValue* source, IRValue* target, Type* result_type);
    IRInstruction* create_syscmd_absorb(IRValue* absorber, IRValue* absorbed, Type* result_type);
    IRInstruction* create_syscmd_manifest(IRValue* soul, Type* result_type);
    
    // Constants
    ConstantInt* get_int32(int32_t value);
    ConstantInt* get_int64(int64_t value);
    ConstantFloat* get_float32(float value);
    ConstantFloat* get_float64(double value);
    ConstantBool* get_bool(bool value);
    ConstantString* get_string(const std::string& value);
    
    // Types
    Type* get_void_type() { return Type::get_void(); }
    Type* get_bool_type() { return Type::get_bool(); }
    Type* get_int32_type() { return Type::get_int32(); }
    Type* get_int64_type() { return Type::get_int64(); }
    Type* get_float32_type() { return Type::get_float32(); }
    Type* get_float64_type() { return Type::get_float64(); }
    Type* get_string_type() { return Type::get_string(); }
    Type* get_soul_type() { return Type::get_soul(); }
    Type* get_entity_type() { return Type::get_entity(); }
    
    // Helper to get or create types
    Type* get_type_from_ast(void* type_expr);
    
    // Block/function management
    IRBasicBlock* create_block(IRFunction* func, std::string name = "");
    IRFunction* create_function(std::string name, Type* return_type);
    
private:
    IRModule* module_;
    IRBasicBlock* current_block_;
    std::vector<IRInstruction*> inst_stack_;
    
    // Cached constants
    std::unordered_map<int32_t, ConstantInt*> int32_cache_;
    std::unordered_map<int64_t, ConstantInt*> int64_cache_;
    std::unordered_map<float, ConstantFloat*> float32_cache_;
    std::unordered_map<double, ConstantFloat*> float64_cache_;
};

/**
 * @brief Convert IROpcode to string
 */
inline const char* ir_opcode_to_string(IROpcode op) {
    switch (op) {
        case IROpcode::NOP: return "nop";
        case IROpcode::BR: return "br";
        case IROpcode::BR_COND: return "br_cond";
        case IROpcode::RET: return "ret";
        case IROpcode::PHI: return "phi";
        case IROpcode::ALLOCA: return "alloca";
        case IROpcode::LOAD: return "load";
        case IROpcode::STORE: return "store";
        case IROpcode::ZEXT: return "zext";
        case IROpcode::SEXT: return "sext";
        case IROpcode::TRUNC: return "trunc";
        case IROpcode::ADD: return "add";
        case IROpcode::SUB: return "sub";
        case IROpcode::MUL: return "mul";
        case IROpcode::DIV: return "div";
        case IROpcode::MOD: return "mod";
        case IROpcode::NEG: return "neg";
        case IROpcode::AND: return "and";
        case IROpcode::OR: return "or";
        case IROpcode::XOR: return "xor";
        case IROpcode::NOT: return "not";
        case IROpcode::SHL: return "shl";
        case IROpcode::SHR: return "shr";
        case IROpcode::CMP_EQ: return "eq";
        case IROpcode::CMP_NE: return "ne";
        case IROpcode::CMP_SGT: return "sgt";
        case IROpcode::CMP_SGE: return "sge";
        case IROpcode::CMP_SLT: return "slt";
        case IROpcode::CMP_SLE: return "sle";
        case IROpcode::CALL: return "call";
        case IROpcode::SYSCMD_ORIGIN: return "@origin";
        case IROpcode::SYSCMD_FLOW: return "@flow";
        case IROpcode::SYSCMD_ABSORB: return "@absorb";
        case IROpcode::SYSCMD_MANIFEST: return "@manifest";
        case IROpcode::PRINT: return "print";
        default: return "invalid";
    }
}

} // namespace ir
} // namespace veldocra

