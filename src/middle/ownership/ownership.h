/**
 * @file ownership.h
 * @brief VeldoCra 5-Layer Defense Ownership System
 * @author Dr. Bright
 * 
 * 5-LAYER DEFENSE SYSTEM:
 * 1. C++ Layer (Deterministic Destruction): Scope-based cleanup with RAII
 * 2. Ada Layer (Range & Constraint Guard): Static range violation detection
 * 3. Python Layer (Safe Runtime Fallback): IR Safety Hooks for dynamic checks
 * 4. Full AST Integration: Proper ast::Node* walking for use-after-move
 * 5. Rust Layer (Zero-Cost): Compile-time lifetime tracking only
 * 
 * ZERO-COST ABSTRACTION: All ownership checking happens at COMPILE TIME.
 * The final binary has ZERO runtime overhead.
 */

#pragma once
#include "frontend/ast/ast.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <stack>
#include <map>

// Forward declare AST types
namespace veldocra {
namespace ast {
    // Forward declarations - actual definitions in ast.h
    struct Node;
    struct Expr;
    struct Stmt;
    struct BlockStmtNode;
    struct LetStmtNode;
    struct VariableExprNode;
    struct BinaryExprNode;
    struct CallExprNode;
    struct LiteralExprNode;
    struct IfStmtNode;
    struct WhileStmtNode;
    struct ForStmtNode;
    struct FnStmtNode;
    struct ReturnStmtNode;
    struct ParamDeclNode;
}

namespace ir {
    class IRBuilder;
}

namespace ownership {

// Forward declarations
class OwnershipContext;
class OwnershipChecker;
class Lifetime;
class VariableConstraint;
class SafetyHook;

/**
 * @brief Ownership kinds - states a value can have
 */
enum class OwnershipKind : uint8_t {
    None = 0,       // No ownership (e.g., literals, destroyed)
    Owned,          // Owns the data (destructor will clean up)
    Borrowed,       // Borrowed immutably
    BorrowedMutable, // Borrowed mutably
    Moved,          // Ownership has been moved
    Static,         // Static lifetime (global, never deallocated)
    Weak,           // Weak reference (doesn't prevent drop)
    Destroyed,      // RAII: destroyed at end of scope
};

// ============================================================================
// Ada Layer: VariableConstraint - Range & Constraint Guard
// ============================================================================

/**
 * @brief Variable constraint for Ada Layer
 * Tracks range constraints (e.g., mana: 0..1000)
 */
class VariableConstraint {
public:
    VariableConstraint();
    VariableConstraint(int64_t min_val, int64_t max_val);
    
    bool has_range() const;
    int64_t get_min() const;
    int64_t get_max() const;
    
    bool is_in_range(int64_t value) const;
    bool can_statically_violate(int64_t value) const;
    
    std::string to_string() const;
    
private:
    bool has_range_;
    int64_t min_value_;
    int64_t max_value_;
};

// ============================================================================
// Python Layer: SafetyHook - Safe Runtime Fallback
// ============================================================================

/**
 * @brief Safety hook for Python Layer
 * Generates IR for runtime safety checks
 */
class SafetyHook {
public:
    enum class HookType : uint8_t {
        BoundsCheck,
        NullCheck,
        DivZeroCheck,
        RangeCheck,
        TypeCheck,
    };
    
    SafetyHook(HookType type, std::string message);
    
    HookType get_type() const;
    const std::string& get_message() const;
    
    static SafetyHook create_bounds_check(std::string var_name, int64_t index, int64_t bound);
    static SafetyHook create_null_check(std::string var_name);
    static SafetyHook create_divzero_check(std::string var_name);
    
private:
    HookType type_;
    std::string message_;
};

// ============================================================================
// Rust Layer: Lifetime - Zero-Cost Compile-Time Only
// ============================================================================

/**
 * @brief Lifetime - compile-time only, zero runtime cost
 */
class Lifetime {
public:
    Lifetime() : start_(0), end_(0), is_static_(false) {}
    Lifetime(uint64_t start, uint64_t end) : start_(start), end_(end), is_static_(false) {}
    
    uint64_t get_start() const { return start_; }
    uint64_t get_end() const { return end_; }
    bool is_static() const { return is_static_; }
    bool contains(uint64_t point) const { 
        return is_static_ || (point >= start_ && point <= end_); 
    }
    
    void set_start(uint64_t s) { start_ = s; }
    void set_end(uint64_t e) { end_ = e; }
    void set_static() { is_static_ = true; }
    
private:
    uint64_t start_;
    uint64_t end_;
    bool is_static_;
};

// ============================================================================
// OwnershipInfo - Variable Ownership State
// ============================================================================

/**
 * @brief Variable ownership information
 */
class OwnershipInfo {
public:
    OwnershipInfo() : kind_(OwnershipKind::Owned), type_name_() {}
    explicit OwnershipInfo(OwnershipKind kind) : kind_(kind), type_name_() {}
    OwnershipInfo(OwnershipKind kind, std::string type_name) 
        : kind_(kind), type_name_(std::move(type_name)) {}
    
    OwnershipKind get_kind() const { return kind_; }
    void set_kind(OwnershipKind kind) { kind_ = kind; }
    
    const std::string& get_type_name() const { return type_name_; }
    void set_type_name(const std::string& name) { type_name_ = name; }
    
    bool can_borrow() const { 
        return kind_ == OwnershipKind::Owned || kind_ == OwnershipKind::Borrowed; 
    }
    bool can_move() const { return kind_ == OwnershipKind::Owned; }
    bool is_borrowed() const { 
        return kind_ == OwnershipKind::Borrowed || kind_ == OwnershipKind::BorrowedMutable; 
    }
    bool is_moved() const { return kind_ == OwnershipKind::Moved; }
    bool is_destroyed() const { return kind_ == OwnershipKind::Destroyed || kind_ == OwnershipKind::None; }
    
private:
    OwnershipKind kind_;
    std::string type_name_;
};

// ============================================================================
// OwnershipContext - C++ Layer: Scope-based Cleanup
// ============================================================================

/**
 * @brief Ownership context - manages ownership within a scope
 * COMPILE-TIME ONLY: Zero runtime cost
 */
class OwnershipContext {
public:
    OwnershipContext();
    ~OwnershipContext();
    
    OwnershipContext(const OwnershipContext&) = delete;
    OwnershipContext& operator=(const OwnershipContext&) = delete;
    OwnershipContext(OwnershipContext&&) noexcept;
    OwnershipContext& operator=(OwnershipContext&&) noexcept;
    
    // Variable declarations
    void declare_variable(std::string name, OwnershipInfo info);
    void declare_variable(std::string name, OwnershipInfo info, VariableConstraint constraint);
    
    // Ownership operations
    void move_variable(std::string name);
    void borrow_variable(std::string borrower, std::string lender, bool mutable_borrow);
    void end_borrow(std::string borrower);
    
    // Lookup
    OwnershipInfo* lookup(std::string_view name);
    const OwnershipInfo* lookup(std::string_view name) const;
    bool contains(std::string_view name) const;
    
    // C++ Layer: Scope management with RAII
    void enter_scope();
    void exit_scope();
    void add_to_scope(std::string name);
    
    // Point tracking (for lifetime analysis)
    uint64_t get_current_point() const { return current_point_; }
    void advance_point() { ++current_point_; }
    
    // Ada Layer: Constraint management
    void set_constraint(std::string name, VariableConstraint constraint);
    const VariableConstraint* get_constraint(std::string_view name) const;
    
    // Python Layer: Safety hook management
    void add_safety_hook(std::string context, SafetyHook hook);
    const std::vector<SafetyHook>* get_safety_hooks(std::string_view context) const;
    void clear_safety_hooks();
    
    // Verbose logging
    void set_verbose(bool verbose) { verbose_ = verbose; }
    
private:
    size_t scope_depth_;
    uint64_t current_point_;
    std::unordered_map<std::string, OwnershipInfo> variables_;
    std::stack<std::vector<std::string>> scope_variables_;
    
    // Ada Layer: Constraints
    std::unordered_map<std::string, VariableConstraint> constraints_;
    
    // Borrow tracking
    std::unordered_map<std::string, std::string> active_borrows_;
    
    // Python Layer: Safety hooks
    std::unordered_map<std::string, std::vector<SafetyHook>> safety_hooks_;
    
    // Verbose flag
    bool verbose_;
};

// ============================================================================
// OwnershipError - Error Reporting
// ============================================================================

/**
 * @brief Ownership error types
 */
enum class OwnershipErrorKind {
    UseAfterMove,
    BorrowAfterMove,
    MutableBorrowWhileImmutable,
    ImmutableBorrowWhileMutable,
    BorrowOfMovedValue,
    InvalidLifetime,
    DanglingPointer,
    DoubleFree,
    UndefinedVariable,
    RangeViolation,
};

/**
 * @brief Ownership error with location info
 */
struct OwnershipError {
    OwnershipErrorKind kind;
    std::string message;
    std::string variable_name;
    uint64_t point;
    uint32_t line;
    uint32_t column;
    
    std::string to_string() const;
};

// ============================================================================
// OwnershipChecker - Full AST Integration
// ============================================================================

/**
 * @brief Ownership checker - performs static ownership analysis
 * COMPILE-TIME ONLY: Zero runtime cost
 */
class OwnershipChecker {
public:
    // Maximum recursion depth to prevent stack overflow
    static constexpr size_t kMaxRecursionDepth = 10000;
    
    OwnershipChecker();
    ~OwnershipChecker();
    
    OwnershipChecker(const OwnershipChecker&) = delete;
    OwnershipChecker& operator=(const OwnershipChecker&) = delete;
    
    // Check a program (AST) - Full Integration with ast::Node*
    void check(ast::Node* ast_root);
    
    bool has_errors() const { return !errors_.empty(); }
    const std::vector<OwnershipError>& get_errors() const { return errors_; }
    void clear_errors() { errors_.clear(); }
    
    OwnershipContext& get_context() { return context_; }
    const OwnershipContext& get_context() const { return context_; }
    
    void set_verbose(bool verbose) { 
        verbose_ = verbose; 
        context_.set_verbose(verbose);
    }
    
    // Python Layer: Generate IR safety hooks
    void generate_safety_hooks(ir::IRBuilder* builder);
    
private:
    OwnershipContext context_;
    std::vector<OwnershipError> errors_;
    bool verbose_;
    
    // Depth tracking to prevent stack overflow
    size_t current_depth_;
    
    // Check depth and return false if max exceeded
    bool check_depth();
    
    void report_error(OwnershipErrorKind kind, std::string_view var_name, 
                     std::string_view message, uint32_t line = 0, uint32_t column = 0);
    
    // Full AST walking - compile time only
    void walk_node(ast::Node* node);
    void walk_statement(ast::Stmt* stmt);
    void walk_expression(ast::Expr* expr);
    
    // Statement handlers - C++ Layer
    void handle_let(ast::LetStmtNode* stmt);
    void handle_block(ast::BlockStmtNode* stmt);
    void handle_if(ast::IfStmtNode* stmt);
    void handle_while(ast::WhileStmtNode* stmt);
    void handle_for(ast::ForStmtNode* stmt);
    void handle_fn(ast::FnStmtNode* stmt);
    void handle_return(ast::ReturnStmtNode* stmt);
    
    // Expression handlers - Full Use-After-Move Detection
    void handle_variable(ast::VariableExprNode* expr);
    void handle_binary(ast::BinaryExprNode* expr);
    void handle_call(ast::CallExprNode* expr);
    void handle_literal(ast::LiteralExprNode* expr);
    
    // Ada Layer: Helper for constant evaluation
    int64_t evaluate_constant_int(ast::Expr* expr);
};

// ============================================================================
// Simple ownership analysis result
// ============================================================================

/**
 * @brief Simple ownership analysis result
 */
struct OwnershipAnalysisResult {
    bool success;
    std::vector<OwnershipError> errors;
    
    static OwnershipAnalysisResult ok() { return {true, {}}; }
    static OwnershipAnalysisResult error(OwnershipError err) { return {false, {std::move(err)}}; }
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Helper to get ownership kind string
 */
inline const char* ownership_kind_to_string(OwnershipKind kind) {
    switch (kind) {
        case OwnershipKind::None: return "none";
        case OwnershipKind::Owned: return "owned";
        case OwnershipKind::Borrowed: return "borrowed";
        case OwnershipKind::BorrowedMutable: return "borrowed_mut";
        case OwnershipKind::Moved: return "moved";
        case OwnershipKind::Static: return "static";
        case OwnershipKind::Weak: return "weak";
        case OwnershipKind::Destroyed: return "destroyed";
        default: return "unknown";
    }
}

/**
 * @brief Helper to get error kind string
 */
inline const char* error_kind_to_string(OwnershipErrorKind kind) {
    switch (kind) {
        case OwnershipErrorKind::UseAfterMove: return "use after move";
        case OwnershipErrorKind::BorrowAfterMove: return "borrow after move";
        case OwnershipErrorKind::MutableBorrowWhileImmutable: return "mutable borrow while immutable borrow";
        case OwnershipErrorKind::ImmutableBorrowWhileMutable: return "immutable borrow while mutable borrow";
        case OwnershipErrorKind::BorrowOfMovedValue: return "borrow of moved value";
        case OwnershipErrorKind::InvalidLifetime: return "invalid lifetime";
        case OwnershipErrorKind::DanglingPointer: return "dangling pointer";
        case OwnershipErrorKind::DoubleFree: return "double free";
        case OwnershipErrorKind::UndefinedVariable: return "undefined variable";
        case OwnershipErrorKind::RangeViolation: return "range violation";
        default: return "unknown error";
    }
}

} // namespace ownership
} // namespace veldocra

