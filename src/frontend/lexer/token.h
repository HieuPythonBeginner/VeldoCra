/**
 * @file token.h
 * @brief VeldoCra Token Definitions
 * @author Dr. Bright
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <array>
#include <vector>

namespace veldocra {
namespace lexer {

/**
 * @brief Token types for VeldoCra
 */
enum class TokenType : uint16_t {
    // Special tokens
    EndOfFile = 0,
    Error,
    Newline,
    Indent,
    Dedent,
    
    // Literals
    Identifier,
    Integer,
    Float,
    String,
    Char,
    Boolean,
    
    // Keywords - Genesis (Declarations)
    Kw_Print,
    Kw_If,
    Kw_Else,
    Kw_Elif,
    Kw_For,
    Kw_While,
    Kw_Def,
    Kw_Class,
    Kw_Return,
    Kw_Break,
    Kw_Continue,
    Kw_Pass,
    Kw_Fn,
    Kw_Mut,
    Kw_Let,
    Kw_Const,
    Kw_Struct,
    Kw_Enum,
    Kw_Match,
    Kw_Import,
    Kw_From,
    Kw_As,
    Kw_True,
    Kw_False,
    Kw_None,
    
    // NEW: VeldoCra-themed Keywords - Genesis (Declarations)
    Kw_Av,           // var
    Kw_Limit,        // let
    Kw_Senect,       // auto
    Kw_Immo,         // const
    Kw_Omni,         // static
    Kw_Evolve,       // mut
    Kw_Species,      // type
    
    // Causality Control (Flow)
    Kw_Verdict,      // if
    Kw_Fail,         // else
    Kw_Path,         // switch
    Kw_Analysis,     // match
    Kw_Cycle,        // for
    Kw_Sustain,      // while
    Kw_Samsara,      // loop
    Kw_End,          // break
    Kw_Skip,         // continue
    
    // Abilities & Actions (Functions)
    Kw_Ability,      // def
    Kw_Feat,         // fn
    Kw_Hax,          // func
    Kw_Scale,        // return
    Kw_Stasis,       // yield
    Kw_NonLinear,    // async
    Kw_Wait,         // await
    
    // Entity Morphology (OOP)
    Kw_Race,         // class
    Kw_Avatar,       // struct
    Kw_Grant,        // impl
    Kw_Blessing,     // trait
    Kw_Contract,     // interface
    Kw_Glory,        // public
    Kw_Void,         // private
    Kw_Core,         // self
    Kw_Ego,          // this
    
    // Soul Manipulation (Memory/Low-level)
    Kw_Vec,          // ptr
    Kw_Echo,         // ref
    Kw_Touch,        // deref
    Kw_Forge,        // alloc
    Kw_Pur,          // calloc
    Kw_Clm,          // malloc
    Kw_Rsz,          // realloc
    Kw_Ee,           // free
    Kw_Manifest,     // new
    Kw_EeInfLayers,  // delete
    Kw_Forbidden,    // unsafe
    Kw_Anomaly,      // volatile
    Kw_Cap,          // sizeof
    
    // True Form (System/ASM)
    Kw_Origin,       // asm
    Kw_Flow,         // mov
    Kw_Sink,         // push
    Kw_Rise,         // pop
    Kw_Summon,       // call
    Kw_Resoul,       // ret
    Kw_Leap,         // jmp
    Kw_Halt,         // int
    Kw_Petition,     // syscall
    Kw_Inhale,       // in
    Kw_Exhale,       // out
    
    // Doomsday Protocols (Error/End)
    Kw_Clash,        // try
    Kw_Counter,      // catch, except
    Kw_Lowdiff,      // panic
    Kw_Debuff,       // raise
    Kw_Dispel,       // drop
    Kw_EndOfCanon,   // exit
    
    // Module & Import System
    Kw_Absorb,       // include
    Kw_Federation,   // module
    Kw_Legion,       // package
    Kw_Proclaim,     // export
    Kw_Alias,        // as
    Kw_Domain,       // namespace
    
    // Operators
    Plus,          // +
    Minus,         // -
    Star,          // *
    Slash,         // /
    Percent,       // %
    Ampersand,     // &
    Pipe,          // |
    Caret,         // ^
    Tilde,         // ~
    Exclaim,       // !
    Question,      // ?
    Colon,         // :
    Semicolon,     // ;
    Comma,         // ,
    Dot,           // .
    
    // Assignment
    Assign,        // =
    PlusAssign,    // +=
    MinusAssign,   // -=
    StarAssign,    // *=
    SlashAssign,   // /=
    ColonAssign,   // :=
    
    // Comparison
    Eq,            // ==
    Neq,           // !=
    Lt,            // <
   Gt,            // >
    Lte,           // <=
    Gte,           // >=
    And,           // &&
    Or,            // ||
    
    // Shift
    LShift,        // <<
    RShift,        // >>
    
    // Brackets
    ParenOpen,     // (
    ParenClose,    // )
    BraceOpen,     // {
    BraceClose,    // }
    BracketOpen,    // [
    BracketClose,   // ]
    
    // Arrow
    Arrow,         // ->
    
    // Comment
    Comment,
    
    // Count for sizing
    COUNT
};

/**
 * @brief Token flags
 */
enum class TokenFlags : uint16_t {
    None = 0,
    HasWhitespace = 1 << 0,
    IsMultiChar = 1 << 1,
    IsHotPath = 1 << 2,  // Frequently accessed token
};

/**
 * @brief Compact POD token representation
 * Maximum size: 16 bytes
 */
struct Token {
    TokenType type;          // 2 bytes
    uint16_t flags;          // 2 bytes
    uint32_t offset;        // 4 bytes (source offset)
    uint16_t length;        // 2 bytes
    uint16_t line;           // 2 bytes (line number)
    uint32_t column;        // 4 bytes (column - needs 4 for large files)
    
    // Padding to 16 bytes
    // Total: 2+2+4+2+2+4 = 16 bytes
    
    /**
     * @brief Get token text from source
     */
    constexpr std::string_view get_text(std::string_view source) const {
        return source.substr(offset, length);
    }
    
    /**
     * @brief Check if token is a keyword
     */
    constexpr bool is_keyword() const {
        return static_cast<uint16_t>(type) >= static_cast<uint16_t>(TokenType::Kw_Print) &&
               static_cast<uint16_t>(type) <= static_cast<uint16_t>(TokenType::Kw_None);
    }
    
    /**
     * @brief Check if token is an operator
     */
    constexpr bool is_operator() const {
        return static_cast<uint16_t>(type) >= static_cast<uint16_t>(TokenType::Plus) &&
               static_cast<uint16_t>(type) <= static_cast<uint16_t>(TokenType::Arrow);
    }
    
    /**
     * @brief Check if token is a literal
     */
    constexpr bool is_literal() const {
        return type == TokenType::Identifier ||
               type == TokenType::Integer ||
               type == TokenType::Float ||
               type == TokenType::String ||
               type == TokenType::Char ||
               type == TokenType::Boolean;
    }
};

/**
 * @brief Token keywords lookup table
 * 
 * NOTE: The keyword lookup functionality has been moved to keyword_table.h
 * Use veldocra::lexer::KeywordTable::lookup() for keyword lookups
 * Use veldocra::lexer::KeywordCategoryLookup for category-based dispatch
 * 
 * Include keyword_table.h directly in files that need it.
 */

// Note: The KeywordTable class with full keyword definitions is now in keyword_table.h
// This is kept for backward compatibility - the actual implementation is in keyword_table.h

/**
 * @brief Token stream for fast iteration
 */
class TokenStream {
public:
    TokenStream() = default;
    ~TokenStream() = default;
    
    // Disable copying
    TokenStream(const TokenStream&) = delete;
    TokenStream& operator=(const TokenStream&) = delete;
    
    // Allow moving
    TokenStream(TokenStream&&) = default;
    TokenStream& operator=(TokenStream&&) = default;
    
    /**
     * @brief Add token to stream
     */
    void push_back(const Token& token) {
        tokens_.push_back(token);
    }
    
    /**
     * @brief Get token at index
     */
    const Token& operator[](size_t index) const {
        return tokens_[index];
    }
    
    /**
     * @brief Get token at index with bounds check
     */
    const Token& at(size_t index) const {
        return tokens_.at(index);
    }
    
    /**
     * @brief Get number of tokens
     */
    size_t size() const { return tokens_.size(); }
    
    /**
     * @brief Check if stream is empty
     */
    bool empty() const { return tokens_.empty(); }
    
    /**
     * @brief Clear the stream
     */
    void clear() { tokens_.clear(); }
    
    /**
     * @brief Get iterator to beginning
     */
    auto begin() { return tokens_.begin(); }
    
    /**
     * @brief Get iterator to end
     */
    auto end() { return tokens_.end(); }
    
    /**
     * @brief Get const iterator to beginning
     */
    auto begin() const { return tokens_.cbegin(); }
    
    /**
     * @brief Get const iterator to end
     */
    auto end() const { return tokens_.cend(); }
    
    /**
     * @brief Reserve space for tokens
     */
    void reserve(size_t count) { tokens_.reserve(count); }
    
private:
    std::vector<Token> tokens_;
};

} // namespace lexer
} // namespace veldocra

