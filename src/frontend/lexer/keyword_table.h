/**
 * @file keyword_table.h
 * @brief VeldoCra Modular Keyword Table - Header
 * @author Dr. Bright
 * 
 * Extensible keyword system with:
 * - Category-based classification for type dispatch
 * - Optimized perfect hash lookup (O(1) average)
 * - Runtime keyword registration for extensibility
 */

#pragma once

#include "token.h"
#include <string_view>
#include <array>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <shared_mutex>

namespace veldocra {
namespace lexer {

/**
 * @brief Keyword categories for type-based dispatch
 * 
 * Parser uses these categories to dispatch handlers,
 * remaining "blind" to specific keyword names.
 */
enum class KeywordCategory : uint8_t {
    Unknown = 0,
    Declaration,      // let, const, mut, av, limit, immo, evolve, species
    FlowControl,       // if, else, for, while, verdict, fail, path, cycle, sustain
    Function,          // def, fn, ability, feat, return, scale
    OOP,               // class, struct, race, avatar, grant, blessing
    Memory,            // ptr, ref, vec, echo, touch, forge, manifest, free
    SystemCommand,     // origin, flow, sink, rise, summon, resoul, leap, halt, petition
    ErrorHandling,    // try, catch, panic, raise, clash, counter, lowdiff, debuff
    Module,            // include, import, module, absorb, federation, legion
    Literal,           // true, false, None
    Operator,          // and, or, not
    Comment,           // cmt, mul_cmt
    Count
};

/**
 * @brief Keyword entry with metadata for type dispatch
 */
struct KeywordEntry {
    std::string_view text;           // Keyword text
    TokenType type;                  // Token type
    KeywordCategory category;        // Category for dispatch
    std::string_view alias;          // Optional alias (e.g., "if" for "verdict")
    
    constexpr KeywordEntry() : text{}, type(TokenType::Identifier), category(KeywordCategory::Unknown), alias{} {}
    constexpr KeywordEntry(std::string_view t, TokenType tt, KeywordCategory c, std::string_view a = {})
        : text(t), type(tt), category(c), alias(a) {}
};

/**
 * @brief Perfect hash configuration for keyword lookup
 * Uses a precomputed hash function for O(1) average lookup
 */
struct HashConfig {
    static constexpr uint64_t hash(std::string_view str) {
        // FNV-1a hash - excellent for short strings
        uint64_t hash = 14695981039346656037ULL; // FNV offset basis
        for (char c : str) {
            hash ^= static_cast<uint64_t>(c);
            hash *= 1099511628211ULL; // FNV prime
        }
        return hash;
    }
    
    static constexpr uint64_t table_size = 256; // Power of 2 for fast modulo
    static constexpr uint64_t hash_mask = table_size - 1;
};

/**
 * @brief Modular Keyword Table with optimized lookup and runtime registration
 * 
 * Features:
 * - Perfect hash lookup for O(1) average performance
 * - Category-based dispatch support
 * - Thread-safe keyword registration
 * - Runtime extensibility
 */
class KeywordTable {
public:
    /**
     * @brief Get singleton instance
     */
    static KeywordTable& instance() {
        static KeywordTable table;
        return table;
    }
    
    // =========================================================================
    // Lookup Operations (O(1) average with perfect hash)
    // =========================================================================
    
    /**
     * @brief Lookup keyword by text
     * @return TokenType or Identifier if not found
     */
    TokenType lookup(std::string_view text) const {
        if (text.empty()) return TokenType::Identifier;
        
        uint64_t hash = HashConfig::hash(text);
        uint64_t index = hash & HashConfig::hash_mask;
        
        // Check primary bucket
        const auto& primary = hash_buckets_[index];
        if (primary.type != TokenType::Identifier) {
            if (primary.text == text) return primary.type;
        }
        
        // Check alias bucket
        const auto& alias = alias_buckets_[index];
        if (alias.type != TokenType::Identifier) {
            if (alias.text == text) return alias.type;
        }
        
        // Fallback to linear search for edge cases
        return lookup_linear(text);
    }
    
    /**
     * @brief Lookup category by text
     * @return KeywordCategory or Unknown if not found
     */
    KeywordCategory lookup_category(std::string_view text) const {
        if (text.empty()) return KeywordCategory::Unknown;
        
        uint64_t hash = HashConfig::hash(text);
        uint64_t index = hash & HashConfig::hash_mask;
        
        // Check primary bucket
        const auto& primary = hash_buckets_[index];
        if (primary.type != TokenType::Identifier) {
            if (primary.text == text) return primary.category;
        }
        
        // Check alias bucket
        const auto& alias = alias_buckets_[index];
        if (alias.type != TokenType::Identifier) {
            if (alias.text == text) return alias.category;
        }
        
        // Fallback to linear search
        return lookup_category_linear(text);
    }
    
    /**
     * @brief Lookup category by TokenType
     * @return KeywordCategory
     */
    KeywordCategory lookup_category_by_type(TokenType type) const {
        auto it = type_to_category_.find(type);
        if (it != type_to_category_.end()) {
            return it->second;
        }
        return KeywordCategory::Unknown;
    }
    
    /**
     * @brief Check if text is a system command
     * Uses universal pattern: lowercase alphabetic only, 3-8 chars
     * @return true if matches system command pattern
     */
    bool is_system_command(std::string_view text) const {
        // Check category first
        if (lookup_category(text) == KeywordCategory::SystemCommand) {
            return true;
        }
        
        // Universal pattern check: lowercase alpha only, 3-8 chars
        if (text.length() < 3 || text.length() > 8) return false;
        
        for (char c : text) {
            if (c < 'a' || c > 'z') return false;
        }
        
        return true;
    }
    
    /**
     * @brief Check if text is a keyword
     * @return true if keyword found
     */
    bool is_keyword(std::string_view text) const {
        return lookup(text) != TokenType::Identifier;
    }
    
    // =========================================================================
    // Runtime Registration (Thread-Safe)
    // =========================================================================
    
    /**
     * @brief Register a new keyword (thread-safe)
     * @note Use during initialization phase for best performance
     * @return true if registered successfully
     */
    bool register_keyword(std::string_view text, TokenType type, 
                         KeywordCategory category, std::string_view alias = {}) {
        std::unique_lock lock(registration_mutex_);
        
        // Check if already exists
        if (is_keyword(text)) {
            return false;
        }
        
        // Add to dynamic registry
        dynamic_keywords_.emplace_back(text, type, category, alias);
        
        // Update hash map
        uint64_t hash = HashConfig::hash(text);
        uint64_t index = hash & HashConfig::hash_mask;
        
        if (hash_buckets_[index].text.empty()) {
            hash_buckets_[index] = {text, type, category, {}};
        } else {
            // Use alias bucket
            alias_buckets_[index] = {text, type, category, {}};
        }
        
        // Update type to category map
        type_to_category_[type] = category;
        
        return true;
    }
    
    /**
     * @brief Check if keyword registration is allowed
     * @return true if registration is still open
     */
    bool can_register() const {
        return !registration_closed_;
    }
    
    /**
     * @brief Close registration (call after initialization)
     */
    void close_registration() {
        std::unique_lock lock(registration_mutex_);
        registration_closed_ = true;
    }
    
    // =========================================================================
    // Category-based Dispatch Helpers
    // =========================================================================
    
    /**
     * @brief Check if TokenType belongs to category
     */
    bool is_category(TokenType type, KeywordCategory category) const {
        return lookup_category_by_type(type) == category;
    }
    
    /**
     * @brief Get all keywords in a category
     */
    template<typename Func>
    void for_each_in_category(KeywordCategory category, Func&& func) const {
        for (const auto& entry : static_keywords_) {
            if (entry.category == category) {
                func(entry);
            }
        }
        for (const auto& entry : dynamic_keywords_) {
            if (entry.category == category) {
                func(entry);
            }
        }
    }
    
    /**
     * @brief Get system command TokenType by name
     */
    TokenType get_system_command(std::string_view text) const {
        if (is_system_command(text)) {
            return lookup(text);
        }
        return TokenType::Identifier;
    }

private:
    /**
     * @brief Private constructor - initializes static keywords
     */
    KeywordTable() {
        initialize_static_keywords();
        initialize_hash_buckets();
    }
    
    /**
     * @brief Initialize static keyword table
     */
    void initialize_static_keywords() {
        // Declaration keywords
        static_keywords_.emplace_back("let", TokenType::Kw_Let, KeywordCategory::Declaration);
        static_keywords_.emplace_back("const", TokenType::Kw_Const, KeywordCategory::Declaration);
        static_keywords_.emplace_back("mut", TokenType::Kw_Mut, KeywordCategory::Declaration);
        static_keywords_.emplace_back("av", TokenType::Kw_Av, KeywordCategory::Declaration);
        static_keywords_.emplace_back("limit", TokenType::Kw_Limit, KeywordCategory::Declaration);
        static_keywords_.emplace_back("senect", TokenType::Kw_Senect, KeywordCategory::Declaration);
        static_keywords_.emplace_back("immo", TokenType::Kw_Immo, KeywordCategory::Declaration);
        static_keywords_.emplace_back("omni", TokenType::Kw_Omni, KeywordCategory::Declaration);
        static_keywords_.emplace_back("evolve", TokenType::Kw_Evolve, KeywordCategory::Declaration);
        static_keywords_.emplace_back("species", TokenType::Kw_Species, KeywordCategory::Declaration);
        
        // Flow Control keywords
        static_keywords_.emplace_back("if", TokenType::Kw_If, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("else", TokenType::Kw_Else, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("elif", TokenType::Kw_Elif, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("for", TokenType::Kw_For, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("while", TokenType::Kw_While, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("verdict", TokenType::Kw_Verdict, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("fail", TokenType::Kw_Fail, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("path", TokenType::Kw_Path, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("analysis", TokenType::Kw_Analysis, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("cycle", TokenType::Kw_Cycle, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("sustain", TokenType::Kw_Sustain, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("samsara", TokenType::Kw_Samsara, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("end", TokenType::Kw_End, KeywordCategory::FlowControl);
        static_keywords_.emplace_back("skip", TokenType::Kw_Skip, KeywordCategory::FlowControl);
        
        // Function keywords
        static_keywords_.emplace_back("def", TokenType::Kw_Def, KeywordCategory::Function);
        static_keywords_.emplace_back("fn", TokenType::Kw_Fn, KeywordCategory::Function);
        static_keywords_.emplace_back("ability", TokenType::Kw_Ability, KeywordCategory::Function);
        static_keywords_.emplace_back("feat", TokenType::Kw_Feat, KeywordCategory::Function);
        static_keywords_.emplace_back("hax", TokenType::Kw_Hax, KeywordCategory::Function);
        static_keywords_.emplace_back("return", TokenType::Kw_Return, KeywordCategory::Function);
        static_keywords_.emplace_back("scale", TokenType::Kw_Scale, KeywordCategory::Function);
        static_keywords_.emplace_back("stasis", TokenType::Kw_Stasis, KeywordCategory::Function);
        static_keywords_.emplace_back("non_linear", TokenType::Kw_NonLinear, KeywordCategory::Function);
        static_keywords_.emplace_back("wait", TokenType::Kw_Wait, KeywordCategory::Function);
        
        // OOP keywords
        static_keywords_.emplace_back("class", TokenType::Kw_Class, KeywordCategory::OOP);
        static_keywords_.emplace_back("struct", TokenType::Kw_Struct, KeywordCategory::OOP);
        static_keywords_.emplace_back("race", TokenType::Kw_Race, KeywordCategory::OOP);
        static_keywords_.emplace_back("avatar", TokenType::Kw_Avatar, KeywordCategory::OOP);
        static_keywords_.emplace_back("grant", TokenType::Kw_Grant, KeywordCategory::OOP);
        static_keywords_.emplace_back("blessing", TokenType::Kw_Blessing, KeywordCategory::OOP);
        static_keywords_.emplace_back("contract", TokenType::Kw_Contract, KeywordCategory::OOP);
        static_keywords_.emplace_back("glory", TokenType::Kw_Glory, KeywordCategory::OOP);
        static_keywords_.emplace_back("void", TokenType::Kw_Void, KeywordCategory::OOP);
        static_keywords_.emplace_back("core", TokenType::Kw_Core, KeywordCategory::OOP);
        static_keywords_.emplace_back("ego", TokenType::Kw_Ego, KeywordCategory::OOP);
        
        // Memory keywords
        static_keywords_.emplace_back("vec", TokenType::Kw_Vec, KeywordCategory::Memory);
        static_keywords_.emplace_back("echo", TokenType::Kw_Echo, KeywordCategory::Memory);
        static_keywords_.emplace_back("touch", TokenType::Kw_Touch, KeywordCategory::Memory);
        static_keywords_.emplace_back("forge", TokenType::Kw_Forge, KeywordCategory::Memory);
        static_keywords_.emplace_back("pur", TokenType::Kw_Pur, KeywordCategory::Memory);
        static_keywords_.emplace_back("clm", TokenType::Kw_Clm, KeywordCategory::Memory);
        static_keywords_.emplace_back("rsz", TokenType::Kw_Rsz, KeywordCategory::Memory);
        static_keywords_.emplace_back("ee", TokenType::Kw_Ee, KeywordCategory::Memory);
        static_keywords_.emplace_back("manifest", TokenType::Kw_Manifest, KeywordCategory::Memory);
        static_keywords_.emplace_back("ee_inf_layers", TokenType::Kw_EeInfLayers, KeywordCategory::Memory);
        static_keywords_.emplace_back("forbidden", TokenType::Kw_Forbidden, KeywordCategory::Memory);
        static_keywords_.emplace_back("anomaly", TokenType::Kw_Anomaly, KeywordCategory::Memory);
        static_keywords_.emplace_back("cap", TokenType::Kw_Cap, KeywordCategory::Memory);
        
        // System Commands - TRUE FORM (Universal Pattern)
        static_keywords_.emplace_back("origin", TokenType::Kw_Origin, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("flow", TokenType::Kw_Flow, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("sink", TokenType::Kw_Sink, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("rise", TokenType::Kw_Rise, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("summon", TokenType::Kw_Summon, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("resoul", TokenType::Kw_Resoul, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("leap", TokenType::Kw_Leap, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("halt", TokenType::Kw_Halt, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("petition", TokenType::Kw_Petition, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("inhale", TokenType::Kw_Inhale, KeywordCategory::SystemCommand);
        static_keywords_.emplace_back("exhale", TokenType::Kw_Exhale, KeywordCategory::SystemCommand);
        
        // Error Handling keywords
        static_keywords_.emplace_back("clash", TokenType::Kw_Clash, KeywordCategory::ErrorHandling);
        static_keywords_.emplace_back("counter", TokenType::Kw_Counter, KeywordCategory::ErrorHandling);
        static_keywords_.emplace_back("lowdiff", TokenType::Kw_Lowdiff, KeywordCategory::ErrorHandling);
        static_keywords_.emplace_back("debuff", TokenType::Kw_Debuff, KeywordCategory::ErrorHandling);
        static_keywords_.emplace_back("dispel", TokenType::Kw_Dispel, KeywordCategory::ErrorHandling);
        static_keywords_.emplace_back("end_of_canon", TokenType::Kw_EndOfCanon, KeywordCategory::ErrorHandling);
        
        // Module keywords
        static_keywords_.emplace_back("import", TokenType::Kw_Import, KeywordCategory::Module);
        static_keywords_.emplace_back("from", TokenType::Kw_From, KeywordCategory::Module);
        static_keywords_.emplace_back("as", TokenType::Kw_As, KeywordCategory::Module);
        static_keywords_.emplace_back("absorb", TokenType::Kw_Absorb, KeywordCategory::Module);
        static_keywords_.emplace_back("federation", TokenType::Kw_Federation, KeywordCategory::Module);
        static_keywords_.emplace_back("legion", TokenType::Kw_Legion, KeywordCategory::Module);
        static_keywords_.emplace_back("proclaim", TokenType::Kw_Proclaim, KeywordCategory::Module);
        static_keywords_.emplace_back("alias", TokenType::Kw_Alias, KeywordCategory::Module);
        static_keywords_.emplace_back("domain", TokenType::Kw_Domain, KeywordCategory::Module);
        
        // Literal keywords
        static_keywords_.emplace_back("true", TokenType::Kw_True, KeywordCategory::Literal);
        static_keywords_.emplace_back("false", TokenType::Kw_False, KeywordCategory::Literal);
        static_keywords_.emplace_back("None", TokenType::Kw_None, KeywordCategory::Literal);
        
        // Operator keywords
        static_keywords_.emplace_back("and", TokenType::And, KeywordCategory::Operator);
        static_keywords_.emplace_back("or", TokenType::Or, KeywordCategory::Operator);
        static_keywords_.emplace_back("not", TokenType::Exclaim, KeywordCategory::Operator);
        
        // Comment keywords
        static_keywords_.emplace_back("cmt", TokenType::Comment, KeywordCategory::Comment);
        static_keywords_.emplace_back("mul_cmt", TokenType::Comment, KeywordCategory::Comment);
        
        // Print (special) - treat as Function category
        static_keywords_.emplace_back("print", TokenType::Kw_Print, KeywordCategory::Function);
        
        // Wildcard
        static_keywords_.emplace_back("_", TokenType::Identifier, KeywordCategory::Unknown);
        
        // Build type to category map
        for (const auto& kw : static_keywords_) {
            type_to_category_[kw.type] = kw.category;
        }
    }
    
    /**
     * @brief Initialize perfect hash buckets
     */
    void initialize_hash_buckets() {
        // Initialize with empty entries
        for (auto& bucket : hash_buckets_) {
            bucket = {{}, TokenType::Identifier, KeywordCategory::Unknown, {}};
        }
        for (auto& bucket : alias_buckets_) {
            bucket = {{}, TokenType::Identifier, KeywordCategory::Unknown, {}};
        }
        
        // Populate primary buckets
        for (const auto& kw : static_keywords_) {
            uint64_t hash = HashConfig::hash(kw.text);
            uint64_t index = hash & HashConfig::hash_mask;
            
            if (hash_buckets_[index].text.empty()) {
                hash_buckets_[index] = kw;
            } else {
                // Collision - use alias bucket
                uint64_t alias_index = (index + 1) & HashConfig::hash_mask;
                if (alias_buckets_[alias_index].text.empty()) {
                    alias_buckets_[alias_index] = kw;
                }
            }
        }
        
        // Populate alias buckets for entries with aliases
        for (const auto& kw : static_keywords_) {
            if (!kw.alias.empty()) {
                uint64_t hash = HashConfig::hash(kw.alias);
                uint64_t index = hash & HashConfig::hash_mask;
                
                if (alias_buckets_[index].text.empty()) {
                    alias_buckets_[index] = {kw.alias, kw.type, kw.category, {}};
                }
            }
        }
    }
    
    /**
     * @brief Linear search fallback
     */
    TokenType lookup_linear(std::string_view text) const {
        for (const auto& kw : static_keywords_) {
            if (kw.text == text) return kw.type;
        }
        for (const auto& kw : dynamic_keywords_) {
            if (kw.text == text) return kw.type;
        }
        return TokenType::Identifier;
    }
    
    /**
     * @brief Linear search for category
     */
    KeywordCategory lookup_category_linear(std::string_view text) const {
        for (const auto& kw : static_keywords_) {
            if (kw.text == text) return kw.category;
        }
        for (const auto& kw : dynamic_keywords_) {
            if (kw.text == text) return kw.category;
        }
        return KeywordCategory::Unknown;
    }
    
    // Data members
    std::vector<KeywordEntry> static_keywords_;
    std::vector<KeywordEntry> dynamic_keywords_;
    
    // Perfect hash buckets (primary and collision)
    std::array<KeywordEntry, HashConfig::table_size> hash_buckets_;
    std::array<KeywordEntry, HashConfig::table_size> alias_buckets_;
    
    // Type to Category mapping
    std::unordered_map<TokenType, KeywordCategory> type_to_category_;
    
    // Thread safety
    mutable std::shared_mutex registration_mutex_;
    bool registration_closed_ = false;
};

/**
 * @brief Helper class for Parser to use category-based dispatch
 */
class KeywordCategoryLookup {
public:
    /**
     * @brief Check if token type belongs to category
     */
    static bool is(TokenType type, KeywordCategory category) {
        return KeywordTable::instance().is_category(type, category);
    }
    
    /**
     * @brief Check if token is a declaration keyword
     */
    static bool is_declaration(TokenType type) {
        return is(type, KeywordCategory::Declaration);
    }
    
    /**
     * @brief Check if token is a flow control keyword
     */
    static bool is_flow_control(TokenType type) {
        return is(type, KeywordCategory::FlowControl);
    }
    
    /**
     * @brief Check if token is a function keyword
     */
    static bool is_function(TokenType type) {
        return is(type, KeywordCategory::Function);
    }
    
    /**
     * @brief Check if token is a system command
     */
    static bool is_system_command(TokenType type) {
        return is(type, KeywordCategory::SystemCommand);
    }
    
    /**
     * @brief Check if token is an error handling keyword
     */
    static bool is_error_handling(TokenType type) {
        return is(type, KeywordCategory::ErrorHandling);
    }
    
    /**
     * @brief Check if token is a module keyword
     */
    static bool is_module(TokenType type) {
        return is(type, KeywordCategory::Module);
    }
    
    /**
     * @brief Check if token is a memory keyword
     */
    static bool is_memory(TokenType type) {
        return is(type, KeywordCategory::Memory);
    }
    
    /**
     * @brief Check if token is an OOP keyword
     */
    static bool is_oop(TokenType type) {
        return is(type, KeywordCategory::OOP);
    }
    
    /**
     * @brief Get category for token type
     */
    static KeywordCategory get_category(TokenType type) {
        return KeywordTable::instance().lookup_category_by_type(type);
    }
    
    /**
     * @brief Check if text is a system command (pattern-based)
     */
    static bool is_system_command_text(std::string_view text) {
        return KeywordTable::instance().is_system_command(text);
    }
};

} // namespace lexer
} // namespace veldocra

