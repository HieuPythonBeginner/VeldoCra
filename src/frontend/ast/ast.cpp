/**
 * @file ast.cpp
 * @brief VeldoCra AST Implementation with Arena Allocator
 * @author Dr. Bright
 */

#include "ast.h"
#include <cstring>
#include <iostream>
#include <sstream>

namespace veldanava {
namespace ast {

// ============================================================================
// Arena Allocator Implementation
// ============================================================================

/**
 * @brief Construct arena with specified chunk size
 */
ArenaAllocator::ArenaAllocator(size_t chunk_size)
    : chunk_size_(chunk_size)
    , current_chunk_(nullptr)
    , total_allocated_(0)
    , total_used_(0) {
    allocate_chunk();
}

/**
 * @brief Destructor - free all chunks
 */
ArenaAllocator::~ArenaAllocator() {
    reset();
}

/**
 * @brief Move constructor
 */
ArenaAllocator::ArenaAllocator(ArenaAllocator&& other) noexcept
    : chunk_size_(other.chunk_size_)
    , current_chunk_(other.current_chunk_)
    , total_allocated_(other.total_allocated_)
    , total_used_(other.total_used_) {
    other.current_chunk_ = nullptr;
    other.total_allocated_ = 0;
    other.total_used_ = 0;
}

/**
 * @brief Move assignment
 */
ArenaAllocator& ArenaAllocator::operator=(ArenaAllocator&& other) noexcept {
    if (this != &other) {
        reset();
        chunk_size_ = other.chunk_size_;
        current_chunk_ = other.current_chunk_;
        total_allocated_ = other.total_allocated_;
        total_used_ = other.total_used_;
        other.current_chunk_ = nullptr;
        other.total_allocated_ = 0;
        other.total_used_ = 0;
    }
    return *this;
}

/**
 * @brief Allocate new chunk
 */
void ArenaAllocator::allocate_chunk() const {
    char* data = new char[chunk_size_];
    
    Chunk* chunk = new Chunk();
    chunk->data = data;
    chunk->size = chunk_size_;
    chunk->used = 0;
    chunk->next = current_chunk_;
    
    current_chunk_ = chunk;
    total_allocated_ += chunk_size_;
}

/**
 * @brief Allocate memory from arena
 */
void* ArenaAllocator::allocate(size_t size, size_t alignment) const {
    // Align current position
    uintptr_t addr = reinterpret_cast<uintptr_t>(current_chunk_->data + current_chunk_->used);
    uintptr_t aligned = (addr + alignment - 1) & ~(alignment - 1);
    size_t offset = aligned - reinterpret_cast<uintptr_t>(current_chunk_->data);
    
    // Check if we need a new chunk
    if (offset + size > current_chunk_->size) {
        allocate_chunk();
        return allocate(size, alignment);
    }
    
    current_chunk_->used = offset + size;
    total_used_ += size;
    
    return reinterpret_cast<void*>(aligned);
}

/**
 * @brief Reset arena (deallocate all chunks)
 */
void ArenaAllocator::reset() {
    // Free all chunks
    while (current_chunk_) {
        Chunk* next = current_chunk_->next;
        delete[] current_chunk_->data;
        delete current_chunk_;
        current_chunk_ = next;
    }
    
    current_chunk_ = nullptr;
    total_used_ = 0;
    
    // Allocate fresh chunk
    allocate_chunk();
}

// ============================================================================
// AST Builder Implementation
// ============================================================================

/**
 * @brief Construct AST builder
 */
ASTBuilder::ASTBuilder()
    : arena_(1024 * 1024)  // 1MB default
    , error_count_(0) {
}

/**
 * @brief Destructor
 */
ASTBuilder::~ASTBuilder() = default;

/**
 * @brief Move constructor
 */
ASTBuilder::ASTBuilder(ASTBuilder&& other) noexcept
    : arena_(std::move(other.arena_))
    , error_count_(other.error_count_) {
    other.error_count_ = 0;
}

/**
 * @brief Move assignment
 */
ASTBuilder& ASTBuilder::operator=(ASTBuilder&& other) noexcept {
    if (this != &other) {
        arena_ = std::move(other.arena_);
        error_count_ = other.error_count_;
        other.error_count_ = 0;
    }
    return *this;
}

/**
 * @brief Allocate string in arena
 */
const char* ASTBuilder::alloc_string(std::string_view str) const {
    char* buffer = static_cast<char*>(arena_.allocate(str.size() + 1, 1));
    std::memcpy(buffer, str.data(), str.size());
    buffer[str.size()] = '\0';
    return buffer;
}

/**
 * @brief Report error
 */
void ASTBuilder::report_error(std::string_view message) {
    error_count_++;
    std::cerr << "AST Error: " << message << "\n";
}

/**
 * @brief Intern a string in the arena (allocate null-terminated copy)
 */
const char* ASTBuilder::intern_string(std::string_view str) const {
    return alloc_string(str);
}

// ============================================================================
// Literal Expression Creation
// ============================================================================

LiteralExprNode* ASTBuilder::create_literal(int64_t value) {
    auto* node = arena_.create<LiteralExprNode>();
    node->kind = NodeKind::LiteralExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->literal_type = 0;    // int
    node->value.int_val = value;
    return node;
}

LiteralExprNode* ASTBuilder::create_literal(double value) {
    auto* node = arena_.create<LiteralExprNode>();
    node->kind = NodeKind::LiteralExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->literal_type = 1;    // float
    node->value.float_val = value;
    return node;
}

LiteralExprNode* ASTBuilder::create_literal(bool value) {
    auto* node = arena_.create<LiteralExprNode>();
    node->kind = NodeKind::LiteralExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->literal_type = 3;    // bool
    node->value.bool_val = value;
    return node;
}

LiteralExprNode* ASTBuilder::create_literal(std::string_view value) {
    auto* node = arena_.create<LiteralExprNode>();
    node->kind = NodeKind::LiteralExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->literal_type = 2;    // string
    node->value.string_val = alloc_string(value);
    return node;
}

LiteralExprNode* ASTBuilder::create_literal(char value) {
    auto* node = arena_.create<LiteralExprNode>();
    node->kind = NodeKind::LiteralExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->literal_type = 4;    // char
    node->value.char_val = value;
    return node;
}

LiteralExprNode* ASTBuilder::create_none() {
    auto* node = arena_.create<LiteralExprNode>();
    node->kind = NodeKind::LiteralExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->literal_type = 5;    // none
    node->value.none_val = nullptr;
    return node;
}

// Variable expression
VariableExprNode* ASTBuilder::create_variable(const char* name) {
    auto* node = arena_.create<VariableExprNode>();
    node->kind = NodeKind::VariableExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 1;  // lvalue
    node->name = alloc_string(name);
    return node;
}

TypeExprNode* ASTBuilder::create_type_expr(const char* type_name, std::vector<Expr*> type_params) {
    auto* node = arena_.create<TypeExprNode>();
    node->kind = NodeKind::TypeExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 1;  // lvalue
    node->type_name = alloc_string(type_name);
    node->type_params = type_params;
    return node;
}

FStringExprNode* ASTBuilder::create_fstring() {
    auto* node = arena_.create<FStringExprNode>();
    node->kind = NodeKind::FStringExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    return node;
}

// Member expression (ego.field, core.field)
MemberExprNode* ASTBuilder::create_member(Expr* object, const char* member, bool is_ego) {
    auto* node = arena_.create<MemberExprNode>();
    node->kind = NodeKind::MemberExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 1;  // lvalue
    node->object = object;
    node->member = alloc_string(member);
    node->is_ego = is_ego;
    return node;
}

// Binary expression
BinaryExprNode* ASTBuilder::create_binary(Expr* left, uint16_t op, Expr* right) {
    auto* node = arena_.create<BinaryExprNode>();
    node->kind = NodeKind::BinaryExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->left = left;
    node->right = right;
    node->op = op;
    return node;
}

// Unary expression
UnaryExprNode* ASTBuilder::create_unary(uint16_t op, Expr* operand) {
    auto* node = arena_.create<UnaryExprNode>();
    node->kind = NodeKind::UnaryExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->operand = operand;
    node->op = op;
    return node;
}

// Call expression
CallExprNode* ASTBuilder::create_call(Expr* callee) {
    auto* node = arena_.create<CallExprNode>();
    node->kind = NodeKind::CallExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->callee = callee;
    node->args = nullptr;
    node->arg_count = 0;
    node->arg_capacity = 0;
    return node;
}

void ASTBuilder::add_argument(CallExprNode* call, Expr* arg) {
    if (call->arg_count >= call->arg_capacity) {
        size_t new_cap = call->arg_capacity == 0 ? 4 : call->arg_capacity * 2;
        Expr** new_args = static_cast<Expr**>(arena_.allocate(new_cap * sizeof(Expr*), alignof(Expr*)));
        if (call->args) {
            std::memcpy(new_args, call->args, call->arg_count * sizeof(Expr*));
        }
        call->args = new_args;
        call->arg_capacity = new_cap;
    }
    call->args[call->arg_count++] = arg;
}

// Index expression
IndexExprNode* ASTBuilder::create_index(Expr* object, Expr* index) {
    auto* node = arena_.create<IndexExprNode>();
    node->kind = NodeKind::IndexExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->object = object;
    node->index = index;
    return node;
}

// Block statement
BlockStmtNode* ASTBuilder::create_block() {
    auto* node = arena_.create<BlockStmtNode>();
    node->kind = NodeKind::BlockStmt;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->statements = nullptr;
    node->statement_count = 0;
    node->statement_capacity = 0;
    return node;
}

void ASTBuilder::add_statement(BlockStmtNode* block, Stmt* stmt) {
    if (block->statement_count >= block->statement_capacity) {
        size_t new_cap = block->statement_capacity == 0 ? 4 : block->statement_capacity * 2;
        Node** new_stmts = static_cast<Node**>(arena_.allocate(new_cap * sizeof(Node*), alignof(Node*)));
        if (block->statements) {
            std::memcpy(new_stmts, block->statements, block->statement_count * sizeof(Node*));
        }
        block->statements = new_stmts;
        block->statement_capacity = new_cap;
    }
    block->statements[block->statement_count++] = stmt;
}

// If statement
IfStmtNode* ASTBuilder::create_if(Expr* condition, Stmt* then_branch, Stmt* else_branch) {
    auto* node = arena_.create<IfStmtNode>();
    node->kind = NodeKind::IfStmt;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->condition = condition;
    node->then_branch = then_branch;
    node->else_branch = else_branch;
    return node;
}

// While statement
WhileStmtNode* ASTBuilder::create_while(Expr* condition, Stmt* body) {
    auto* node = arena_.create<WhileStmtNode>();
    node->kind = NodeKind::WhileStmt;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->condition = condition;
    node->body = body;
    return node;
}

// For statement
ForStmtNode* ASTBuilder::create_for(const char* iterator, Expr* iterable, Stmt* body) {
    auto* node = arena_.create<ForStmtNode>();
    node->kind = NodeKind::ForStmt;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->iterator_name = alloc_string(iterator);
    node->iterable = iterable;
    node->body = body;
    return node;
}

// Return statement
ReturnStmtNode* ASTBuilder::create_return(Expr* value) {
    auto* node = arena_.create<ReturnStmtNode>();
    node->kind = NodeKind::ReturnStmt;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value = value;
    return node;
}

Stmt* ASTBuilder::create_expr_stmt(Expr* expr) {
    auto* node = arena_.create<ExprStmtNode>();
    node->kind = NodeKind::ExprStmt;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->expr = expr;
    return node;
}

// Let statement
LetStmtNode* ASTBuilder::create_let(const char* name, Expr* init, bool mutable_flag, Expr* type_expr) {
    auto* node = arena_.create<LetStmtNode>();
    node->kind = NodeKind::LetStmt;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->name = alloc_string(name);
    node->type_expr = type_expr;
    node->initializer = init;
    node->is_mutable = mutable_flag;
    return node;
}

// Function statement
FnStmtNode* ASTBuilder::create_fn(const char* name) {
    auto* node = arena_.create<FnStmtNode>();
    node->kind = NodeKind::FnStmt;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->name = alloc_string(name);
    node->params = nullptr;
    node->param_count = 0;
    node->body = nullptr;
    node->return_type = nullptr;
    return node;
}

void ASTBuilder::add_param(FnStmtNode* fn, ParamDeclNode* param) {
    std::cerr << "[ADD_PARAM] called, fn=" << (fn ? fn->name : "null") << " param=" << (param && param->name ? param->name : "null") << std::endl;
    // Expand params array if needed (simplified - in production would use arena)
    ParamDeclNode** new_params = static_cast<ParamDeclNode**>(
        arena_.allocate((fn->param_count + 1) * sizeof(ParamDeclNode*), alignof(ParamDeclNode*)));
    if (fn->params) {
        std::memcpy(new_params, fn->params, fn->param_count * sizeof(ParamDeclNode*));
    }
    new_params[fn->param_count++] = param;
    fn->params = new_params;
}

void ASTBuilder::set_fn_body(FnStmtNode* fn, Stmt* body) {
    fn->body = body;
}

void ASTBuilder::set_fn_return_type(FnStmtNode* fn, Expr* return_type) {
    fn->return_type = return_type;
}

// ============================================================================
// Pattern Creation Functions
// ============================================================================

WildcardPatternNode* ASTBuilder::create_wildcard_pattern() {
    auto* node = arena_.create<WildcardPatternNode>();
    node->kind = NodeKind::Pattern;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->pattern_kind = PatternKind::Wildcard;
    return node;
}

LiteralPatternNode* ASTBuilder::create_literal_pattern(LiteralExprNode* value) {
    auto* node = arena_.create<LiteralPatternNode>();
    node->kind = NodeKind::Pattern;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->pattern_kind = PatternKind::Literal;
    node->value = value;
    return node;
}

VariablePatternNode* ASTBuilder::create_variable_pattern(const char* name, PatternNode* subpattern) {
    auto* node = arena_.create<VariablePatternNode>();
    node->kind = NodeKind::Pattern;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->pattern_kind = PatternKind::Variable;
    node->variable_name = alloc_string(name);
    node->subpattern = subpattern;
    return node;
}

ConstructorPatternNode* ASTBuilder::create_constructor_pattern(const char* constructor_name) {
    auto* node = arena_.create<ConstructorPatternNode>();
    node->kind = NodeKind::Pattern;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->pattern_kind = PatternKind::Constructor;
    node->constructor_name = alloc_string(constructor_name);
    node->args = nullptr;
    node->arg_count = 0;
    node->arg_capacity = 0;
    return node;
}

void ASTBuilder::add_constructor_arg(ConstructorPatternNode* pattern, PatternNode* arg) {
    if (pattern->arg_count >= pattern->arg_capacity) {
        size_t new_cap = pattern->arg_capacity == 0 ? 4 : pattern->arg_capacity * 2;
        PatternNode** new_args = static_cast<PatternNode**>(
            arena_.allocate(new_cap * sizeof(PatternNode*), alignof(PatternNode*)));
        if (pattern->args) {
            std::memcpy(new_args, pattern->args, pattern->arg_count * sizeof(PatternNode*));
        }
        pattern->args = new_args;
        pattern->arg_capacity = new_cap;
    }
    pattern->args[pattern->arg_count++] = arg;
}

TuplePatternNode* ASTBuilder::create_tuple_pattern() {
    auto* node = arena_.create<TuplePatternNode>();
    node->kind = NodeKind::Pattern;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->pattern_kind = PatternKind::Tuple;
    node->elements = nullptr;
    node->element_count = 0;
    node->element_capacity = 0;
    return node;
}

void ASTBuilder::add_tuple_element(TuplePatternNode* pattern, PatternNode* element) {
    if (pattern->element_count >= pattern->element_capacity) {
        size_t new_cap = pattern->element_capacity == 0 ? 4 : pattern->element_capacity * 2;
        PatternNode** new_elements = static_cast<PatternNode**>(
            arena_.allocate(new_cap * sizeof(PatternNode*), alignof(PatternNode*)));
        if (pattern->elements) {
            std::memcpy(new_elements, pattern->elements, pattern->element_count * sizeof(PatternNode*));
        }
        pattern->elements = new_elements;
        pattern->element_capacity = new_cap;
    }
    pattern->elements[pattern->element_count++] = element;
}

ArrayPatternNode* ASTBuilder::create_array_pattern() {
    auto* node = arena_.create<ArrayPatternNode>();
    node->kind = NodeKind::Pattern;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->pattern_kind = PatternKind::Array;
    node->elements = nullptr;
    node->element_count = 0;
    node->element_capacity = 0;
    node->rest_pattern = nullptr;
    return node;
}

void ASTBuilder::add_array_element(ArrayPatternNode* pattern, PatternNode* element) {
    if (pattern->element_count >= pattern->element_capacity) {
        size_t new_cap = pattern->element_capacity == 0 ? 4 : pattern->element_capacity * 2;
        PatternNode** new_elements = static_cast<PatternNode**>(
            arena_.allocate(new_cap * sizeof(PatternNode*), alignof(PatternNode*)));
        if (pattern->elements) {
            std::memcpy(new_elements, pattern->elements, pattern->element_count * sizeof(PatternNode*));
        }
        pattern->elements = new_elements;
        pattern->element_capacity = new_cap;
    }
    pattern->elements[pattern->element_count++] = element;
}

void ASTBuilder::set_rest_pattern(ArrayPatternNode* pattern, PatternNode* rest) {
    pattern->rest_pattern = rest;
}

RangePatternNode* ASTBuilder::create_range_pattern(LiteralExprNode* start, LiteralExprNode* end, bool inclusive) {
    auto* node = arena_.create<RangePatternNode>();
    node->kind = NodeKind::Pattern;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->pattern_kind = PatternKind::Range;
    node->start = start;
    node->end = end;
    node->inclusive = inclusive;
    return node;
}

GuardPatternNode* ASTBuilder::create_guard_pattern(PatternNode* pattern, Expr* guard_condition) {
    auto* node = arena_.create<GuardPatternNode>();
    node->kind = NodeKind::Pattern;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->pattern_kind = PatternKind::Guard;
    node->pattern = pattern;
    node->guard_condition = guard_condition;
    return node;
}

// ============================================================================
// Match Statement Creation
// ============================================================================

MatchStmtNode* ASTBuilder::create_match(Expr* expression) {
    auto* node = arena_.create<MatchStmtNode>();
    node->kind = NodeKind::MatchStmt;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->expression = expression;
    node->arms = nullptr;
    node->arm_count = 0;
    node->arm_capacity = 0;
    return node;
}

void ASTBuilder::add_match_arm(MatchStmtNode* match, PatternNode* pattern, Stmt* body, Expr* guard) {
    if (match->arm_count >= match->arm_capacity) {
        size_t new_cap = match->arm_capacity == 0 ? 4 : match->arm_capacity * 2;
        MatchArmNode** new_arms = static_cast<MatchArmNode**>(
            arena_.allocate(new_cap * sizeof(MatchArmNode*), alignof(MatchArmNode*)));
        if (match->arms) {
            std::memcpy(new_arms, match->arms, match->arm_count * sizeof(MatchArmNode*));
        }
        match->arms = new_arms;
        match->arm_capacity = new_cap;
    }
    
    // Create arm node
    auto* arm = arena_.create<MatchArmNode>();
    arm->kind = NodeKind::MatchStmt;
    arm->line = 0;
    arm->column = 0;
    arm->type_info = nullptr;
    arm->start_offset = 0;
    arm->end_offset = 0;
    arm->pattern = pattern;
    arm->body = body;
    arm->guard = guard;
    
    match->arms[match->arm_count++] = arm;
}

// Array expression
ArrayExprNode* ASTBuilder::create_array() {
    auto* node = arena_.create<ArrayExprNode>();
    node->kind = NodeKind::ArrayExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->elements = nullptr;
    node->element_count = 0;
    node->element_capacity = 0;
    return node;
}

void ASTBuilder::add_array_element(ArrayExprNode* array, Expr* element) {
    if (array->element_count >= array->element_capacity) {
        size_t new_cap = array->element_capacity == 0 ? 4 : array->element_capacity * 2;
        Expr** new_elements = static_cast<Expr**>(
            arena_.allocate(new_cap * sizeof(Expr*), alignof(Expr*)));
        if (array->elements) {
            std::memcpy(new_elements, array->elements, array->element_count * sizeof(Expr*));
        }
        array->elements = new_elements;
        array->element_capacity = new_cap;
    }
    array->elements[array->element_count++] = element;
}

// Dictionary expression
DictExprNode* ASTBuilder::create_dict() {
    auto* node = arena_.create<DictExprNode>();
    node->kind = NodeKind::DictExpr;
    node->line = 0;
    node->column = 0;
    node->type_info = nullptr;
    node->start_offset = 0;
    node->end_offset = 0;
    node->value_category = 0;  // rvalue
    node->keys = nullptr;
    node->values = nullptr;
    node->pair_count = 0;
    node->pair_capacity = 0;
    return node;
}

void ASTBuilder::add_dict_entry(DictExprNode* dict, Expr* key, Expr* value) {
    if (dict->pair_count >= dict->pair_capacity) {
        size_t new_cap = dict->pair_capacity == 0 ? 4 : dict->pair_capacity * 2;
        Expr** new_keys = static_cast<Expr**>(
            arena_.allocate(new_cap * sizeof(Expr*), alignof(Expr*)));
        Expr** new_values = static_cast<Expr**>(
            arena_.allocate(new_cap * sizeof(Expr*), alignof(Expr*)));
        if (dict->keys) {
            std::memcpy(new_keys, dict->keys, dict->pair_count * sizeof(Expr*));
            std::memcpy(new_values, dict->values, dict->pair_count * sizeof(Expr*));
        }
        dict->keys = new_keys;
        dict->values = new_values;
        dict->pair_capacity = new_cap;
    }
    dict->keys[dict->pair_count] = key;
    dict->values[dict->pair_count] = value;
    dict->pair_count++;
}

// ============================================================================
// AST Printer Implementation
// ============================================================================

/**
 * @brief Construct AST printer
 */
ASTPrinter::ASTPrinter(std::string_view source)
    : source_(source)
    , indent_level_(0) {
}

/**
 * @brief Destructor
 */
ASTPrinter::~ASTPrinter() = default;

/**
 * @brief Print AST to stdout
 */
void ASTPrinter::print(Node* node) {
    if (!node) {
        std::cout << "(null)\n";
        return;
    }
    print_node(node);
}

/**
 * @brief Print AST to string
 */
std::string ASTPrinter::to_string(Node* node) {
    std::ostringstream oss;
    // Would implement streaming here
    return oss.str();
}

void ASTPrinter::print_indent() {
    for (int i = 0; i < indent_level_; i++) {
        std::cout << "  ";
    }
}

void ASTPrinter::print_node(Node* node) {
    if (!node) {
        print_indent();
        std::cout << "(null)\n";
        return;
    }
    
    print_indent();
    
    switch (node->kind) {
        case NodeKind::LiteralExpr:
            std::cout << "LiteralExpr\n";
            break;
        case NodeKind::VariableExpr:
            std::cout << "VariableExpr\n";
            break;
        case NodeKind::BinaryExpr:
            std::cout << "BinaryExpr\n";
            break;
        case NodeKind::UnaryExpr:
            std::cout << "UnaryExpr\n";
            break;
        case NodeKind::CallExpr:
            std::cout << "CallExpr\n";
            break;
        case NodeKind::IndexExpr:
            std::cout << "IndexExpr\n";
            break;
        case NodeKind::ArrayExpr:
            std::cout << "ArrayExpr\n";
            break;
        case NodeKind::DictExpr:
            std::cout << "DictExpr\n";
            break;
        case NodeKind::BlockStmt:
            std::cout << "BlockStmt\n";
            break;
        case NodeKind::IfStmt:
            std::cout << "IfStmt\n";
            break;
        case NodeKind::WhileStmt:
            std::cout << "WhileStmt\n";
            break;
        case NodeKind::ForStmt:
            std::cout << "ForStmt\n";
            break;
        case NodeKind::ReturnStmt:
            std::cout << "ReturnStmt\n";
            break;
        case NodeKind::LetStmt:
            std::cout << "LetStmt\n";
            break;
        case NodeKind::FnStmt:
            std::cout << "FnStmt\n";
            break;
        default:
            std::cout << "Unknown(" << static_cast<int>(node->kind) << ")\n";
            break;
    }
}

void ASTPrinter::print_expr(Expr* expr) {
    indent_level_++;
    print_node(expr);
    indent_level_--;
}

void ASTPrinter::print_stmt(Stmt* stmt) {
    indent_level_++;
    print_node(stmt);
    indent_level_--;
}

} // namespace ast
} // namespace veldanava

