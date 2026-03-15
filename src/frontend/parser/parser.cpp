/**
 * @file parser.cpp
 * @brief VeldoCra Parser - Implementation
 * @author Dr. Bright
 * 
 * Recursive descent parser that converts tokens to AST using arena allocator.
 * Uses category-based dispatch for extensibility.
 */

#include "parser.h"
#include "frontend/lexer/keyword_table.h"
#include "frontend/lexer/system_command.h"
#include <cstring>
#include <iostream>
#include <sstream>

namespace veldocra {
namespace parser {

using namespace veldocra::lexer;
using namespace veldocra::ast;

// ============================================================================
// Parser Implementation
// ============================================================================

/**
 * @brief Construct parser with token stream
 */
Parser::Parser(const TokenStream& tokens, std::string_view source)
    : tokens_(tokens)
    , source_(source)
    , current_(0)
    , error_count_(0)
    , at_line_start_(true) {
}

/**
 * @brief Destructor
 */
Parser::~Parser() = default;

/**
 * @brief Move constructor
 */
Parser::Parser(Parser&& other) noexcept
    : tokens_(other.tokens_)
    , source_(other.source_)
    , builder_(std::move(other.builder_))
    , current_(other.current_)
    , error_count_(other.error_count_)
    , at_line_start_(other.at_line_start_) {
}

/**
 * @brief Move assignment
 */
Parser& Parser::operator=(Parser&& other) noexcept {
    if (this != &other) {
        source_ = other.source_;
        current_ = other.current_;
        error_count_ = other.error_count_;
        at_line_start_ = other.at_line_start_;
    }
    return *this;
}

// ===== Token Access =====

const Token& Parser::current_token() const {
    if (current_ < tokens_.size()) {
        return tokens_[current_];
    }
    // Return EOF token if past end
    static const Token eof_token{TokenType::EndOfFile, 0, 0, 0, 0, 0};
    return eof_token;
}

const Token& Parser::peek_token(size_t offset) const {
    size_t idx = current_ + offset;
    if (idx < tokens_.size()) {
        return tokens_[idx];
    }
    static const Token eof_token{TokenType::EndOfFile, 0, 0, 0, 0, 0};
    return eof_token;
}

bool Parser::is_at_end() const {
    return current_token().type == TokenType::EndOfFile;
}

void Parser::advance() {
    current_++;
    // HYBRID FIX: Always try to skip newlines at line start
    // This ensures we properly handle statement terminators (both ; and newline)
    // NOTE: Do NOT skip semicolons here - they need to be explicitly checked by parsers
    while (current_token().type == TokenType::Newline) {
        current_++;
    }
    // Reset at_line_start after skipping newlines
    at_line_start_ = true;
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return false;
    return current_token().type == type;
}

bool Parser::check_next(TokenType type) const {
    return peek_token(1).type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match_any(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

// ===== Error Handling =====

void Parser::report_error(std::string_view message) {
    error_count_++;
    std::cerr << "Parse Error (line " << current_token().line 
              << ", col " << current_token().column << "): " 
              << message << "\n";
}

void Parser::report_error_at(std::string_view message, size_t line, size_t column) {
    error_count_++;
    std::cerr << "Parse Error (line " << line << ", col " << column 
              << "): " << message << "\n";
}

void Parser::synchronize() {
    // Skip tokens until we find a statement boundary
    // Uses category-based dispatch - "blind" to specific keyword names
    while (!is_at_end()) {
        if (check(TokenType::Newline) || 
            check(TokenType::EndOfFile) ||
            KeywordCategoryLookup::is_function(current_token().type) ||
            KeywordCategoryLookup::is_declaration(current_token().type) ||
            KeywordCategoryLookup::is_flow_control(current_token().type)) {
            return;
        }
        advance();
    }
}

// ===== Helper Methods =====

const char* Parser::get_token_text(const Token& token) const {
    return builder_.intern_string(source_.substr(token.offset, token.length));
}

std::string_view Parser::get_token_view(const Token& token) const {
    return source_.substr(token.offset, token.length);
}
// ============================================================================

/**
 * @brief Parse the entire program
 */
Node* Parser::parse_program() {
    auto* block = builder_.create_block();
    
    // Safety counter to prevent infinite loops
    size_t max_iterations = tokens_.size() * 2;  // Allow 2x tokens as max iterations
    size_t iteration_count = 0;
    
    // Skip initial newlines/indents before reporting first token type
    while (check(TokenType::Newline) || check(TokenType::Indent) || 
           check(TokenType::Dedent) || check(TokenType::Semicolon)) {
        advance();
    }
    
    std::cerr << "[PARSER] parse_program: initial token type = " 
              << static_cast<int>(current_token().type) << std::endl;
    
    while (!is_at_end()) {
        // Safety check to prevent infinite loops
        if (++iteration_count > max_iterations) {
            std::cerr << "[PARSER] Maximum iteration count reached, stopping parse to prevent infinite loop\n";
            break;
        }
        
        // Skip newlines
        while (check(TokenType::Newline)) {
            advance();
        }
        
        if (is_at_end()) break;
        
        Stmt* stmt = parse_statement();
        if (stmt) {
            builder_.add_statement(block, stmt);
        } else {
            // CRITICAL: If parse_statement returns nullptr (error case),
            // we MUST advance to prevent infinite loop
            std::cerr << "[PARSER] Statement parse returned nullptr, advancing to prevent infinite loop\n";
            
            // Show the problematic token
            std::string_view token_text = get_token_view(current_token());
            if (!token_text.empty()) {
                std::cerr << "[PARSER] Problematic token: '" << std::string(token_text) 
                          << "' at Line " << current_token().line 
                          << ", Column " << current_token().column << "\n";
            }
            
            // Force advance past the problematic token
            advance();
        }
        
        // Skip newlines between statements
        while (check(TokenType::Newline)) {
            advance();
        }
    }
    
    std::cerr << "[PARSER] Returning block: " << (void*)block << " kind=" << (int)block->kind << std::endl; return reinterpret_cast<Node*>(block);
}

/**
 * @brief Parse a statement (simple or compound)
 * Uses category-based dispatch - "blind" to specific keyword names
 */
Stmt* Parser::parse_statement() {
    // Skip newlines at statement start
    while (check(TokenType::Newline)) {
        advance();
    }
    
    // Also skip INDENT and DEDENT tokens at statement start (for top-level code)
    while (check(TokenType::Indent) || check(TokenType::Dedent)) {
        advance();
    }
    
    // Skip semicolons at statement start (leftover from previous statement)
    while (check(TokenType::Semicolon)) {
        advance();
    }
    
    if (is_at_end()) return nullptr;
    
    at_line_start_ = false;
    
    std::cerr << "[PARSER] parse_statement: token type = " << static_cast<int>(current_token().type) << std::endl;
    
    // Get the category of current token for dispatch
    KeywordCategory category = KeywordCategoryLookup::get_category(current_token().type);
    
    // Compound statements (require indented block)
    // Uses category-based dispatch - no hardcoded keyword checks
    
    // Check for specific keywords directly (Phase 7: verdict, fail, cycle, sustain)
    // CRITICAL: This must come BEFORE the category-based dispatch
    // Priority 1: Enhanced verdict/fail handling with better block parsing
    if (check(TokenType::Kw_Verdict)) {
        std::cerr << "[PARSER] Found Kw_Verdict, calling parse_if_statement()" << std::endl;
        return parse_if_statement();
    }
    
    // Also handle fail explicitly (for if-else chains where fail wasn't consumed)
    if (check(TokenType::Kw_Fail)) {
        std::cerr << "[PARSER] Found standalone Kw_Fail - treating as block end marker" << std::endl;
        return nullptr; // Signal block end for parent
    }
    
    // NEW: Handle traditional control keywords as Identifiers with lookahead
    if (check(TokenType::Identifier) || check(TokenType::Kw_Feat) || check(TokenType::Kw_Fn) || check(TokenType::Kw_Race)) {
        std::string_view kw = get_token_view(current_token());
        std::cerr << "[PARSER] Identifier/Kw check: '" << std::string(kw) << "' type=" << static_cast<int>(current_token().type) << std::endl;
        if (kw == "for") {
            std::cerr << "[PARSER] Found 'for' Identifier - dispatching to parse_for_statement" << std::endl;
            advance();
            return parse_for_statement(); // Consume identifier
        } else if (kw == "if") {
            std::cerr << "[PARSER] Found 'if' Identifier - dispatching to parse_if_statement" << std::endl;
            advance();
            return parse_if_statement();
        } else if (kw == "while") {
            std::cerr << "[PARSER] Found 'while' Identifier - dispatching to parse_while_statement" << std::endl;
            advance();
            return parse_while_statement();
        } else if (kw == "fn" || current_token().type == TokenType::Kw_Feat) {
            std::cerr << "[PARSER] Found 'fn'/Kw_Feat - dispatching to parse_function_definition" << std::endl;
            advance();
            return parse_function_definition();
        } else if (kw == "struct" || kw == "race" || current_token().type == TokenType::Kw_Race) {
            std::cerr << "[PARSER] Found 'struct'/race - dispatching to parse_oop_definition" << std::endl;
            advance();
            return parse_oop_definition();
        } else if (kw == "else") {
            std::cerr << "[PARSER] Found 'else' - treating as fail block end" << std::endl;
            return nullptr; // Signal parent if to handle else branch
        }
    }
    if (check(TokenType::Kw_Sustain)) {
        return parse_while_statement();
    }
    if (check(TokenType::Kw_Cycle)) {
        return parse_for_statement();
    }
    
    // Phase 7: Handle race/avatar/grant/blessing as OOP definitions
    if (check(TokenType::Kw_Race) || check(TokenType::Kw_Avatar) ||
        check(TokenType::Kw_Grant) || check(TokenType::Kw_Blessing)) {
        return parse_oop_definition();
    }
    
    // Phase 7: Handle origin keyword (standalone or scoped)
    if (check(TokenType::Kw_Origin)) {
        return parse_origin_statement();
    }
    
    // Use category-based dispatch as fallback
    switch (category) {
        case KeywordCategory::FlowControl:
            // Handle any remaining flow control keywords
            break;
            
        case KeywordCategory::Function:
            if (check(TokenType::Kw_Ability) ||
                check(TokenType::Kw_Fn) || check(TokenType::Kw_Feat)) {
                return parse_function_definition();
            }
            if (check(TokenType::Kw_Scale)) {
                return parse_return_statement();
            }
            break;
            
        case KeywordCategory::SystemCommand:
            // Handle system command keywords (flow, sink, rise, etc.)
            // These are treated as function-like expressions
            return parse_expression_statement();
            
        default:
            break;
    }
    
    // Simple statements
    return parse_simple_statement();
}

/**
 * @brief Parse simple statement (expression or declaration)
 * Uses category-based dispatch - "blind" to specific keyword names
 */
Stmt* Parser::parse_simple_statement() {
    std::cerr << "[PARSER] parse_simple_statement: current token type = " 
              << static_cast<int>(current_token().type) 
              << " text = '" << std::string(get_token_view(current_token())) << "'" << std::endl;
    
    // CRITICAL FIX: Direct check for 'manifest' keyword (token type Kw_Manifest)
    // Phase 7: 'let' is forbidden, use 'manifest' instead
    if (check(TokenType::Kw_Manifest)) {
        return parse_manifest_statement();
    }
    
    // Also check for other declaration keywords
    if (check(TokenType::Kw_Limit) || check(TokenType::Kw_Av) ||
        check(TokenType::Kw_Immo) || check(TokenType::Kw_Const) ||
        check(TokenType::Kw_Mut) || check(TokenType::Kw_Evolve)) {
        return parse_manifest_statement();
    }
    
    // Use category-based dispatch as fallback
    KeywordCategory category = KeywordCategoryLookup::get_category(current_token().type);
    
    // Manifest statement - uses Declaration category
    if (KeywordCategoryLookup::is_declaration(current_token().type)) {
        return parse_manifest_statement();
    }
    
    // Const statement - uses Declaration category (but handled separately)
    if (check(TokenType::Kw_Const) || check(TokenType::Kw_Immo)) {
        return parse_const_statement();
    }
    
    // Expression statement
    return parse_expression_statement();
}

/**
 * @brief Parse expression statement
 * HYBRID: Accepts optional semicolon OR newline as statement terminator
 */
Stmt* Parser::parse_expression_statement() {
    Expr* expr = parse_expression();
    
    // Handle assignment: expr = expr or expr := expr
    if (check(TokenType::Assign) || check(TokenType::ColonAssign)) {
        TokenType assign_type = current_token().type;
        advance();
        Expr* value = parse_expression();
        
        // Convert to let statement - check node kind
        if (expr->kind == NodeKind::VariableExpr) {
            auto* var = static_cast<VariableExprNode*>(expr);
            bool is_mutable = (assign_type == TokenType::Assign);
            return builder_.create_let(var->name, value, is_mutable);
        }
        
        // Handle member assignment: ego.name = value, core.value = value
        if (expr->kind == NodeKind::MemberExpr) {
            // For member assignments, we create a binary expression with assignment operator
            // The code generator will handle generating the appropriate store instruction
            auto* assign_expr = builder_.create_binary(expr, static_cast<uint16_t>(assign_type), value);
            return reinterpret_cast<Stmt*>(assign_expr);
        }
        
        report_error("Invalid assignment target");
    }
    
    // HYBRID FIX: Consume semicolon if present
    if (check(TokenType::Semicolon)) {
        advance();
    }
    // If no semicolon, let parse_program handle the NEWLINE
    
    // TODO: Handle other assignment types
    
    return reinterpret_cast<Stmt*>(expr);
}

/**
 * @brief Parse block (indented body)
 */
BlockStmtNode* Parser::parse_block() {
    // Expect INDENT token
    if (!check(TokenType::Indent)) {
        report_error("Expected indented block");
        return builder_.create_block();
    }
    advance();
    
    return parse_block_body();
}

/**
 * @brief Parse block body (statements until DEDENT)
 */
BlockStmtNode* Parser::parse_block_body() {
    auto* block = builder_.create_block();
    
    while (!check(TokenType::Dedent) && !is_at_end()) {
        // Skip newlines
        while (check(TokenType::Newline)) {
            advance();
        }
        
        if (check(TokenType::Dedent) || is_at_end()) break;
        
        Stmt* stmt = parse_statement();
        if (stmt) {
            builder_.add_statement(block, stmt);
        }
        
        // Skip newlines between statements
        while (check(TokenType::Newline)) {
            advance();
        }
    }
    
    // Consume DEDENT
    if (check(TokenType::Dedent)) {
        advance();
    }
    
    return block;
}

/**
 * @brief Parse hybrid block - supports both {} and : with ;
 * Returns block regardless of which syntax was used
 */
BlockStmtNode* Parser::parse_hybrid_block() {
    BlockStmtNode* block = nullptr;
    
    if (check(TokenType::BraceOpen)) {
        // {} style block
        advance();
        block = parse_hybrid_block_body();
        // Consume closing brace - may need to skip newlines/dedents first
        while (check(TokenType::Newline) || check(TokenType::Dedent) || check(TokenType::Indent)) {
            advance();
        }
        if (check(TokenType::BraceClose)) {
            std::cerr << "[PARSER] parse_hybrid_block: consuming BraceClose }" << std::endl;
            advance();
        }
    } else if (check(TokenType::Colon)) {
        // : style block - requires statements terminated with ;
        advance();
        block = parse_hybrid_block_body_colon();
    } else if (check(TokenType::Indent)) {
        // Indented block (Python-style)
        block = parse_block_body();
    } else {
        // Single statement
        block = builder_.create_block();
        Stmt* stmt = parse_simple_statement();
        if (stmt) {
            builder_.add_statement(block, stmt);
        }
    }
    
    return block;
}

/**
 * @brief Parse hybrid block body - handles both {} and : with ; styles
 */
BlockStmtNode* Parser::parse_hybrid_block_body() {
    auto* block = builder_.create_block();
    
    std::cerr << "[PARSER] parse_hybrid_block_body: starting, current token = " 
              << static_cast<int>(current_token().type) << std::endl;
    
    // FIXED: Skip INDENT only if at line start AND followed by statement, not always
    if (check(TokenType::Indent) && peek_token(1).type != TokenType::Dedent && peek_token(1).type != TokenType::BraceClose) {
        std::cerr << "[PARSER] parse_hybrid_block_body: skipping targeted INDENT" << std::endl;
        advance();
    }
    
    while (!is_at_end()) {
        // PRIORITY FIX 1: Check for BraceClose FIRST - most important!
        if (check(TokenType::BraceClose)) {
            std::cerr << "[PARSER] parse_hybrid_block_body: found BraceClose, breaking WITHOUT consuming" << std::endl;
            break;
        }
        
        // Skip ONLY Newline (not Dedent here - handle in parent)
        while (check(TokenType::Newline)) {
            advance();
        }
        
        // Double-check BraceClose after newlines
        if (check(TokenType::BraceClose)) {
            std::cerr << "[PARSER] parse_hybrid_block_body: BraceClose after newline, breaking" << std::endl;
            break;
        }
        
        // FIXED: Skip DEDENT only if inside proper indented block (not at brace level)
        // Detect brace context by checking if we saw BraceOpen earlier (simplified: skip only once per block)
        static thread_local bool skipped_dedent_once = false;
        if (check(TokenType::Dedent) && !skipped_dedent_once) {
            std::cerr << "[PARSER] parse_hybrid_block_body: skipping single DEDENT" << std::endl;
            advance();
            skipped_dedent_once = true;
            if (check(TokenType::BraceClose)) {
                std::cerr << "[PARSER] parse_hybrid_block_body: BraceClose after DEDENT, breaking" << std::endl;
                break;
            }
            continue;
        }
        
        if (is_at_end()) break;
        
        std::cerr << "[PARSER] parse_hybrid_block_body: calling parse_statement (token=" 
                  << static_cast<int>(current_token().type) << ")" << std::endl;
        
        Stmt* stmt = parse_statement();
        if (stmt) {
            builder_.add_statement(block, stmt);
        } else {
            // On parse failure, advance conservatively
            advance();
        }
        
        // Reset dedent skip for next statement
        skipped_dedent_once = false;
        
        // Skip newlines between statements
        while (check(TokenType::Newline)) {
            advance();
        }
    }
    
    std::cerr << "[PARSER] parse_hybrid_block_body: exiting, current token = " 
              << static_cast<int>(current_token().type) << std::endl;
    
    return block;
}

/**
 * @brief Parse colon-style block body (statements until ; alone or Dedent)
 * In colon style, ; alone on a line terminates the block
 * Also recognizes fail/else as block terminator for if-else chains
 */
BlockStmtNode* Parser::parse_hybrid_block_body_colon() {
    auto* block = builder_.create_block();
    
    while (!is_at_end()) {
        // Skip newlines
        while (check(TokenType::Newline)) {
            advance();
        }
        
        // Check for standalone semicolon (block terminator in colon style)
        if (check(TokenType::Semicolon)) {
            // Look ahead - is this a standalone ; or statement-ending ;?
            if (peek_token(1).type == TokenType::Newline || 
                peek_token(1).type == TokenType::Dedent ||
                peek_token(1).type == TokenType::EndOfFile ||
                is_at_end()) {
                // Standalone ; - block terminator
                advance();  // consume the ;
                break;
            }
            // It's a statement-ending semicolon, consume and continue
            advance();
            continue;
        }
        
        // FIX: Check for fail keyword at START OF LINE (after newline) - block terminator for if-else
        // Must check this BEFORE calling parse_statement since fail is a keyword
        if (check(TokenType::Kw_Fail)) {
            // This is the else branch - don't consume it, let the parent handle it
            break;
        }
        
        if (is_at_end() || check(TokenType::Dedent)) break;
        
        Stmt* stmt = parse_statement();
        if (stmt) {
            builder_.add_statement(block, stmt);
        }
        
        // Skip newlines between statements
        while (check(TokenType::Newline)) {
            advance();
        }
    }
    
    // Consume DEDENT if present
    if (check(TokenType::Dedent)) {
        advance();
    }
    
    return block;
}

/**
 * @brief Parse manifest statement (variable declaration - Phase 7 VeldoCra)
 * SEMICOLON MANDATORY: ; is now required at end of every simple statement
 */
LetStmtNode* Parser::parse_manifest_statement() {
    std::cerr << "[PARSER] parse_manifest_statement: starting" << std::endl;
    
    // Consume 'manifest'
    advance();
    std::cerr << "[PARSER] parse_manifest_statement: after advance, current token type = " 
              << static_cast<int>(current_token().type) << std::endl;
    
    // Expect identifier
    if (!check(TokenType::Identifier)) {
        std::cerr << "[PARSER] parse_manifest_statement: NOT an Identifier! Token type = " 
                  << static_cast<int>(current_token().type) << std::endl;
        report_error("Expected variable name after 'manifest'");
        return nullptr;
    }
    
    const char* name = get_token_text(current_token());
    std::cerr << "[PARSER] parse_manifest_statement: variable name = [" << static_cast<int>(current_token().type) << "] = " << name << std::endl;
    advance();
    
    Expr* initializer = nullptr;
    bool is_mutable = false;
    
    // Optional type annotation
    if (check(TokenType::Colon)) {
        advance();
        // TODO: Parse type expression
    }
    
    // Optional initializer
    if (check(TokenType::Assign) || check(TokenType::ColonAssign)) {
        std::cerr << "[PARSER] parse_manifest_statement: found '='" << std::endl;
        advance();
        initializer = parse_expression();
    }
    
    // HYBRID: Semicolon optional for manifest - use newline if no ;
    if (check(TokenType::Semicolon)) {
        std::cerr << "[PARSER] parse_manifest_statement: found ';'" << std::endl;
        advance(); // Consume the semicolon
    } else if (check(TokenType::Newline)) {
        std::cerr << "[PARSER] parse_manifest_statement: using newline as terminator" << std::endl;
        advance();
    } // else let parent handle
    
    LetStmtNode* result = builder_.create_let(name, initializer, is_mutable);
    std::cerr << "[PARSER] parse_manifest_statement: result = " << result << std::endl;
    return result;
}

/**
 * @brief Parse const statement (constant declaration)
 * HYBRID: Accepts optional semicolon OR newline as statement terminator
 */
LetStmtNode* Parser::parse_const_statement() {
    // Consume 'const'
    advance();
    
    // Expect identifier
    if (!check(TokenType::Identifier)) {
        report_error("Expected constant name after 'const'");
        return nullptr;
    }
    
    const char* name = get_token_text(current_token());
    advance();
    
    Expr* initializer = nullptr;
    
    // Optional type annotation
    if (check(TokenType::Colon)) {
        advance();
        // TODO: Parse type expression
    }
    
    // Require initializer
    if (check(TokenType::Assign) || check(TokenType::ColonAssign)) {
        advance();
        initializer = parse_expression();
    } else {
        report_error("Constant requires initializer");
    }
    
    // HYBRID FIX: Consume semicolon if present
    if (check(TokenType::Semicolon)) {
        advance();
    }
    // If no semicolon, let parse_program handle the NEWLINE
    
    // Constants are not mutable
    return builder_.create_let(name, initializer, false);
}

/**
 * @brief Parse function definition
 * HYBRID: Accepts optional semicolon OR newline as statement terminator
 */
FnStmtNode* Parser::parse_function_definition() {
    // Name already consumed by dispatch in parse_statement()
    // Expect function name to be current token
    if (!check(TokenType::Identifier)) {
        report_error("Expected function name");
        return nullptr;
    }
    
    const char* name = get_token_text(current_token());
    advance();
    
    auto* fn = builder_.create_fn(name);
    
    // Parse parameters
    if (check(TokenType::ParenOpen)) {
        advance();
        
        while (!check(TokenType::ParenClose) && !is_at_end()) {
            if (check(TokenType::Identifier)) {
                ParamDeclNode* param = builder_.create<ParamDeclNode>();
                param->kind = NodeKind::ParamDecl;
                param->name = get_token_text(current_token());
                param->type_expr = nullptr;
                param->default_value = nullptr;
                advance();
                
                builder_.add_param(fn, param);
                
                if (check(TokenType::Comma)) {
                    advance();
                }
            } else {
                break;
            }
        }
        
        if (check(TokenType::ParenClose)) {
            advance();
        }
    }
    
    // Parse return type
    if (check(TokenType::Arrow)) {
        advance();
        // TODO: Parse return type expression
    }
    
    // Parse body using hybrid block (supports {} braces)
    Stmt* body = parse_hybrid_block();
    builder_.set_fn_body(fn, body);
    
    // The hybrid block consumed inner }, but feat's outer } needs consuming
    if (check(TokenType::BraceClose)) {
        advance();
    }
    
    return fn;
}

/**
 * @brief Parse parameter
 */
ParamDeclNode* Parser::parse_parameter() {
    if (!check(TokenType::Identifier)) {
        return nullptr;
    }
    
    auto* param = builder_.create<ParamDeclNode>();
    param->kind = NodeKind::ParamDecl;
    param->name = get_token_text(current_token());
    param->type_expr = nullptr;
    param->default_value = nullptr;
    advance();
    
    // TODO: Parse type annotation
    if (check(TokenType::Colon)) {
        advance();
        // Skip type for now
    }
    
    // TODO: Parse default value
    if (check(TokenType::Assign)) {
        advance();
        param->default_value = parse_expression();
    }
    
    return param;
}

/**
 * @brief Parse if statement
 * Uses category-based dispatch - handles both OLD and NEW keywords
 * HYBRID: Supports both {} and : with ; syntax
 */
IfStmtNode* Parser::parse_if_statement() {
    // Consume 'if' or 'verdict' - both are in FlowControl category
    advance();
    
    // Parse condition
    Expr* condition = parse_expression();
    
    // Parse then branch using hybrid block
    Stmt* then_branch = parse_hybrid_block();
    
    // FIX: After parse_hybrid_block returns, the BraceClose should already be consumed
    // Just skip newlines and DEDENT/INDENT, but NOT fail/else keywords
    // These are the block terminators we need to check
    while (check(TokenType::Newline) || check(TokenType::Dedent) || check(TokenType::Indent)) {
        std::cerr << "[PARSER] parse_if_statement: skipping token " << static_cast<int>(current_token().type) << std::endl;
        advance();
    }
    
    std::cerr << "[PARSER] parse_if_statement: after then_branch, current token = " 
              << static_cast<int>(current_token().type) 
              << " text = '" << std::string(get_token_view(current_token())) << "'" << std::endl;
    
    // Handle ELSE keywords - only NEW (fail) is in FlowControl category
    Stmt* else_branch = nullptr;
    if (check(TokenType::Kw_Fail)) {
        std::cerr << "[PARSER] parse_if_statement: found fail!" << std::endl;
        advance();
        
        // Skip newlines after fail
        while (check(TokenType::Newline)) {
            advance();
        }
        
        // Check for else-if chain: fail verdict cond { }
        if (check(TokenType::Kw_Verdict)) {
            std::cerr << "[PARSER] parse_if_statement: found fail verdict (else-if chain)" << std::endl;
            else_branch = parse_if_statement();  // Recursive call handles next verdict
        } else {
            std::cerr << "[PARSER] parse_if_statement: plain fail else branch" << std::endl;
            else_branch = parse_hybrid_block();
        }
    }
    
    return builder_.create_if(condition, then_branch, else_branch);
}

/**
 * @brief Parse while statement
 * HYBRID: Supports both {} and : with ; syntax
 */
WhileStmtNode* Parser::parse_while_statement() {
    // Consume 'while' or 'sustain'
    advance();
    
    // Parse condition
    Expr* condition = parse_expression();
    
    // Parse body using hybrid block
    Stmt* body = parse_hybrid_block();
    
    return builder_.create_while(condition, body);
}

/**
 * @brief Parse for statement
 * HYBRID: Supports both {} and : with ; syntax
 */
ForStmtNode* Parser::parse_for_statement() {
    // Consume 'for' or 'cycle'
    advance();
    
    // Parse iterator variable
    if (!check(TokenType::Identifier)) {
        report_error("Expected iterator variable name");
        return nullptr;
    }
    
    const char* iterator_name = get_token_text(current_token());
    advance();
    
    // Expect 'in' keyword
    if (!check(TokenType::Identifier) || get_token_view(current_token()) != "in") {
        report_error("Expected 'in' keyword");
        return nullptr;
    }
    advance();
    
    // Parse iterable expression
    Expr* iterable = parse_expression();
    
    // Parse body using hybrid block
    Stmt* body = parse_hybrid_block();
    
    return builder_.create_for(iterator_name, iterable, body);
}

/**
 * @brief Parse return statement
 * HYBRID: Accepts optional semicolon OR newline as statement terminator
 */
ReturnStmtNode* Parser::parse_return_statement() {
    // Consume 'return'
    advance();
    
    // Parse return value (optional)
    Expr* value = nullptr;
    if (!check(TokenType::Newline) && !is_at_end()) {
        value = parse_expression();
    }
    
    // HYBRID: Optionally consume semicolon - accepts both ; and newline
    if (check(TokenType::Semicolon)) {
        advance();
    }
    
    return builder_.create_return(value);
}

// ============================================================================
// Expression Parsing (Precedence Climbing)
// ============================================================================

/**
 * @brief Parse expression (lowest precedence)
 */
Expr* Parser::parse_expression() {
    return parse_assignment();
}

/**
 * @brief Parse assignment expression
 */
Expr* Parser::parse_assignment() {
    Expr* left = parse_logical_or();
    
    if (check(TokenType::Assign) || check(TokenType::ColonAssign)) {
        uint16_t op = static_cast<uint16_t>(current_token().type);
        advance();
        Expr* right = parse_assignment();
        return builder_.create_binary(left, op, right);
    }
    
    return left;
}

/**
 * @brief Parse logical OR (||)
 */
Expr* Parser::parse_logical_or() {
    Expr* left = parse_logical_and();
    
    while (check(TokenType::Or)) {
        uint16_t op = static_cast<uint16_t>(current_token().type);
        advance();
        Expr* right = parse_logical_and();
        left = builder_.create_binary(left, op, right);
    }
    
    return left;
}

/**
 * @brief Parse logical AND (&&)
 */
Expr* Parser::parse_logical_and() {
    Expr* left = parse_equality();
    
    while (check(TokenType::And)) {
        uint16_t op = static_cast<uint16_t>(current_token().type);
        advance();
        Expr* right = parse_equality();
        left = builder_.create_binary(left, op, right);
    }
    
    return left;
}

/**
 * @brief Parse equality (==, !=)
 */
Expr* Parser::parse_equality() {
    Expr* left = parse_comparison();
    
    while (check(TokenType::Eq) || check(TokenType::Neq)) {
        uint16_t op = static_cast<uint16_t>(current_token().type);
        advance();
        Expr* right = parse_comparison();
        left = builder_.create_binary(left, op, right);
    }
    
    return left;
}

/**
 * @brief Parse comparison (<, >, <=, >=)
 */
Expr* Parser::parse_comparison() {
    Expr* left = parse_term();
    
    while (check(TokenType::Lt) || check(TokenType::Gt) ||
           check(TokenType::Lte) || check(TokenType::Gte)) {
        uint16_t op = static_cast<uint16_t>(current_token().type);
        advance();
        Expr* right = parse_term();
        left = builder_.create_binary(left, op, right);
    }
    
    return left;
}

/**
 * @brief Parse term (+, -)
 */
Expr* Parser::parse_term() {
    Expr* left = parse_factor();
    
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        uint16_t op = static_cast<uint16_t>(current_token().type);
        advance();
        Expr* right = parse_factor();
        left = builder_.create_binary(left, op, right);
    }
    
    return left;
}

/**
 * @brief Parse factor (*, /, %)
 */
Expr* Parser::parse_factor() {
    Expr* left = parse_unary();
    
    while (check(TokenType::Star) || check(TokenType::Slash) || 
           check(TokenType::Percent)) {
        std::cerr << "[PARSER] Found modulo/arith op: " << static_cast<int>(current_token().type) << std::endl;
        uint16_t op = static_cast<uint16_t>(current_token().type);
        advance();
        Expr* right = parse_unary();
        left = builder_.create_binary(left, op, right);
    }
    
    return left;
}

/**
 * @brief Parse unary operators (-, !, ~)
 */
Expr* Parser::parse_unary() {
    if (check(TokenType::Minus) || check(TokenType::Exclaim) || 
        check(TokenType::Tilde)) {
        uint16_t op = static_cast<uint16_t>(current_token().type);
        advance();
        Expr* operand = parse_unary();
        return builder_.create_unary(op, operand);
    }
    
    return parse_postfix();
}

/**
 * @brief Parse postfix expressions (function calls, member access, etc.)
 */
Expr* Parser::parse_postfix() {
    Expr* expr = parse_primary();
    
    while (true) {
        if (check(TokenType::ParenOpen)) {
            // Function call
            expr = parse_call(expr);
        } else if (check(TokenType::Dot)) {
            // Member access
            advance();
            if (check(TokenType::Identifier)) {
                const char* member = get_token_text(current_token());
                advance();
                // TODO: Create member expression
            }
        } else if (check(TokenType::BracketOpen)) {
            // Index expression
            advance();
            Expr* index = parse_expression();
            if (check(TokenType::BracketClose)) {
                advance();
            }
            // TODO: Create index expression
            (void)index; // Suppress unused warning
        } else {
            break;
        }
    }
    
    return expr;
}

/**
 * @brief Parse primary (literals, identifiers, parenthesized expressions)
 */
Expr* Parser::parse_primary() {
    // Literals
    if (check(TokenType::Integer) || check(TokenType::Float) ||
        check(TokenType::String) || check(TokenType::Char) ||
        check(TokenType::Boolean) || check(TokenType::Kw_True) ||
        check(TokenType::Kw_False) || check(TokenType::Kw_None)) {
        return parse_literal();
    }
    
    // Parenthesized expression
    if (check(TokenType::ParenOpen)) {
        advance();
        Expr* expr = parse_expression();
        if (check(TokenType::ParenClose)) {
            advance();
        }
        return expr;
    }
    
    // Identifier (variable reference or function call)
    if (check(TokenType::Identifier)) {
        const char* name = get_token_text(current_token());
        advance();
        return builder_.create_variable(name);
    }
    
    // Print function
    if (check(TokenType::Kw_Print)) {
        advance();
        if (check(TokenType::ParenOpen)) {
            advance();
            Expr* arg = parse_expression();
            if (check(TokenType::ParenClose)) {
                advance();
            }
            auto* call = builder_.create_call(builder_.create_variable("print"));
            builder_.add_argument(call, arg);
            return call;
        }
    }
    
    // ego keyword (this) - with member access
    if (check(TokenType::Kw_Ego)) {
        advance();
        // Create implicit "this" reference
        auto* this_ref = builder_.create_variable("ego");
        
        // Check for member access
        if (check(TokenType::Dot)) {
            advance();
            if (check(TokenType::Identifier)) {
                const char* member = get_token_text(current_token());
                advance();
                return builder_.create_member(this_ref, member, true);  // is_ego = true
            }
        }
        // Just ego by itself - return as variable
        return this_ref;
    }
    
    // core keyword (self) - with member access
    if (check(TokenType::Kw_Core)) {
        advance();
        // Create implicit "self" reference
        auto* self_ref = builder_.create_variable("core");
        
        // Check for member access
        if (check(TokenType::Dot)) {
            advance();
            if (check(TokenType::Identifier)) {
                const char* member = get_token_text(current_token());
                advance();
                return builder_.create_member(self_ref, member, false);  // is_ego = false
            }
        }
        // Just core by itself - return as variable
        return self_ref;
    }
    
    // Memory keywords with parentheses: forge, pur, clm, rsz
    if (check(TokenType::Kw_Forge) || check(TokenType::Kw_Pur) || 
        check(TokenType::Kw_Clm) || check(TokenType::Kw_Rsz)) {
        const char* func_name = get_token_text(current_token());
        advance();
        
        if (check(TokenType::ParenOpen)) {
            return parse_call(builder_.create_variable(func_name));
        }
        // If no paren, treat as variable
        return builder_.create_variable(func_name);
    }
    
    // Memory keywords without parentheses: ee, ee_inf_layers, vec, echo, touch
    // These are unary operations: ee ptr, ee_inf_layers obj, vec name, echo name, touch var
    if (check(TokenType::Kw_Ee) || check(TokenType::Kw_EeInfLayers) ||
        check(TokenType::Kw_Vec) || check(TokenType::Kw_Echo) || check(TokenType::Kw_Touch)) {
        TokenType mem_op = current_token().type;
        advance();
        
        // These keywords take an operand (variable name)
        if (check(TokenType::Identifier)) {
            const char* operand = get_token_text(current_token());
            advance();
            // Create unary expression with the memory operation
            // op code: 1000 = ee, 1001 = ee_inf_layers, 1002 = vec, 1003 = echo, 1004 = touch
            uint16_t op = 0;
            if (mem_op == TokenType::Kw_Ee) op = 1000;
            else if (mem_op == TokenType::Kw_EeInfLayers) op = 1001;
            else if (mem_op == TokenType::Kw_Vec) op = 1002;
            else if (mem_op == TokenType::Kw_Echo) op = 1003;
            else if (mem_op == TokenType::Kw_Touch) op = 1004;
            
            return builder_.create_unary(op, builder_.create_variable(operand));
        }
        
        // No operand - just return as variable
        std::string_view name;
        if (mem_op == TokenType::Kw_Ee) name = "ee";
        else if (mem_op == TokenType::Kw_EeInfLayers) name = "ee_inf_layers";
        else if (mem_op == TokenType::Kw_Vec) name = "vec";
        else if (mem_op == TokenType::Kw_Echo) name = "echo";
        else name = "touch";
        return builder_.create_variable(std::string(name).c_str());
    }
    
    // System command keywords: flow, sink, rise, summon, resoul, leap, halt, petition
    // These are treated as function-like calls without parentheses
    if (check(TokenType::Kw_Flow) || check(TokenType::Kw_Sink) || 
        check(TokenType::Kw_Rise) || check(TokenType::Kw_Summon) ||
        check(TokenType::Kw_Resoul) || check(TokenType::Kw_Leap) ||
        check(TokenType::Kw_Halt) || check(TokenType::Kw_Petition)) {
        const char* cmd_name = get_token_text(current_token());
        advance();
        
        // System commands can have comma-separated arguments without parentheses
        // e.g., "flow x, y" or "sink 10"
        auto* call = builder_.create_call(builder_.create_variable(cmd_name));
        
        // Parse arguments (comma-separated expressions)
        while (!check(TokenType::Newline) && !is_at_end() && !check(TokenType::Semicolon)) {
            Expr* arg = parse_expression();
            builder_.add_argument(call, arg);
            
            if (check(TokenType::Comma)) {
                advance();
            } else {
                break;
            }
        }
        
        return call;
    }
    
    // CRITICAL FIX: Always advance on unexpected token to prevent infinite loop!
    // Show the exact token that caused the failure
    std::string_view token_text = get_token_view(current_token());
    if (token_text.empty()) {
        report_error("[PARSE-ERROR] Unexpected token at line " + std::to_string(current_token().line) + 
                    " - NO TOKEN (possible EOF or internal error)");
    } else {
        std::string error_msg = "[PARSE-ERROR] Unexpected Token '";
        error_msg += std::string(token_text) + "' at Line " + std::to_string(current_token().line);
        error_msg += ", Column " + std::to_string(current_token().column);
        report_error(error_msg);
    }
    
    // MUST advance to prevent infinite loop!
    advance();
    
    return builder_.create_none();
}

/**
 * @brief Parse literal expression
 */
LiteralExprNode* Parser::parse_literal() {
    const Token& token = current_token();
    
    switch (token.type) {
        case TokenType::Integer: {
            advance();
            std::string_view text = get_token_view(token);
            int64_t value = 0;
            // Parse integer
            for (char c : text) {
                if (c >= '0' && c <= '9') {
                    value = value * 10 + (c - '0');
                }
            }
            return builder_.create_literal(value);
        }
        
        case TokenType::Float: {
            advance();
            std::string_view text = get_token_view(token);
            double value = std::stod(std::string(text));
            return builder_.create_literal(value);
        }
        
        case TokenType::String: {
            advance();
            std::string_view text = get_token_view(token);
            // Remove quotes
            if (text.size() >= 2) {
                text = text.substr(1, text.size() - 2);
            }
            // Copy string to arena
            return builder_.create_literal(static_cast<const char*>(text.data()));
        }
        
        case TokenType::Char: {
            advance();
            std::string_view text = get_token_view(token);
            char value = text.length() > 0 ? text[0] : '\0';
            return builder_.create_literal(value);
        }
        
        case TokenType::Boolean:
        case TokenType::Kw_True:
        case TokenType::Kw_False: {
            advance();
            bool value = (token.type == TokenType::Boolean) || 
                         (token.type == TokenType::Kw_True);
            return builder_.create_literal(value);
        }
        
        case TokenType::Kw_None: {
            advance();
            return builder_.create_none();
        }
        
        default:
            report_error("Expected literal");
            advance();
            return builder_.create_none();
    }
}

/**
 * @brief Parse function call
 */
CallExprNode* Parser::parse_call(Expr* callee) {
    // Consume opening paren
    advance();
    
    auto* call = builder_.create_call(callee);
    
    // Parse arguments
    while (!check(TokenType::ParenClose) && !is_at_end()) {
        Expr* arg = parse_expression();
        builder_.add_argument(call, arg);
        
        if (check(TokenType::Comma)) {
            advance();
        }
    }
    
    // Consume closing paren
    if (check(TokenType::ParenClose)) {
        advance();
    }
    
    return call;
}

// ============================================================================
// Phase 7: OOP Definitions (race, avatar, grant, blessing)
// ============================================================================

/**
 * @brief Parse OOP definition (race, avatar, grant, blessing)
 * HYBRID: Supports both {} and : with ; syntax
 */
Stmt* Parser::parse_oop_definition() {
    // Consume the keyword (race, avatar, grant, blessing)
    const Token& keyword_token = current_token();
    advance();
    
    // Expect type/class name
    const char* class_name = nullptr;
    if (check(TokenType::Identifier)) {
        class_name = get_token_text(current_token());
        advance();
    }
    
    // Parse body using hybrid block
    BlockStmtNode* body = parse_hybrid_block();
    
    // Create a class/struct statement node
    // For now, create a block statement with the class info
    auto* class_stmt = builder_.create_block();
    
    // Add body statements to class block
    for (size_t i = 0; i < body->statement_count; i++) {
        builder_.add_statement(class_stmt, reinterpret_cast<Stmt*>(body->statements[i]));
    }
    
    return reinterpret_cast<Stmt*>(class_stmt);
}

// ============================================================================
// Phase 7: origin keyword (standalone or scoped)
// ============================================================================

/**
 * @brief Parse origin statement
 * origin can be:
 * - Standalone: origin (activates ASM mode for rest of file)
 * - Scoped: origin { ... } (activates ASM mode only in block)
 */
Stmt* Parser::parse_origin_statement() {
    // Consume 'origin'
    advance();
    
    // Check if there's a block following
    if (check(TokenType::BraceOpen) || check(TokenType::Indent) || check(TokenType::Colon)) {
        // Scoped origin: origin { ... } or origin : ... ;
        BlockStmtNode* body = parse_hybrid_block();
        
        auto* block = builder_.create_block();
        for (size_t i = 0; i < body->statement_count; i++) {
            builder_.add_statement(block, reinterpret_cast<Stmt*>(body->statements[i]));
        }
        return reinterpret_cast<Stmt*>(block);
    }
    
    // Standalone origin - just consume it (ASM mode activated)
    // The rest of the file will be treated as ASM instructions
    // Create an empty block that marks the start of ASM mode
    auto* block = builder_.create_block();
    return reinterpret_cast<Stmt*>(block);
}

// ===== Main Parse Entry Point =====

/**
 * @brief Parse the token stream
 */
ParseResult Parser::parse() {
    Node* root = parse_program();
    
    if (error_count_ > 0) {
        std::ostringstream oss;
        oss << error_count_ << " parse error(s)";
        return ParseResult::fail(oss.str());
    }
    
    return ParseResult::ok(root);
}

} // namespace parser
} // namespace veldocra

