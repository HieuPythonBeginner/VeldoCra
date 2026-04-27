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

namespace veldanava {
namespace lexer {

// Dummy implementations for now
Lexer::Lexer(std::string_view source, std::string_view filename)
    : source_(source), filename_(filename), position_(0), line_(1), column_(1), error_count_(0),
      indent_stack_({0}) {
    current_ = Token{TokenType::EndOfFile, 0, 0, 1, 1};
}

TokenStream Lexer::tokenize() {
    TokenStream tokens;
    while (!is_at_end()) {
        Token token = next_token();
        tokens.push_back(token);
        if (token.type == TokenType::EndOfFile) {
            break;
        }
    }
    return tokens;
}

Token Lexer::next_token() {
    // Skip whitespace and comments
    while (!is_at_end()) {
        char c = peek_char();
        if (is_whitespace(c)) {
            advance();
        } else if (c == '/' && peek_next_char() == '/') {
            skip_single_line_comment();
        } else if (c == '/' && peek_next_char() == '*') {
            skip_multi_line_comment();
        } else {
            break;
        }
    }

    if (is_at_end()) {
        return make_token(TokenType::Eof, 0);
    }

    char c = peek_char();

    if (is_alpha_underscore(c)) {
        return scan_identifier();
    }

    // 2. Handle newline token
    if (!is_at_end() && peek_char() == '\n') {
        advance(); // consume '\n'
        handle_indentation(); // compute indent of next line, queue INDENT/DEDENT
        return make_token(TokenType::Newline, 1);
    }

    // 3. Skip whitespace between tokens (spaces/tabs not at line start)
    skip_whitespace();

    // 4. Check EOF after whitespace
    if (is_at_end()) {
        // If pending dedent from EOF, flush them first
        if (!pending_indent_tokens_.empty()) {
            Token t = pending_indent_tokens_.front();
            pending_indent_tokens_.erase(pending_indent_tokens_.begin());
            return t;
        }
        // If still not at base indent, emit DEDENTs to unwind
        if (indent_stack_.back() != 0) {
            while (!indent_stack_.empty() && indent_stack_.back() > 0) {
                indent_stack_.pop_back();
                pending_indent_tokens_.push_back(make_token(TokenType::Dedent, 0));
            }
            if (!pending_indent_tokens_.empty()) {
                Token t = pending_indent_tokens_.front();
                pending_indent_tokens_.erase(pending_indent_tokens_.begin());
                return t;
            }
        }
        return make_token(TokenType::EndOfFile, 0);
    }

    if (is_digit(c)) {
        return scan_number();
    }

    if (c == '"') {
        return scan_string();
    }

    // Comments
    if (c == '/' && peek_next_char() == '/') {
        skip_single_line_comment();
        return next_token();
    }
    if (c == '/' && peek_next_char() == '*') {
        if (!skip_multi_line_comment()) {
            report_error("Unterminated multi-line comment");
            return make_token(TokenType::EndOfFile, 0);
        }
        return next_token();
    }

    // Operators and punctuation
    return scan_operator();
}

Token Lexer::peek_token() {
    if (is_at_end()) {
        return make_token(TokenType::EndOfFile, 0);
    }
    
    size_t saved_pos = position_;
    size_t saved_line = line_;
    size_t saved_column = column_;
    
    Token token = next_token();
    
    position_ = saved_pos;
    line_ = saved_line;
    column_ = saved_column;
    
    return token;
}

char Lexer::peek_char() const {
    return is_at_end() ? '\0' : source_[position_];
}

char Lexer::peek_next_char() const {
    return (position_ + 1 >= source_.size()) ? '\0' : source_[position_ + 1];
}

void Lexer::skip_whitespace() {
    while (!is_at_end() && (peek_char() == ' ' || peek_char() == '\t')) {
        advance();
    }
}

void Lexer::advance() {
    if (!is_at_end()) {
        char c = source_[position_];
        position_++;
        if (c == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
    }
}

Token Lexer::handle_newline() {
    return next_token();
}

void Lexer::handle_indentation() {
    // Measure indentation (spaces/tabs) at start of the line
    uint32_t current_indent = 0;
    while (!is_at_end()) {
        char c = peek_char();
        if (c == ' ') {
            advance();
            current_indent++;
        } else if (c == '\t') {
            advance();
            current_indent += 4;  // tab = 4 spaces
        } else {
            break;
        }
    }
    
    uint32_t last_indent = indent_stack_.back();
    if (current_indent > last_indent) {
        indent_stack_.push_back(current_indent);
        pending_indent_tokens_.push_back(make_token(TokenType::Indent, 0));
    } else if (current_indent < last_indent) {
        // Emit DEDENT tokens until we find matching indent
        while (!indent_stack_.empty() && current_indent < indent_stack_.back()) {
            indent_stack_.pop_back();
            pending_indent_tokens_.push_back(make_token(TokenType::Dedent, 0));
        }
        // After popping, check if indent matches
        if (indent_stack_.empty() || indent_stack_.back() != current_indent) {
            report_error("Indentation error: inconsistent indent");
            // Reset to safe state: clear stack, set to 0
            indent_stack_.clear();
            indent_stack_.push_back(0);
        }
    }
    // If equal, nothing to emit
}

Token Lexer::make_token(TokenType type, uint16_t length) {
    Token token;
    token.type = type;
    token.offset = position_ - length;
    token.length = length;
    token.line = line_;
    token.column = column_;
    return token;
}

void Lexer::report_error(std::string_view message) {
    errors_.push_back({line_, column_, std::string(message)});
    error_count_++;
}

Token Lexer::scan_identifier() {
    size_t start = position_;
    while (!is_at_end() && (is_alpha_underscore(peek_char()) || is_digit(peek_char()))) {
        advance();
    }
    size_t length = position_ - start;
    std::string_view text(source_.data() + start, length);
    
    // Look up in keyword table
    TokenType token_type = KeywordTable::instance().lookup(text);
    return make_token(token_type, length);
}

Token Lexer::scan_number() {
    size_t start = position_;
    while (!is_at_end() && is_digit(peek_char())) {
        advance();
    }
    return make_token(TokenType::Integer, position_ - start);
}

Token Lexer::scan_char() {
    advance(); // skip '
    if (!is_at_end() && peek_char() != '\'') {
        advance(); // skip char
    }
    if (!is_at_end() && peek_char() == '\'') {
        advance(); // skip '
    }
    return make_token(TokenType::Char, 1);
}

Token Lexer::scan_string() {
    char quote = peek_char();
    advance(); // skip opening quote
    size_t start = position_;
    while (!is_at_end() && peek_char() != quote) {
        advance();
    }
    size_t length = position_ - start;
    Token token = make_token(TokenType::String, length); // Call BEFORE skipping closing quote
    if (!is_at_end()) {
        advance(); // skip closing quote
    }
    return token;
}

Token Lexer::scan_operator() {
    char c = peek_char();
    advance(); // consume character
    
    // Check for multi-character operators
    switch (c) {
        case ';': return make_token(TokenType::Semicolon, 1);
        case '(': return make_token(TokenType::ParenOpen, 1);
        case ')': return make_token(TokenType::ParenClose, 1);
        case '{': return make_token(TokenType::BraceOpen, 1);
        case '}': return make_token(TokenType::BraceClose, 1);
        case '[': return make_token(TokenType::BracketOpen, 1);
        case ']': return make_token(TokenType::BracketClose, 1);
        case ':': return make_token(TokenType::Colon, 1);
        case ',': return make_token(TokenType::Comma, 1);
        case '.': return make_token(TokenType::Dot, 1);
        case '+': return make_token(TokenType::Plus, 1);
        case '-': 
            // Check for -> (Arrow) or -= (MinusAssign)
            if (!is_at_end() && peek_char() == '>') {
                advance();
                return make_token(TokenType::Arrow, 2);
            }
            if (!is_at_end() && peek_char() == '=') {
                advance();
                return make_token(TokenType::MinusAssign, 2);
            }
            return make_token(TokenType::Minus, 1);
        case '*': 
            if (!is_at_end() && peek_char() == '=') {
                advance();
                return make_token(TokenType::StarAssign, 2);
            }
            return make_token(TokenType::Star, 1);
        case '/': return make_token(TokenType::Slash, 1);
        case '%': 
            if (!is_at_end() && peek_char() == '=') {
                advance();
                return make_token(TokenType::PercentAssign, 2);
            }
            return make_token(TokenType::Percent, 1);
        case '=': 
            // Check for => (FatArrow) or == (Eq)
            if (!is_at_end() && peek_char() == '>') {
                advance();
                return make_token(TokenType::FatArrow, 2);
            }
            if (!is_at_end() && peek_char() == '=') {
                advance();
                return make_token(TokenType::Eq, 2);
            }
            return make_token(TokenType::Assign, 1);
        case '!': 
            if (!is_at_end() && peek_char() == '=') {
                advance();
                return make_token(TokenType::Neq, 2);
            }
            return make_token(TokenType::Exclaim, 1);
        case '<': 
            if (!is_at_end() && peek_char() == '=') {
                advance();
                return make_token(TokenType::Lte, 2);
            }
            if (!is_at_end() && peek_char() == '<') {
                advance();
                return make_token(TokenType::LShift, 2);
            }
            return make_token(TokenType::Lt, 1);
        case '>': 
            if (!is_at_end() && peek_char() == '=') {
                advance();
                return make_token(TokenType::Gte, 2);
            }
            if (!is_at_end() && peek_char() == '>') {
                advance();
                return make_token(TokenType::RShift, 2);
            }
            return make_token(TokenType::Gt, 1);
        case '&': 
            if (!is_at_end() && peek_char() == '&') {
                advance();
                return make_token(TokenType::And, 2);
            }
            return make_token(TokenType::Ampersand, 1);
        case '|': 
            if (!is_at_end() && peek_char() == '|') {
                advance();
                return make_token(TokenType::Or, 2);
            }
            return make_token(TokenType::Pipe, 1);
        case '^': return make_token(TokenType::Caret, 1);
        case '~': return make_token(TokenType::Tilde, 1);
        case '?': return make_token(TokenType::Question, 1);
        default:
            return make_token(TokenType::Identifier, 1);
    }
}

bool Lexer::is_alpha_underscore(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::is_digit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::is_whitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void Lexer::skip_single_line_comment() {
    // Skip until end of line
    while (!is_at_end() && peek_char() != '\n') {
        advance();
    }
}

bool Lexer::skip_multi_line_comment() {
    // TODO: Implement proper multi-line comment skipping
    // For now, skip until end of file to avoid infinite loop
    while (!is_at_end()) {
        // Check for end of comment "*/"
        if (peek_char() == '*' && peek_next_char() == '/') {
            advance(); // consume '*'
            advance(); // consume '/'
            return true;
        }
        advance();
    }
    return false;
}

} // namespace lexer
} // namespace veldanava

