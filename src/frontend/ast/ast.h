/**
 * @file ast.h
 * @brief VeldoCra AST Definitions with Arena Allocator
 * @author Dr. Bright
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <vector>
#include <array>
#include <memory>

namespace veldocra {
namespace ast {

/**
 * @brief Arena allocator for AST nodes
 * 
 * Uses bump-pointer allocation for O(1) node creation.
 * Memory is allocated in large chunks (1MB+) to minimize overhead.
 */
class ArenaAllocator {
public:
    /**
     * @brief Construct arena with specified chunk size
     * @param chunk_size Size of each chunk in bytes (default 1MB)
     */
    explicit ArenaAllocator(size_t chunk_size = 1024 * 1024);
    ~ArenaAllocator();
    
    // Disable copying, enable moving
    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;
    ArenaAllocator(ArenaAllocator&&) noexcept;
    ArenaAllocator& operator=(ArenaAllocator&&) noexcept;
    
    /**
     * @brief Allocate memory from arena
     * @param size Size in bytes
     * @param alignment Alignment requirement
     * @return Pointer to allocated memory
     */
    void* allocate(size_t size, size_t alignment = 8) const;
    
    /**
     * @brief Allocate T with constructor args
     * @tparam T Type to allocate
     * @tparam Args Argument types
     * @param args Arguments to forward to constructor
     * @return Pointer to allocated object
     */
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        void* ptr = allocate(sizeof(T), alignof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }
    
    /**
     * @brief Reset arena (deallocate all chunks)
     */
    void reset();
    
    /**
     * @brief Get total allocated memory
     */
    size_t total_allocated() const { return total_allocated_; }
    
    /**
     * @brief Get total used memory
     */
    size_t total_used() const { return total_used_; }

private:
    struct Chunk {
        char* data;
        size_t size;
        size_t used;
        Chunk* next;
    };
    
    size_t chunk_size_;
    mutable Chunk* current_chunk_;
    mutable size_t total_allocated_;
    mutable size_t total_used_;
    
    /**
     * @brief Allocate new chunk
     */
    void allocate_chunk() const;
};

/**
 * @brief AST node types
 */
enum class NodeKind : uint16_t {
    // Expressions
    Expr,
    LiteralExpr,
    VariableExpr,
    MemberExpr,
    BinaryExpr,
    UnaryExpr,
    CallExpr,
    IndexExpr,
    ArrayExpr,
    DictExpr,
    LambdaExpr,
    
    // Statements
    Stmt,
    ExprStmt,
    BlockStmt,
    IfStmt,
    WhileStmt,
    ForStmt,
    ReturnStmt,
    BreakStmt,
    ContinueStmt,
    LetStmt,
    ConstStmt,
    FnStmt,
    ClassStmt,
    StructStmt,
    EnumStmt,
    ImportStmt,
    MatchStmt,
    
    // Declarations
    Decl,
    ParamDecl,
    FieldDecl,
    VariantDecl,
};

/**
 * @brief AST node base class
 */
struct Node {
    NodeKind kind;
    uint32_t line;
    uint32_t column;
    
    // Type information (filled by type checker)
    void* type_info;
    
    // Source location
    uint32_t start_offset;
    uint32_t end_offset;
};

/**
 * @brief Expression base
 */
struct Expr : Node {
    // Value category (lvalue, rvalue)
    uint8_t value_category;  // 0 = rvalue, 1 = lvalue
    uint8_t padding[7];
};

/**
 * @brief Statement base
 */
struct Stmt : Node {
    // Statements don't need additional fields for now
};

// Forward declarations
struct LiteralExprNode;
struct VariableExprNode;
struct MemberExprNode;
struct BinaryExprNode;
struct UnaryExprNode;
struct CallExprNode;
struct BlockStmtNode;
struct IfStmtNode;
struct WhileStmtNode;
struct ForStmtNode;
struct ReturnStmtNode;
struct LetStmtNode;
struct FnStmtNode;
struct ParamDeclNode;

/**
 * @brief Literal expression (numbers, strings, bool, None)
 */
struct LiteralExprNode : Expr {
    // Literal value union
    union Value {
        int64_t int_val;
        double float_val;
        bool bool_val;
        char char_val;
        const char* string_val;  // Stored in arena
        void* none_val;
        
        Value() : none_val(nullptr) {}
    } value;
    
    // Literal type: 0=int, 1=float, 2=string, 3=bool, 4=char, 5=none
    uint8_t literal_type;
    uint8_t padding[7];
};

/**
 * @brief Variable reference expression
 */
struct VariableExprNode : Expr {
    const char* name;  // Stored in arena
};

/**
 * @brief Member expression (ego.field, core.field)
 */
struct MemberExprNode : Expr {
    Expr* object;      // The object being accessed (ego, core, or other)
    const char* member;  // Member name (stored in arena)
    bool is_ego;       // true if using ego (this), false if using core (self)
};

/**
 * @brief Binary expression (a + b, a * b, etc.)
 */
struct BinaryExprNode : Expr {
    Expr* left;
    Expr* right;
    uint16_t op;  // Operator enum
    uint16_t padding;
};

/**
 * @brief Unary expression (-a, !a, etc.)
 */
struct UnaryExprNode : Expr {
    Expr* operand;
    uint16_t op;
    uint16_t padding;
};

/**
 * @brief Function call expression
 */
struct CallExprNode : Expr {
    Expr* callee;
    Expr** args;
    size_t arg_count;
    size_t arg_capacity;
};

/**
 * @brief Block statement
 */
struct BlockStmtNode : Stmt {
    Node** statements;
    size_t statement_count;
    size_t statement_capacity;
};

/**
 * @brief If statement
 */
struct IfStmtNode : Stmt {
    Expr* condition;
    Stmt* then_branch;
    Stmt* else_branch;  // Can be nullptr or another ifStmt
};

/**
 * @brief While statement
 */
struct WhileStmtNode : Stmt {
    Expr* condition;
    Stmt* body;
};

/**
 * @brief For statement
 */
struct ForStmtNode : Stmt {
    const char* iterator_name;
    Expr* iterable;
    Stmt* body;
};

/**
 * @brief Return statement
 */
struct ReturnStmtNode : Stmt {
    Expr* value;
};

/**
 * @brief Let statement (variable declaration)
 */
struct LetStmtNode : Stmt {
    const char* name;
    Expr* type_expr;  // Optional type annotation
    Expr* initializer;
    bool is_mutable;
    bool padding[7];
};

/**
 * @brief Function statement
 */
struct FnStmtNode : Stmt {
    const char* name;
    ParamDeclNode** params;
    size_t param_count;
    Stmt* body;
    Expr* return_type;
};

/**
 * @brief Parameter declaration
 */
struct ParamDeclNode : Node {
    const char* name;
    Expr* type_expr;
    Expr* default_value;
};

/**
 * @brief AST builder with arena allocator
 */
class ASTBuilder {
public:
    explicit ASTBuilder();
    ~ASTBuilder();
    
    // Disable copying
    ASTBuilder(const ASTBuilder&) = delete;
    ASTBuilder& operator=(const ASTBuilder&) = delete;
    
    // Allow moving
    ASTBuilder(ASTBuilder&&) noexcept;
    ASTBuilder& operator=(ASTBuilder&&) noexcept;
    
    // Generic node creation using arena
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        return arena_.create<T>(std::forward<Args>(args)...);
    }
    
    // Expression creation
    LiteralExprNode* create_literal(int64_t value);
    LiteralExprNode* create_literal(double value);
    LiteralExprNode* create_literal(bool value);
    LiteralExprNode* create_literal(const char* value);
    LiteralExprNode* create_literal(char value);
    LiteralExprNode* create_none();
    
    VariableExprNode* create_variable(const char* name);
    
    // Member expression (ego.field, core.field)
    MemberExprNode* create_member(Expr* object, const char* member, bool is_ego);
    
    BinaryExprNode* create_binary(Expr* left, uint16_t op, Expr* right);
    UnaryExprNode* create_unary(uint16_t op, Expr* operand);
    
    CallExprNode* create_call(Expr* callee);
    void add_argument(CallExprNode* call, Expr* arg);
    
    // Statement creation
    BlockStmtNode* create_block();
    void add_statement(BlockStmtNode* block, Stmt* stmt);
    
    IfStmtNode* create_if(Expr* condition, Stmt* then_branch, Stmt* else_branch = nullptr);
    WhileStmtNode* create_while(Expr* condition, Stmt* body);
    ForStmtNode* create_for(const char* iterator, Expr* iterable, Stmt* body);
    ReturnStmtNode* create_return(Expr* value = nullptr);
    
    LetStmtNode* create_let(const char* name, Expr* init, bool mutable_flag = false);
    
    FnStmtNode* create_fn(const char* name);
    void add_param(FnStmtNode* fn, ParamDeclNode* param);
    void set_fn_body(FnStmtNode* fn, Stmt* body);
    
    // Utility
    ArenaAllocator& get_arena() { return arena_; }
    
    /**
     * @brief Get error count
     */
    size_t get_error_count() const { return error_count_; }
    
    /**
     * @brief Check if has errors
     */
    bool has_errors() const { return error_count_ > 0; }
    
    /**
     * @brief Intern a string in the arena (allocate null-terminated copy)
     * @param str String view to copy
     * @return Null-terminated c-string stored in arena
     */
    const char* intern_string(std::string_view str) const;

private:
    mutable ArenaAllocator arena_;
    size_t error_count_;
    
    /**
     * @brief Allocate string in arena
     */
    const char* alloc_string(std::string_view str) const;
    
    /**
     * @brief Report error
     */
    void report_error(std::string_view message);
};

/**
 * @brief AST printer for debugging
 */
class ASTPrinter {
public:
    explicit ASTPrinter(std::string_view source);
    ~ASTPrinter();
    
    /**
     * @brief Print AST to stdout
     */
    void print(Node* node);
    
    /**
     * @brief Print AST to string
     */
    std::string to_string(Node* node);

private:
    std::string_view source_;
    int indent_level_;
    
    void print_indent();
    void print_node(Node* node);
    void print_expr(Expr* expr);
    void print_stmt(Stmt* stmt);
};

} // namespace ast
} // namespace veldocra

