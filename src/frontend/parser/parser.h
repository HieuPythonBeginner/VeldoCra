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

namespace veldanava {
namespace parser {

// Block delimiter styles for verdict/fail consistency checking
enum class BlockDelimiterStyle {
    CURLY_BRACES,      // {}
    COLON_SEMICOLON,   // : with ;
    PARENTHESIS,       // ()
    SINGLE_STATEMENT   // no delimiters
};

/**
 * @brief Parse result containing AST and status
 */
struct ParseResult {
    veldanava::ast::Node* root;
    bool success;
    std::string error_message;
    size_t error_line;
    size_t error_column;

    static ParseResult ok(veldanava::ast::Node* node) {
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
 * ENHANCED: Strict delimiter consistency for verdict/fail blocks
 */
class Parser {
public:
    /**
     * @brief Construct parser with token stream
     * @param tokens Token stream to parse
     * @param source Source code for error reporting
     */
    Parser(const veldanava::lexer::TokenStream& tokens, std::string_view source);
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
    veldanava::ast::ASTBuilder& get_builder() { return builder_; }
    
    /**
     * @brief Check if parsing had errors
     */
    bool has_errors() const { return error_count_ > 0; }
    
    /**
     * @brief Get error count
     */
    size_t get_error_count() const { return error_count_; }

private:
    const veldanava::lexer::TokenStream& tokens_;
    std::string_view source_;
    veldanava::ast::ASTBuilder builder_;
    
    size_t current_;           // Current token index
    size_t error_count_;       // Number of parse errors
    bool at_line_start_;       // At start of line (for indentation)
    
    // Current token access
    const veldanava::lexer::Token& current_token() const;
    const veldanava::lexer::Token& peek_token(size_t offset) const;
    bool is_at_end() const;
    
    // Token navigation
    void advance();
    bool check(veldanava::lexer::TokenType type) const;
    bool check_next(veldanava::lexer::TokenType type) const;
    bool match(veldanava::lexer::TokenType type);
    bool match_any(const std::vector<veldanava::lexer::TokenType>& types);
    
    // Error handling
    void report_error(std::string_view message);
    void report_error_at(std::string_view message, size_t line, size_t column);
    
    // Synchronization points
    void synchronize();
    
    // ===== Grammar Rules =====
    
    // Program - list of statements
    veldanava::ast::Node* parse_program();
    
    // Statement parsing
    veldanava::ast::Stmt* parse_statement();
    veldanava::ast::Stmt* parse_simple_statement();
    veldanava::ast::Stmt* parse_compound_statement();
    
    // Expression statement
    veldanava::ast::Stmt* parse_expression_statement();
    
    // Block statement (braces {} group)
    veldanava::ast::BlockStmtNode* parse_block();
    
    // Variable declarations
    veldanava::ast::LetStmtNode* parse_const_statement();
    
    // Function definitions
    veldanava::ast::BlockStmtNode* parse_block_body(veldanava::lexer::TokenType end_token = veldanava::lexer::TokenType::EndOfFile);
    veldanava::ast::LetStmtNode* parse_let_statement();
    veldanava::ast::FnStmtNode* parse_function_definition();
    veldanava::ast::ParamDeclNode* parse_parameter();
    
    // Control flow
    veldanava::ast::IfStmtNode* parse_if_statement();
    veldanava::ast::WhileStmtNode* parse_while_statement();
    veldanava::ast::ForStmtNode* parse_for_statement();
    veldanava::ast::ReturnStmtNode* parse_return_statement();
    veldanava::ast::MatchStmtNode* parse_match_statement();
    veldanava::ast::PatternNode* parse_pattern();

    // Error handling statements
    veldanava::ast::Stmt* parse_try_statement();
    veldanava::ast::Stmt* parse_raise_statement();

    // Module system statements
    veldanava::ast::Stmt* parse_import_statement();
    veldanava::ast::Stmt* parse_module_statement();
    veldanava::ast::Stmt* parse_package_statement();

    // Phase 7: OOP definitions (race, avatar, grant, blessing)
    veldanava::ast::Stmt* parse_oop_definition();
    veldanava::ast::Stmt* parse_race_definition();
    veldanava::ast::Stmt* parse_avatar_definition();
    
    // Phase 7: origin keyword (standalone or scoped)
    veldanava::ast::Stmt* parse_origin_statement();
 
    // Phase 7: memory management keywords
    veldanava::ast::Stmt* parse_memory_statement();
    
    // Expression parsing (precedence climbing)
    veldanava::ast::Expr* parse_expression();
    veldanava::ast::Expr* parse_assignment();
    veldanava::ast::Expr* parse_type_expr();
    veldanava::ast::Expr* parse_logical_or();
    veldanava::ast::Expr* parse_logical_and();
    veldanava::ast::Expr* parse_equality();
    veldanava::ast::Expr* parse_comparison();
    veldanava::ast::Expr* parse_term();
    veldanava::ast::Expr* parse_factor();
    veldanava::ast::Expr* parse_unary();
    veldanava::ast::Expr* parse_postfix();
    veldanava::ast::Expr* parse_primary();
    
    // Literals
    veldanava::ast::LiteralExprNode* parse_literal();
    
    // Function calls
    veldanava::ast::CallExprNode* parse_call(veldanava::ast::Expr* callee);
    
    // Helper methods
    const char* get_token_text(const veldanava::lexer::Token& token) const;
    std::string_view get_token_view(const veldanava::lexer::Token& token) const;
};

} // namespace parser
} // namespace veldanava
