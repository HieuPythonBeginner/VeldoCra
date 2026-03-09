/**
 * @file parser.h
 * @brief VeldoCra Parser - Header
 * @author Dr. Bright
 * 
 * Recursive descent parser that converts tokens to AST using arena allocator.
 * Uses category-based dispatch for extensibility.
 */

#pragma once

#include "../lexer/token.h"
#include "../lexer/keyword_table.h"
#include "../ast/ast.h"
#include <string_view>
#include <string>
#include <vector>
#include <memory>

namespace veldocra {
namespace parser {

/**
 * @brief Parser result containing AST and status
 */
struct ParseResult {
    veldocra::ast::Node* root;
    bool success;
    std::string error_message;
    size_t error_line;
    size_t error_column;
    
    static ParseResult ok(veldocra::ast::Node* node) {
        return {node, true, "", 0, 0};
    }
    
    static ParseResult fail(std::string msg, size_t line = 0, size_t col = 0) {
        return {nullptr, false, std::move(msg), line, col};
    }
};

/**
 * @brief Recursive descent parser for VeldoCra
 * 
 * Converts token stream to AST using arena allocator.
 * Grammar follows Python-like indentation-sensitive syntax.
 */
class Parser {
public:
    /**
     * @brief Construct parser with token stream
     * @param tokens Token stream to parse
     * @param source Source code for error reporting
     */
    Parser(const veldocra::lexer::TokenStream& tokens, std::string_view source);
    ~Parser();
    
    // Disable copying
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    
    // Allow moving
    Parser(Parser&&) noexcept;
    Parser& operator=(Parser&&) noexcept;
    
    /**
     * @brief Parse the token stream
     * @return ParseResult with AST root or error
     */
    ParseResult parse();
    
    /**
     * @brief Get AST builder for external use
     */
    veldocra::ast::ASTBuilder& get_builder() { return builder_; }
    
    /**
     * @brief Check if parsing had errors
     */
    bool has_errors() const { return error_count_ > 0; }
    
    /**
     * @brief Get error count
     */
    size_t get_error_count() const { return error_count_; }

private:
    const veldocra::lexer::TokenStream& tokens_;
    std::string_view source_;
    veldocra::ast::ASTBuilder builder_;
    
    size_t current_;           // Current token index
    size_t error_count_;       // Number of parse errors
    bool at_line_start_;       // At start of line (for indentation)
    
    // Current token access
    const veldocra::lexer::Token& current_token() const;
    const veldocra::lexer::Token& peek_token(size_t offset) const;
    bool is_at_end() const;
    
    // Token navigation
    void advance();
    bool check(veldocra::lexer::TokenType type) const;
    bool check_next(veldocra::lexer::TokenType type) const;
    bool match(veldocra::lexer::TokenType type);
    bool match_any(const std::vector<veldocra::lexer::TokenType>& types);
    
    // Error handling
    void report_error(std::string_view message);
    void report_error_at(std::string_view message, size_t line, size_t column);
    
    // Synchronization points
    void synchronize();
    
    // ===== Grammar Rules =====
    
    // Program - list of statements
    veldocra::ast::Node* parse_program();
    
    // Statement parsing
    veldocra::ast::Stmt* parse_statement();
    veldocra::ast::Stmt* parse_simple_statement();
    veldocra::ast::Stmt* parse_compound_statement();
    
    // Expression statement
    veldocra::ast::Stmt* parse_expression_statement();
    
    // Block statement (indented group)
    veldocra::ast::BlockStmtNode* parse_block();
    veldocra::ast::BlockStmtNode* parse_block_body();
    
    // Hybrid block parsing (supports both {} and : with ;)
    veldocra::ast::BlockStmtNode* parse_hybrid_block();
    veldocra::ast::BlockStmtNode* parse_hybrid_block_body();
    veldocra::ast::BlockStmtNode* parse_hybrid_block_body_colon();
    
    // Variable declarations
    veldocra::ast::LetStmtNode* parse_manifest_statement();
    veldocra::ast::LetStmtNode* parse_const_statement();
    
    // Function definitions
    veldocra::ast::FnStmtNode* parse_function_definition();
    veldocra::ast::ParamDeclNode* parse_parameter();
    
    // Control flow
    veldocra::ast::IfStmtNode* parse_if_statement();
    veldocra::ast::WhileStmtNode* parse_while_statement();
    veldocra::ast::ForStmtNode* parse_for_statement();
    veldocra::ast::ReturnStmtNode* parse_return_statement();
    
    // Phase 7: OOP definitions (race, avatar, grant, blessing)
    veldocra::ast::Stmt* parse_oop_definition();
    
    // Phase 7: origin keyword (standalone or scoped)
    veldocra::ast::Stmt* parse_origin_statement();
    
    // Expression parsing (precedence climbing)
    veldocra::ast::Expr* parse_expression();
    veldocra::ast::Expr* parse_assignment();
    veldocra::ast::Expr* parse_logical_or();
    veldocra::ast::Expr* parse_logical_and();
    veldocra::ast::Expr* parse_equality();
    veldocra::ast::Expr* parse_comparison();
    veldocra::ast::Expr* parse_term();
    veldocra::ast::Expr* parse_factor();
    veldocra::ast::Expr* parse_unary();
    veldocra::ast::Expr* parse_postfix();
    veldocra::ast::Expr* parse_primary();
    
    // Literals
    veldocra::ast::LiteralExprNode* parse_literal();
    
    // Function calls
    veldocra::ast::CallExprNode* parse_call(veldocra::ast::Expr* callee);
    
    // Helper methods
    const char* get_token_text(const veldocra::lexer::Token& token) const;
    std::string_view get_token_view(const veldocra::lexer::Token& token) const;
};

} // namespace parser
} // namespace veldocra

