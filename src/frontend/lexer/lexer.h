/**
 * @file lexer.h
 * @brief VeldoCra Lexer - Header
 * @author Dr. Bright
 */

#pragma once

#include "frontend/lexer/token.h"
#include <string_view>
#include <vector>
#include <cstdint>

namespace veldocra {
namespace lexer {

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
    bool is_at_end() const { return current_.type == TokenType::EndOfFile; }
    
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
    
private:
    std::string_view source_;
    std::string_view filename_;
    
    size_t position_;      // Current position in source
    uint32_t line_;        // Current line number (1-based)
    uint32_t column_;      // Current column number (1-based)
    
    Token current_;        // Current token
    Token next_;           // Next token (lookahead)
    
    size_t error_count_;   // Number of errors
    
    // Indentation tracking
    std::vector<uint32_t> indent_stack_;
    uint32_t current_indent_;
    bool at_line_start_;
    
    // Token stream being built
    TokenStream tokens_;
    
    // Private methods
    
    /**
     * @brief Advance to next character
     */
    char advance_char();
    
    /**
     * @brief Peek at current character
     */
    char peek_char() const;
    
    /**
     * @brief Peek at next character
     */
    char peek_next_char() const;
    
    /**
     * @brief Skip whitespace
     */
    void skip_whitespace();
    
    /**
     * @brief Skip comments
     */
    void skip_comment();
    
    /**
     * @brief Skip single-line comments (//)
     */
    void skip_single_line_comment();
    
    /**
     * @brief Skip multi-line comments
     * @return true if comment was properly closed, false otherwise
     */
    bool skip_multi_line_comment();
    
    /**
     * @brief Skip multi-line comments with { }
     * @return true if comment was properly closed, false otherwise
     */
    bool skip_multiline_brace_comment();
    
    /**
     * @brief Scan identifier or keyword
     */
    Token scan_identifier();
    
    /**
     * @brief Scan number literal
     */
    Token scan_number();
    
    /**
     * @brief Scan string literal
     */
    Token scan_string();
    
    /**
     * @brief Scan operator
     */
    Token scan_operator();
    
    /**
     * @brief Handle newline
     */
    Token handle_newline();
    
    /**
     * @brief Handle indentation
     */
    void handle_indentation();
    
    /**
     * @brief Make token at current position
     */
    Token make_token(TokenType type, uint16_t length = 0);
    
    /**
     * @brief Report error
     */
    void report_error(std::string_view message);
    
    /**
     * @brief Check if character is newline
     */
    static bool is_newline(char c) {
        return c == '\n' || c == '\r';
    }
    
    /**
     * @brief Check if character is decimal digit
     */
    static bool is_digit(char c) {
        return c >= '0' && c <= '9';
    }
    
    /**
     * @brief Check if character is hex digit
     */
    static bool is_hex_digit(char c) {
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    }
    
    /**
     * @brief Check if character is alpha (a-z, A-Z)
     */
    static bool is_alpha(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z');
    }
    
    /**
     * @brief Check if character is alpha or underscore
     */
    static bool is_alpha_underscore(char c) {
        return is_alpha(c) || c == '_';
    }
    
    /**
     * @brief Check if character is alphanumeric
     */
    static bool is_alphanumeric(char c) {
        return is_alpha(c) || is_digit(c) || c == '_';
    }
};

} // namespace lexer
} // namespace veldocra


