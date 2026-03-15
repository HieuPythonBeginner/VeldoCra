/**
 * @file lexer.cpp
 * @brief VeldoCra Lexer - Implementation
 * @author Dr. Bright
 */

#include "lexer.h"
#include "keyword_table.h"
#include <cstring>
#include <algorithm>
#include <iostream>
#include <unordered_set>

namespace veldocra {
namespace lexer {

/**
 * @brief Construct lexer with source
 */
Lexer::Lexer(std::string_view source, std::string_view filename)
    : source_(source)
    , filename_(filename)
    , position_(0)
    , line_(1)
    , column_(1)
    , error_count_(0)
    , current_indent_(0)
    , at_line_start_(true) {
    // Don't initialize current_ to EOF - it will be set during tokenization
    // Initialize next_ as placeholder
    next_ = {TokenType::EndOfFile, 0, 0, 0, 1, 1};
    
    // Reserve space for tokens (estimate based on source size)
    tokens_.reserve(source.size() / 4);
}

/**
 * @brief Tokenize the entire source
 */
TokenStream Lexer::tokenize() {
    // Clear any existing tokens
    tokens_.clear();
    
    // Initialize indentation stack with base level
    indent_stack_.clear();
    indent_stack_.push_back(0);
    current_indent_ = 0;
    
    // Get the first token before entering the loop
    current_ = next_token();
    
    std::cerr << "[LEXER] First token type: " << static_cast<int>(current_.type) << std::endl;
    
    // Tokenize until end of file
    while (!is_at_end()) {
        // Handle line start for indentation tracking
        if (at_line_start_) {
            handle_indentation();
            at_line_start_ = false;
        }
        
        // Get next token
        Token token = current_;
        
        std::cerr << "[LEXER] Char: " << peek_char() << " -> Processing token type: " << static_cast<int>(token.type) << std::endl;
        
        // Skip certain tokens
        if (token.type == TokenType::Comment) {
            current_ = next_token();
            std::cerr << "[LEXER] After comment, token type: " << static_cast<int>(current_.type) << std::endl;
            continue;
        }
        
        // Handle newlines - may need to emit INDENT/DEDENT
        if (token.type == TokenType::Newline) {
            tokens_.push_back(token);
            at_line_start_ = true;
            current_ = next_token();
            std::cerr << "[LEXER] After newline, token type: " << static_cast<int>(current_.type) << std::endl;
            continue;
        }
        
        // Add token to stream
        tokens_.push_back(token);
        
        // Get next token for next iteration
        current_ = next_token();
        std::cerr << "[LEXER] After pushing [" << tokens_.size() << " tokens], next token type: " << static_cast<int>(current_.type) << std::endl;
    }
    
    // Handle end-of-file indentation
    if (current_indent_ > 0) {
        // Emit DEDENT tokens for remaining indentation levels
        while (indent_stack_.size() > 1) {
            indent_stack_.pop_back();
            Token dedent;
            dedent.type = TokenType::Dedent;
            dedent.flags = 0;
            dedent.offset = static_cast<uint32_t>(position_);
            dedent.length = 0;
            dedent.line = line_;
            dedent.column = column_;
            tokens_.push_back(dedent);
        }
    }
    
    // Add EOF token
    Token eof;
    eof.type = TokenType::EndOfFile;
    eof.flags = 0;
    eof.offset = static_cast<uint32_t>(position_);
    eof.length = 0;
    eof.line = line_;
    eof.column = column_;
    tokens_.push_back(eof);
    
    return std::move(tokens_);
}

/**
 * @brief Get next token
 */
Token Lexer::next_token() {
    // Skip whitespace at line start
    if (at_line_start_) {
        skip_whitespace();
    }
    
    // Check for end of source
    if (position_ >= source_.size()) {
        return make_token(TokenType::EndOfFile, 0);
    }
    
    // Get current character
    char c = peek_char();
    char next = peek_next_char();
    
    // Handle newlines
    if (is_newline(c)) {
        return handle_newline();
    }
    
    // Skip whitespace
    if (c == ' ' || c == '\t' || c == '\r') {
        skip_whitespace();
        c = peek_char();
    }
    
    // Handle end after whitespace
    if (position_ >= source_.size() || is_newline(c)) {
        return make_token(TokenType::Newline, 0);
    }
    
    // Handle comments
    if (c == '#') {
        skip_comment();
        return next_token();
    }
    
    // Handle single-line comments (//)
    if (c == '/' && next == '/') {
        skip_single_line_comment();
        return next_token();
    }
    
    // Handle multi-line comments (/* */)
    if (c == '/' && next == '*') {
        if (!skip_multi_line_comment()) {
            report_error("Unterminated multi-line comment");
        }
        return next_token();
    }
    
    // Handle identifiers and keywords
    if (is_alpha_underscore(c)) {
        return scan_identifier();
    }
    
    // Handle VeldoCra comment keywords: cmt, mul_cmt
    // These are treated as identifiers, but will be handled in scan_identifier
    
    // Handle numbers
    if (is_digit(c)) {
        return scan_number();
    }
    
    // Handle strings
    if (c == '"' || c == '\'') {
        return scan_string();
    }
    
    // Handle operators and punctuation
    return scan_operator();
}

/**
 * @brief Peek at next token without consuming
 */
Token Lexer::peek_token() {
    // Save state
    size_t saved_pos = position_;
    uint32_t saved_line = line_;
    uint32_t saved_col = column_;
    bool saved_line_start = at_line_start_;
    
    // Get next token
    Token token = next_token();
    
    // Restore state
    position_ = saved_pos;
    line_ = saved_line;
    column_ = saved_col;
    at_line_start_ = saved_line_start;
    
    return token;
}

/**
 * @brief Advance to next token
 */
void Lexer::advance() {
    current_ = next_token();
}

/**
 * @brief Advance to next character
 */
char Lexer::advance_char() {
    char c = source_[position_++];
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

/**
 * @brief Peek at current character
 */
char Lexer::peek_char() const {
    if (position_ >= source_.size()) {
        return '\0';
    }
    return source_[position_];
}

/**
 * @brief Peek at next character
 */
char Lexer::peek_next_char() const {
    if (position_ + 1 >= source_.size()) {
        return '\0';
    }
    return source_[position_ + 1];
}

/**
 * @brief Skip whitespace
 */
void Lexer::skip_whitespace() {
    while (position_ < source_.size()) {
        char c = peek_char();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance_char();
        } else {
            break;
        }
    }
}

/**
 * @brief Skip comments
 */
void Lexer::skip_comment() {
    while (position_ < source_.size() && !is_newline(peek_char())) {
        advance_char();
    }
}

/**
 * @brief Skip single-line comments (//)
 */
void Lexer::skip_single_line_comment() {
    while (position_ < source_.size() && !is_newline(peek_char())) {
        advance_char();
    }
}

/**
 * @brief Skip multi-line comments
 * @return true if comment was properly closed, false otherwise
 */
bool Lexer::skip_multi_line_comment() {
    advance_char(); // consume '*'
    advance_char(); // consume '/'
    
    while (position_ < source_.size()) {
        if (peek_char() == '*' && peek_next_char() == '/') {
            advance_char(); // consume '*'
            advance_char(); // consume '/'
            return true;
        }
        advance_char();
    }
    
    return false; // Unterminated comment
}

/**
 * @brief Skip multi-line comments with { }
 * @return true if comment was properly closed, false otherwise
 */
bool Lexer::skip_multiline_brace_comment() {
    int brace_depth = 1;
    
    while (position_ < source_.size()) {
        char c = peek_char();
        
        if (c == '{') {
            brace_depth++;
            advance_char();
        } else if (c == '}') {
            brace_depth--;
            if (brace_depth == 0) {
                advance_char(); // consume '}'
                return true;
            }
            advance_char();
        } else if (c == '\n') {
            advance_char();
            line_++;
            column_ = 1;
        } else if (c == '\r') {
            advance_char();
            if (peek_char() == '\n') {
                advance_char();
            }
            line_++;
            column_ = 1;
        } else {
            advance_char();
        }
    }
    
    return false; // Unterminated comment
}

/**
 * @brief Scan identifier or keyword
 */
Token Lexer::scan_identifier() {
    uint32_t start_offset = static_cast<uint32_t>(position_);
    uint32_t start_line = line_;
    uint32_t start_column = column_;
    
    // Read identifier
    while (position_ < source_.size() && is_alphanumeric(peek_char())) {
        advance_char();
    }
    
    uint32_t length = static_cast<uint32_t>(position_) - start_offset;
    std::string_view text = source_.substr(start_offset, length);
    
    // Check for VeldoCra comment keywords
    if (text == "cmt") {
        // Single-line comment keyword - create a comment token to be skipped
        Token comment_token;
        comment_token.type = TokenType::Comment;
        comment_token.flags = 0;
        comment_token.offset = start_offset;
        comment_token.length = static_cast<uint16_t>(length);
        comment_token.line = start_line;
        comment_token.column = start_column;
        
        // Skip the comment content
        skip_single_line_comment();
        
        // Return the comment token - it will be filtered in tokenize()
        return comment_token;
    }
    
    if (text == "mul_cmt") {
        // Multi-line comment with { }
        // Skip whitespace and check for opening brace
        skip_whitespace();
        if (peek_char() == '{') {
            advance_char(); // consume '{'
            if (!skip_multiline_brace_comment()) {
                report_error("Unterminated multi-line comment");
            }
            return next_token();
        } else {
            // If no brace, treat as single-line comment
            skip_single_line_comment();
            return next_token();
        }
    }
    
    // PHASE 7: 'let' is FORBIDDEN - return error with clear message
    // Use 'manifest' to bring your variables to life!
    if (text == "let") {
        Token error_token;
        error_token.type = TokenType::Error;
        error_token.flags = 0;
        error_token.offset = start_offset;
        error_token.length = static_cast<uint16_t>(length);
        error_token.line = start_line;
        error_token.column = start_column;
        
        // Report the error with the Overlord's message
        std::cerr << "[LEXER-ERROR] 'let' is a forbidden soul. Use 'manifest' to bring your variables to life!" << std::endl;
        error_count_++;
        
        return error_token;
    }
    
    // Check for keywords - BUT skip traditional keywords to allow them as variable names
    // VeldoCra uses Phase 7 keywords (verdict, fail, race, etc.) instead of traditional ones
    TokenType type = TokenType::Identifier; // Default to identifier
    
    // Only check for Phase 7 keywords (non-traditional)
    static const std::unordered_set<std::string> phase7_keywords = {
        // Declaration - Phase 7 only
        "av", "limit", "senect", "immo", "omni", "evolve", "species",
        // Flow Control - Phase 7 only  
        "verdict", "fail", "path", "analysis", "cycle", "sustain", "samsara", "end", "skip",
        // Function - Phase 7 only
        "fn", "ability", "feat", "hax", "scale", "stasis", "non_linear", "wait",
        // OOP - Phase 7 only
        "race", "avatar", "grant", "blessing", "contract", "glory", "void", "core", "ego",
        // Memory - Phase 7 only
        "vec", "echo", "touch", "forge", "pur", "clm", "rsz", "ee", "manifest", 
        "ee_inf_layers", "forbidden", "anomaly", "cap",
        // System Commands - Phase 7 only
        "origin", "flow", "sink", "rise", "summon", "resoul", "leap", "halt", 
        "petition", "inhale", "exhale",
        // Error Handling - Phase 7 only
        "clash", "counter", "lowdiff", "debuff", "dispel", "end_of_canon",
        // Module - Phase 7 only
        "import", "from", "as", "absorb", "federation", "legion", "proclaim", "alias", "domain",
        // Special - kept
        "print", "true", "false", "None", "and", "or", "not",
        "cmt", "mul_cmt"
    };
    
    if (phase7_keywords.count(std::string(text))) {
        type = KeywordTable::instance().lookup(text);
    } else if (text == "i64") {
        type = TokenType::Integer;  // Treat as type hint for now, or add Kw_I64 if token.h has it
        std::cerr << "[LEXER] Found i64 type" << std::endl;
    }
    
    Token token;
    token.type = type;
    token.flags = 0;
    token.offset = start_offset;
    token.length = static_cast<uint16_t>(length);
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

/**
 * @brief Scan number literal
 */
Token Lexer::scan_number() {
    uint32_t start_offset = static_cast<uint32_t>(position_);
    uint32_t start_line = line_;
    uint32_t start_column = column_;
    
    // Read integer part
    while (position_ < source_.size() && is_digit(peek_char())) {
        advance_char();
    }
    
    // Check for decimal point
    if (peek_char() == '.' && is_digit(peek_next_char())) {
        advance_char(); // consume '.'
        while (position_ < source_.size() && is_digit(peek_char())) {
            advance_char();
        }
        
        Token token;
        token.type = TokenType::Float;
        token.flags = 0;
        token.offset = start_offset;
        token.length = static_cast<uint16_t>(position_ - start_offset);
        token.line = start_line;
        token.column = start_column;
        return token;
    }
    
    // Check for hex, octal, binary
    if (peek_char() == 'x' || peek_char() == 'X' ||
        peek_char() == 'o' || peek_char() == 'O' ||
        peek_char() == 'b' || peek_char() == 'B') {
        advance_char();
        while (position_ < source_.size() && 
               (is_hex_digit(peek_char()) || peek_char() == '_')) {
            advance_char();
        }
    }
    
    Token token;
    token.type = TokenType::Integer;
    token.flags = 0;
    token.offset = start_offset;
    token.length = static_cast<uint16_t>(position_ - start_offset);
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

/**
 * @brief Scan string literal
 */
Token Lexer::scan_string() {
    uint32_t start_offset = static_cast<uint32_t>(position_);
    uint32_t start_line = line_;
    uint32_t start_column = column_;
    
    char quote = peek_char();  // " or '
    advance_char(); // consume opening quote
    
    // Handle multi-line strings
    bool is_triple = (peek_char() == quote && peek_next_char() == quote);
    if (is_triple) {
        advance_char();
        advance_char();
    }
    
    // Scan string content
    while (position_ < source_.size()) {
        char c = peek_char();
        
        if (is_newline(c) && !is_triple) {
            report_error("Unterminated string literal");
            break;
        }
        
        if (c == '\\') {
            // Escape sequence
            advance_char();
            if (position_ < source_.size()) {
                advance_char();
            }
            continue;
        }
        
        if (c == quote) {
            if (is_triple) {
                if (peek_char() == quote && peek_next_char() == quote) {
                    advance_char();
                    advance_char();
                    advance_char();
                    break;
                }
            } else {
                advance_char(); // consume closing quote
                break;
            }
        }
        
        advance_char();
    }
    
    Token token;
    token.type = TokenType::String;
    token.flags = 0;
    token.offset = start_offset;
    token.length = static_cast<uint16_t>(position_ - start_offset);
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

/**
 * @brief Scan operator
 */
Token Lexer::scan_operator() {
    uint32_t start_offset = static_cast<uint32_t>(position_);
    uint32_t start_line = line_;
    uint32_t start_column = column_;
    
    char c = advance_char();
    char next = peek_char();
    
    // Two-character operators
    TokenType type = TokenType::Error;
    uint16_t length = 1;
    
    switch (c) {
        case ':':
            if (next == '=') {
                advance_char();
                type = TokenType::ColonAssign;
                length = 2;
            } else {
                type = TokenType::Colon;
            }
            break;
            
        case '=':
            if (next == '=') {
                advance_char();
                type = TokenType::Eq;
                length = 2;
            } else {
                type = TokenType::Assign;
            }
            break;
            
        case '!':
            if (next == '=') {
                advance_char();
                type = TokenType::Neq;
                length = 2;
            } else {
                type = TokenType::Exclaim;
            }
            break;
            
        case '<':
            if (next == '=') {
                advance_char();
                type = TokenType::Lte;
                length = 2;
            } else if (next == '<') {
                advance_char();
                type = TokenType::LShift;
                length = 2;
            } else {
                type = TokenType::Lt;
            }
            break;
            
        case '>':
            if (next == '=') {
                advance_char();
                type = TokenType::Gte;
                length = 2;
            } else if (next == '>') {
                advance_char();
                type = TokenType::RShift;
                length = 2;
            } else {
                type = TokenType::Gt;
            }
            break;
            
        case '&':
            if (next == '&') {
                advance_char();
                type = TokenType::And;
                length = 2;
            } else {
                type = TokenType::Ampersand;
            }
            break;
            
        case '|':
            if (next == '|') {
                advance_char();
                type = TokenType::Or;
                length = 2;
            } else {
                type = TokenType::Pipe;
            }
            break;
            
        case '+':
            if (next == '=') {
                advance_char();
                type = TokenType::PlusAssign;
                length = 2;
            } else {
                type = TokenType::Plus;
            }
            break;
            
        case '-':
            if (next == '=') {
                advance_char();
                type = TokenType::MinusAssign;
                length = 2;
            } else if (next == '>') {
                advance_char();
                type = TokenType::Arrow;
                length = 2;
            } else {
                type = TokenType::Minus;
            }
            break;
            
        case '*':
            if (next == '=') {
                advance_char();
                type = TokenType::StarAssign;
                length = 2;
            } else {
                type = TokenType::Star;
            }
            break;
            
        case '/':
            if (next == '=') {
                advance_char();
                type = TokenType::SlashAssign;
                length = 2;
            } else {
                type = TokenType::Slash;
            }
            break;
            
        case '(':
            type = TokenType::ParenOpen;
            break;
            
        case ')':
            type = TokenType::ParenClose;
            break;
            
        case '{':
            type = TokenType::BraceOpen;
            break;
            
        case '}':
            type = TokenType::BraceClose;
            break;
            
        case '[':
            type = TokenType::BracketOpen;
            break;
            
        case ']':
            type = TokenType::BracketClose;
            break;
            
        case ',':
            type = TokenType::Comma;
            break;
            
        case '.':
            type = TokenType::Dot;
            break;
            
        case ';':
            type = TokenType::Semicolon;
            break;
            
        case '%':
            type = TokenType::Percent;
            break;
            
        case '^':
            type = TokenType::Caret;
            break;
            
        case '~':
            type = TokenType::Tilde;
            break;
            
        case '?':
            type = TokenType::Question;
            break;
            
        default:
            report_error("Unknown character");
            type = TokenType::Error;
            break;
    }
    
    Token token;
    token.type = type;
    token.flags = 0;
    token.offset = start_offset;
    token.length = length;
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

/**
 * @brief Handle newline
 */
Token Lexer::handle_newline() {
    // Handle \r\n (Windows) and \r (old Mac)
    char c = peek_char();
    if (c == '\r') {
        advance_char();
        if (peek_char() == '\n') {
            advance_char();
        }
    } else if (c == '\n') {
        advance_char();
    }
    
    Token token;
    token.type = TokenType::Newline;
    token.flags = 0;
    token.offset = static_cast<uint32_t>(position_) - 1;
    token.length = 1;
    token.line = line_;
    token.column = column_;
    
    return token;
}

/**
 * @brief Handle indentation
 */
void Lexer::handle_indentation() {
    // Count leading whitespace
    uint32_t indent = 0;
    while (position_ < source_.size()) {
        char c = peek_char();
        if (c == ' ') {
            indent++;
            advance_char();
        } else if (c == '\t') {
            // Tab = 4 spaces (configurable)
            indent += 4;
            advance_char();
        } else {
            break;
        }
    }
    

    // Skip if line is empty or starts with a closing bracket
    // These should not affect indentation tracking
    if (position_ >= source_.size()) {
        return;  // Empty line
    }
    char next_char = peek_char();
    if (next_char == '}' || next_char == '(' || next_char == ']') {
        // Do not emit indent/dedent for lines starting with closing brackets
        return;
    }

    // Compare with current indentation level
    if (indent > current_indent_) {
        // Indent increase - emit INDENT
        indent_stack_.push_back(indent);
        current_indent_ = indent;
        
        Token token;
        token.type = TokenType::Indent;
        token.flags = 0;
        token.offset = static_cast<uint32_t>(position_);
        token.length = 0;
        token.line = line_;
        token.column = 1;
        tokens_.push_back(token);
        
    } else if (indent < current_indent_) {
        // Indent decrease - emit DEDENT(s)
        while (indent_stack_.size() > 1 && indent_stack_.back() > indent) {
            indent_stack_.pop_back();
            
            Token token;
            token.type = TokenType::Dedent;
            token.flags = 0;
            token.offset = static_cast<uint32_t>(position_);
            token.length = 0;
            token.line = line_;
            token.column = 1;
            tokens_.push_back(token);
        }
        current_indent_ = indent_stack_.back();
    }
    // If indent == current_indent_, no change
}

/**
 * @brief Make token at current position
 */
Token Lexer::make_token(TokenType type, uint16_t length) {
    Token token;
    token.type = type;
    token.flags = 0;
    token.offset = static_cast<uint32_t>(position_);
    token.length = length;
    token.line = line_;
    token.column = column_;
    return token;
}

/**
 * @brief Report error
 */
void Lexer::report_error(std::string_view message) {
    error_count_++;
    // In a full implementation, would add to diagnostic system
}

} // namespace lexer
} // namespace veldocra

