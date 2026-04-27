/**
 * @file lexer.h
 * @brief VeldoCra Lexer - Header
 * @author Dr. Bright
 */

#pragma once

#include "frontend/lexer/token.h"
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace veldanava {
namespace lexer {

/**
 * @brief Lexer error information
 */
struct LexerError {
    uint32_t line;
    uint32_t column;
    std::string message;
};

/**
 * @brief Lexer character classification using inline x86-64 assembly
 */
class CharClassifier {
public:
    /**
     * @brief Classify character type using SIMD-optimized checks
     * @param c Character to classify
     * @return Bitmask of character properties
     */
    static uint32_t classify(char c) {
        uint32_t result;
        
        // Inline x86-64 assembly for fast classification
        __asm__ __volatile__(
            "movzbl %[c], %[result]\n"
            // Check if digit (0-9): '0' <= c <= '9'
            "movl $0, %[result]\n"
            "cmpb $0x30, %b0\n"
            "jb 1f\n"
            "cmpb $0x39, %b0\n"
            "ja 1f\n"
            "orl $1, %[result]\n"
            "jmp 9f\n"
            "1:\n"
            // Check if alpha (a-z, A-Z): ('a' <= c <= 'z') || ('A' <= c <= 'Z')
            "cmpb $0x61, %b0\n"
            "jb 2f\n"
            "cmpb $0x7a, %b0\n"
            "ja 2f\n"
            "orl $2, %[result]\n"
            "jmp 9f\n"
            "2:\n"
            "cmpb $0x41, %b0\n"
            "jb 3f\n"
            "cmpb $0x5a, %b0\n"
            "ja 3f\n"
            "orl $2, %[result]\n"
            "jmp 9f\n"
            "3:\n"
            // Check if whitespace
            "cmpb $0x20, %b0\n"
            "je 9f\n"
            "cmpb $0x09, %b0\n"
            "je 9f\n"
            "cmpb $0x0a, %b0\n"
            "je 9f\n"
            "cmpb $0x0d, %b0\n"
            "je 9f\n"
            "cmpb $0x09, %b0\n"
            "ja 9f\n"
            "orl $4, %[result]\n"
            "9:\n"
            : [result] "=&r" (result)
            : [c] "r" (c)
            : "cc"
        );
        
        return result;
    }
    
    // Character property flags
    static constexpr uint32_t IS_DIGIT    = 1 << 0;
    static constexpr uint32_t IS_ALPHA    = 1 << 1;
    static constexpr uint32_t IS_WHITESPACE = 1 << 2;
};

/**
 * @brief Main lexer class
 * 
 * Converts source text into tokens with support for:
 * - Indentation tracking (Python-style)
 * - Brace tracking (C/Rust-style)
 * - Fast scanning with inline assembly
 */
class Lexer {
public:
    /**
     * @brief Construct lexer with source
     * @param source Source code to tokenize
     * @param filename Source filename for error reporting
     */
    Lexer(std::string_view source, std::string_view filename = "<unknown>");
    ~Lexer() = default;
    
    // Disable copying
    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;
    
    // Allow moving
    Lexer(Lexer&&) = default;
    Lexer& operator=(Lexer&&) = default;
    
    /**
     * @brief Tokenize the entire source
     * @return TokenStream containing all tokens
     */
    TokenStream tokenize();
    
    /**
     * @brief Get next token
     * @return Next token
     */
    Token next_token();
    
    /**
     * @brief Peek at next token without consuming
     * @return Next token without advancing
     */
    Token peek_token();
    
    /**
     * @brief Get current token
     * @return Current token
     */
    const Token& current_token() const { return current_; }
    
    /**
     * @brief Check if at end of input
     */
    bool is_at_end() const { return position_ >= source_.size(); }
    
    /**
     * @brief Advance to next token
     */
    void advance();
    
    /**
     * @brief Get current position
     */
    size_t get_position() const { return position_; }
    
    /**
     * @brief Get current line number
     */
    uint32_t get_line() const { return line_; }
    
    /**
     * @brief Get current column number
     */
    uint32_t get_column() const { return column_; }
    
    /**
     * @brief Get source being lexed
     */
    std::string_view get_source() const { return source_; }
    
    /**
     * @brief Check for errors
     */
    bool has_errors() const { return error_count_ > 0; }
    
    /**
     * @brief Get error count
     */
    size_t get_error_count() const { return error_count_; }

    /**
     * @brief Get errors
     */
    const std::vector<LexerError>& get_errors() const { return errors_; }
    
private:
    std::string_view source_;
    std::string_view filename_;
    
    size_t position_;      // Current position in source
    uint32_t line_;        // Current line number (1-based)
    uint32_t column_;      // Current column number (1-based)
    
    Token current_;        // Current token
    Token next_;           // Next token (lookahead)
    
    size_t error_count_;   // Number of errors
    std::vector<LexerError> errors_; // Error details
    
    // Indentation tracking
    std::vector<uint32_t> indent_stack_;
    std::vector<Token> pending_indent_tokens_;  // Buffer for INDENT/DEDENT tokens
    
    // Helper methods
    char peek_char() const;
    char peek_next_char() const;
    void skip_whitespace();
    Token handle_newline();
    void handle_indentation();
    Token make_token(TokenType type, uint16_t length);
    uint32_t current_indent_;
    
    // Source tracking
    bool at_line_start_;
    
    // Token stream
    TokenStream tokens_;
    
    // Character classification
    CharClassifier classifier_;
    
    // Transpile directive
    std::string transpile_directive_;
    
    // Lexer helper functions (scanners)
    Token scan_identifier();
    Token scan_number();
    Token scan_string();
    Token scan_char();
    Token scan_operator();
    bool is_alpha_underscore(char c) const;
    bool is_digit(char c) const;
    bool is_whitespace(char c) const;
    void skip_single_line_comment();
    bool skip_multi_line_comment();
    void report_error(std::string_view message);
    
    /**
     * @brief Get text for a token
     */
    std::string_view get_token_text(const Token& token) const {
        return source_.substr(token.offset, token.length);
    }
    
    /**
     * @brief Get current transpile directive
     */
    const std::string& get_transpile_directive() const { return transpile_directive_; }
};

} // namespace lexer
} // namespace veldanava


