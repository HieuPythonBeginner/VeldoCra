/**
 * @file ir.cpp
 * @brief VeldoCra SSA IR Implementation
 * @author Dr. Bright
 * 
 * Implements the SSA Intermediate Representation with:
 * - Soul-based type intrinsics
 * - System command support (origin, flow)
 * - Basic optimizations
 */

#include "middle/ir/ir.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace veldocra {
namespace ir {

// Static counters
size_t IRBasicBlock::block_counter_ = 0;
size_t IRFunction::function_counter_ = 0;

// ============================================================================
// Type Implementations
// ============================================================================

class VoidType : public Type {
public:
    VoidType() : Type(TypeKind::Void) { size_ = 0; align_ = 1; }
    std::string to_string() const override { return "void"; }
};

class BoolType : public Type {
public:
    BoolType() : Type(TypeKind::Bool) { size_ = 1; align_ = 1; }
    std::string to_string() const override { return "bool"; }
};

class CharType : public Type {
public:
    CharType() : Type(TypeKind::Char) { size_ = 1; align_ = 1; }
    std::string to_string() const override { return "char"; }
};

class Int32Type : public Type {
public:
    Int32Type() : Type(TypeKind::Int32) { size_ = 4; align_ = 4; }
    std::string to_string() const override { return "i32"; }
};

class Int64Type : public Type {
public:
    Int64Type() : Type(TypeKind::Int64) { size_ = 8; align_ = 8; }
    std::string to_string() const override { return "i64"; }
};

class Float32Type : public Type {
public:
    Float32Type() : Type(TypeKind::Float32) { size_ = 4; align_ = 4; }
    std::string to_string() const override { return "f32"; }
};

class Float64Type : public Type {
public:
    Float64Type() : Type(TypeKind::Float64) { size_ = 8; align_ = 8; }
    std::string to_string() const override { return "f64"; }
};

class StringType : public Type {
public:
    StringType() : Type(TypeKind::String) { size_ = 8; align_ = 8; }
    std::string to_string() const override { return "string"; }
};

// Soul-based types
class SoulType : public Type {
public:
    SoulType() : Type(TypeKind::Soul) { size_ = 16; align_ = 8; }
    std::string to_string() const override { return "soul"; }
};

class EntityType : public Type {
public:
    EntityType() : Type(TypeKind::Entity) { size_ = 24; align_ = 8; }
    std::string to_string() const override { return "entity"; }
};

class PointerType : public Type {
public:
    PointerType(Type* element) : Type(TypeKind::Pointer), element_(element) { 
        size_ = 8; align_ = 8; 
    }
    Type* get_element_type() const { return element_; }
    std::string to_string() const override { 
        return element_->to_string() + "*"; 
    }
private:
    Type* element_;
};

// Static type instances
static VoidType g_void_type;
static BoolType g_bool_type;
static CharType g_char_type;
static Int32Type g_int32_type;
static Int64Type g_int64_type;
static Float32Type g_float32_type;
static Float64Type g_float64_type;
static StringType g_string_type;
static SoulType g_soul_type;
static EntityType g_entity_type;
static std::vector<std::unique_ptr<PointerType>> g_pointer_types;

Type* Type::get_void() { return &g_void_type; }
Type* Type::get_bool() { return &g_bool_type; }
Type* Type::get_char() { return &g_char_type; }
Type* Type::get_int32() { return &g_int32_type; }
Type* Type::get_int64() { return &g_int64_type; }
Type* Type::get_float32() { return &g_float32_type; }
Type* Type::get_float64() { return &g_float64_type; }
Type* Type::get_string() { return &g_string_type; }
Type* Type::get_soul() { return &g_soul_type; }
Type* Type::get_entity() { return &g_entity_type; }

Type* Type::get_pointer_to(Type* element_type) {
    // Simple cache - in production would use arena
    for (auto& ptr : g_pointer_types) {
        if (ptr->get_element_type() == element_type) {
            return ptr.get();
        }
    }
    g_pointer_types.push_back(std::make_unique<PointerType>(element_type));
    return g_pointer_types.back().get();
}

// ============================================================================
// IRValue Implementations
// ============================================================================

std::string ConstantInt::to_string() const {
    return std::to_string(value_);
}

std::string ConstantFloat::to_string() const {
    std::ostringstream oss;
    oss << value_;
    return oss.str();
}

std::string ConstantBool::to_string() const {
    return value_ ? "true" : "false";
}

std::string ConstantString::to_string() const {
    return "\"" + value_ + "\"";
}

std::string Argument::to_string() const {
    return "%" + name_;
}

// ============================================================================
// IRInstruction Implementation
// ============================================================================

IRInstruction::IRInstruction(IROpcode opcode, Type* type, IRBasicBlock* parent)
    : IRValue(ValueKind::Instruction, type), opcode_(opcode), parent_(parent), id_(0) {}

std::string IRInstruction::to_string() const {
    std::ostringstream oss;
    oss << "%" << id_ << " = " << ir_opcode_to_string(opcode_);
    for (size_t i = 0; i < operands_.size(); i++) {
        if (i > 0) oss << ", ";
        if (operands_[i]) {
            // Handle different value kinds - use kind check instead of dynamic_cast
            if (operands_[i]->get_kind() == IRValue::ValueKind::Argument) {
                oss << operands_[i]->to_string();
            } else if (operands_[i]->get_kind() == IRValue::ValueKind::Instruction) {
                // Cast using static since we know it's an instruction
                auto* inst = static_cast<const IRInstruction*>(operands_[i]);
                oss << "%" << inst->get_id();
            } else {
                oss << operands_[i]->to_string();
            }
        } else {
            oss << "null";
        }
    }
    return oss.str();
}

// ============================================================================
// PhiInst Implementation
// ============================================================================

PhiInst::PhiInst(Type* type, IRBasicBlock* parent)
    : IRInstruction(IROpcode::PHI, type, parent) {}

void PhiInst::add_incoming(IRValue* value, IRBasicBlock* block) {
    incomings_.push_back({value, block});
}

std::vector<std::pair<IRValue*, IRBasicBlock*>> PhiInst::get_incomings() const {
    return incomings_;
}

std::string PhiInst::to_string() const {
    std::ostringstream oss;
    oss << "%" << id_ << " = phi " << type_->to_string();
    for (size_t i = 0; i < incomings_.size(); i++) {
        if (i > 0) oss << ", ";
        auto* block = incomings_[i].second;
        oss << "[" << (incomings_[i].first ? incomings_[i].first->to_string() : "undef") 
            << ", " << block->get_name() << "]";
    }
    return oss.str();
}

// ============================================================================
// IRBasicBlock Implementation
// ============================================================================

IRBasicBlock::IRBasicBlock(IRFunction* parent, std::string name)
    : parent_(parent), name_(std::move(name)), id_(block_counter_++), terminator_(nullptr) {
    if (name_.empty()) {
        name_ = "bb_" + std::to_string(id_);
    }
}

IRBasicBlock::~IRBasicBlock() {
    // Instructions are owned by the block
    for (auto* inst : instructions_) {
        delete inst;
    }
}

void IRBasicBlock::add_instruction(IRInstruction* inst) {
    instructions_.push_back(inst);
}

// ============================================================================
// IRFunction Implementation
// ============================================================================

IRFunction::IRFunction(IRModule* parent, std::string name, Type* return_type)
    : parent_(parent), name_(std::move(name)), return_type_(return_type) {}

IRFunction::~IRFunction() {
    for (auto* block : blocks_) {
        delete block;
    }
    for (auto* arg : arguments_) {
        delete arg;
    }
}

void IRFunction::add_basic_block(IRBasicBlock* block) {
    blocks_.push_back(block);
}

IRValue* IRFunction::find_local(std::string_view name) const {
    auto it = locals_.find(std::string(name));
    if (it != locals_.end()) {
        return it->second;
    }
    return nullptr;
}

void IRFunction::add_local(std::string name, IRValue* value) {
    locals_[std::move(name)] = value;
}

// ============================================================================
// IRModule Implementation
// ============================================================================

IRModule::IRModule() : name_counter_(0) {}

IRModule::~IRModule() {
    for (auto* func : functions_) {
        delete func;
    }
}

void IRModule::add_function(IRFunction* func) {
    functions_.push_back(func);
    function_map_[func->get_name()] = func;
}

IRFunction* IRModule::get_function(std::string_view name) const {
    auto it = function_map_.find(std::string(name));
    if (it != function_map_.end()) {
        return it->second;
    }
    return nullptr;
}

void IRModule::add_global(std::string name, IRValue* value) {
    globals_[std::move(name)] = value;
}

IRValue* IRModule::find_global(std::string_view name) const {
    auto it = globals_.find(std::string(name));
    if (it != globals_.end()) {
        return it->second;
    }
    return nullptr;
}

size_t IRModule::add_string_constant(std::string value) {
    string_constants_.push_back(std::move(value));
    return string_constants_.size() - 1;
}

std::string IRModule::get_unique_name(std::string base) {
    return base + "_" + std::to_string(name_counter_++);
}

// ============================================================================
// IRBuilder Implementation
// ============================================================================

IRBuilder::IRBuilder(IRModule* module) : module_(module), current_block_(nullptr) {}

IRBuilder::~IRBuilder() {
    // Clean up cached constants
    for (auto& [key, val] : int32_cache_) delete val;
    for (auto& [key, val] : int64_cache_) delete val;
    for (auto& [key, val] : float32_cache_) delete val;
    for (auto& [key, val] : float64_cache_) delete val;
}

IRInstruction* IRBuilder::create_nop(Type* type) {
    auto* inst = new IRInstruction(IROpcode::NOP, type, current_block_);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_binary(IROpcode op, IRValue* left, IRValue* right, Type* result_type) {
    IRInstruction* inst = nullptr;
    
    // Map AST ops to IR ops
    IROpcode ir_op;
    switch (static_cast<uint16_t>(op)) {
        case 1: ir_op = IROpcode::ADD; break;  // Add
        case 2: ir_op = IROpcode::SUB; break;  // Sub
        case 3: ir_op = IROpcode::MUL; break;  // Mul
        case 4: ir_op = IROpcode::DIV; break;  // Div
        case 5: ir_op = IROpcode::MOD; break;  // Mod
        default: ir_op = IROpcode::ADD; break;
    }
    
    inst = new IRInstruction(ir_op, result_type, current_block_);
    inst->add_operand(left);
    inst->add_operand(right);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_unary(IROpcode op, IRValue* operand, Type* result_type) {
    auto* inst = new IRInstruction(op, result_type, current_block_);
    inst->add_operand(operand);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_cmp(CmpPredicate pred, IRValue* left, IRValue* right) {
    IROpcode op;
    switch (pred) {
        case CmpPredicate::EQ: op = IROpcode::CMP_EQ; break;
        case CmpPredicate::NE: op = IROpcode::CMP_NE; break;
        case CmpPredicate::SGT: op = IROpcode::CMP_SGT; break;
        case CmpPredicate::SGE: op = IROpcode::CMP_SGE; break;
        case CmpPredicate::SLT: op = IROpcode::CMP_SLT; break;
        case CmpPredicate::SLE: op = IROpcode::CMP_SLE; break;
        default: op = IROpcode::CMP_EQ; break;
    }
    auto* inst = new IRInstruction(op, Type::get_bool(), current_block_);
    inst->add_operand(left);
    inst->add_operand(right);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_call(IRFunction* callee, const std::vector<IRValue*>& args, Type* result_type) {
    auto* inst = new IRInstruction(IROpcode::CALL, result_type, current_block_);
    for (auto* arg : args) {
        inst->add_operand(arg);
    }
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_alloca(Type* alloc_type) {
    auto* ptr_type = Type::get_pointer_to(alloc_type);
    auto* inst = new IRInstruction(IROpcode::ALLOCA, ptr_type, current_block_);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_load(IRValue* addr, Type* load_type) {
    auto* inst = new IRInstruction(IROpcode::LOAD, load_type, current_block_);
    inst->add_operand(addr);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_store(IRValue* value, IRValue* addr) {
    auto* inst = new IRInstruction(IROpcode::STORE, Type::get_void(), current_block_);
    inst->add_operand(value);
    inst->add_operand(addr);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_br(IRBasicBlock* target) {
    auto* inst = new IRInstruction(IROpcode::BR, Type::get_void(), current_block_);
    current_block_->add_instruction(inst);
    current_block_->set_terminator(inst);
    return inst;
}

IRInstruction* IRBuilder::create_br_cond(IRValue* cond, IRBasicBlock* true_block, IRBasicBlock* false_block) {
    auto* inst = new IRInstruction(IROpcode::BR_COND, Type::get_void(), current_block_);
    inst->add_operand(cond);
    current_block_->add_instruction(inst);
    current_block_->set_terminator(inst);
    return inst;
}

IRInstruction* IRBuilder::create_ret(IRValue* value) {
    auto* inst = new IRInstruction(IROpcode::RET, value->get_type(), current_block_);
    inst->add_operand(value);
    current_block_->add_instruction(inst);
    current_block_->set_terminator(inst);
    return inst;
}

IRInstruction* IRBuilder::create_ret_void() {
    auto* inst = new IRInstruction(IROpcode::RET, Type::get_void(), current_block_);
    current_block_->add_instruction(inst);
    current_block_->set_terminator(inst);
    return inst;
}

// ============================================================================
// System Command (Soul Intrinsics) Implementations
// ============================================================================

IRInstruction* IRBuilder::create_syscmd_origin(IRValue* entity_type, Type* result_type) {
    // @origin - Create soul origin
    auto* inst = new IRInstruction(IROpcode::SYSCMD_ORIGIN, result_type, current_block_);
    inst->add_operand(entity_type);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_syscmd_flow(IRValue* source, IRValue* target, Type* result_type) {
    // @flow - Control soul flow
    auto* inst = new IRInstruction(IROpcode::SYSCMD_FLOW, result_type, current_block_);
    inst->add_operand(source);
    inst->add_operand(target);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_syscmd_absorb(IRValue* absorber, IRValue* absorbed, Type* result_type) {
    // @absorb - Absorb entity
    auto* inst = new IRInstruction(IROpcode::SYSCMD_ABSORB, result_type, current_block_);
    inst->add_operand(absorber);
    inst->add_operand(absorbed);
    current_block_->add_instruction(inst);
    return inst;
}

IRInstruction* IRBuilder::create_syscmd_manifest(IRValue* soul, Type* result_type) {
    // @manifest - Manifest soul
    auto* inst = new IRInstruction(IROpcode::SYSCMD_MANIFEST, result_type, current_block_);
    inst->add_operand(soul);
    current_block_->add_instruction(inst);
    return inst;
}

// ============================================================================
// Constants
// ============================================================================

ConstantInt* IRBuilder::get_int32(int32_t value) {
    auto it = int32_cache_.find(value);
    if (it != int32_cache_.end()) {
        return it->second;
    }
    auto* c = new ConstantInt(value, Type::get_int32());
    int32_cache_[value] = c;
    return c;
}

ConstantInt* IRBuilder::get_int64(int64_t value) {
    auto it = int64_cache_.find(value);
    if (it != int64_cache_.end()) {
        return it->second;
    }
    auto* c = new ConstantInt(value, Type::get_int64());
    int64_cache_[value] = c;
    return c;
}

ConstantFloat* IRBuilder::get_float32(float value) {
    auto it = float32_cache_.find(value);
    if (it != float32_cache_.end()) {
        return it->second;
    }
    auto* c = new ConstantFloat(value, Type::get_float32());
    float32_cache_[value] = c;
    return c;
}

ConstantFloat* IRBuilder::get_float64(double value) {
    auto it = float64_cache_.find(value);
    if (it != float64_cache_.end()) {
        return it->second;
    }
    auto* c = new ConstantFloat(value, Type::get_float64());
    float64_cache_[value] = c;
    return c;
}

ConstantBool* IRBuilder::get_bool(bool value) {
    static ConstantBool true_val(true, Type::get_bool());
    static ConstantBool false_val(false, Type::get_bool());
    return value ? &true_val : &false_val;
}

ConstantString* IRBuilder::get_string(const std::string& value) {
    auto* c = new ConstantString(value, Type::get_string());
    return c;
}

// ============================================================================
// Block and Function Management
// ============================================================================

IRBasicBlock* IRBuilder::create_block(IRFunction* func, std::string name) {
    auto* block = new IRBasicBlock(func, std::move(name));
    func->add_basic_block(block);
    return block;
}

IRFunction* IRBuilder::create_function(std::string name, Type* return_type) {
    auto* func = new IRFunction(module_, std::move(name), return_type);
    module_->add_function(func);
    return func;
}

Type* IRBuilder::get_type_from_ast(void* type_expr) {
    // Placeholder - would integrate with actual type system
    (void)type_expr;
    return Type::get_int32();
}

// ============================================================================
// IR Printer (for debugging)
// ============================================================================

void print_module(IRModule* module) {
    std::cout << "; Module\n";
    for (auto* func : module->get_functions()) {
        std::cout << "define " << func->get_return_type()->to_string() 
                  << " @" << func->get_name() << "() {\n";
        
        for (auto* block : func->get_basic_blocks()) {
            std::cout << block->get_name() << ":\n";
            for (auto* inst : block->get_instructions()) {
                std::cout << "  " << inst->to_string() << "\n";
            }
        }
        std::cout << "}\n\n";
    }
}

} // namespace ir
} // namespace veldocra

