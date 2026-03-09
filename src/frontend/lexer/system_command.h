/**
 * @file system_command.h
 * @brief VeldoCra System Command Handler - Header
 * @author Dr. Bright
 * 
 * System command handling with type-based dispatch.
 * Allows runtime registration of new commands.
 */

#pragma once

#include "keyword_table.h"
#include <functional>
#include <unordered_map>
#include <string_view>
#include <vector>

namespace veldocra {
namespace lexer {

/**
 * @brief System command types
 */
enum class SystemCommandType : uint8_t {
    None = 0,
    Origin,      // asm - assembly code block
    Flow,        // mov - move data
    Sink,        // push - push to stack
    Rise,        // pop - pop from stack
    Summon,      // call - function call
    Resoul,      // ret - return from function
    Leap,        // jmp - jump
    Halt,        // int - interrupt
    Petition,    // syscall - system call
    Inhale,     // in - input
    Exhale,      // out - output
    Count
};

/**
 * @brief System command entry
 */
struct SystemCommandEntry {
    std::string_view name;           // Command name (e.g., "origin")
    SystemCommandType type;          // Command type enum
    KeywordCategory category;        // Category (should be SystemCommand)
    std::string_view description;    // Description for help/docs
    
    constexpr SystemCommandEntry(std::string_view n, SystemCommandType t, 
                                  KeywordCategory c, std::string_view d = {})
        : name(n), type(t), category(c), description(d) {}
};

/**
 * @brief System command handler function type
 */
using SystemCommandHandler = std::function<void(void* context)>;

/**
 * @brief System command registry with type-based dispatch
 */
class SystemCommandRegistry {
public:
    /**
     * @brief Get singleton instance
     */
    static SystemCommandRegistry& instance() {
        static SystemCommandRegistry registry;
        return registry;
    }
    
    // =========================================================================
    // Registration
    // =========================================================================
    
    /**
     * @brief Register a new system command
     * @return true if registered successfully
     */
    bool register_command(std::string_view name, SystemCommandType type,
                         KeywordCategory category = KeywordCategory::SystemCommand,
                         std::string_view description = {}) {
        // Register in keyword table
        TokenType token_type = static_cast<TokenType>(
            static_cast<uint16_t>(TokenType::Kw_Origin) + static_cast<uint8_t>(type)
        );
        
        bool kw_registered = KeywordTable::instance().register_keyword(
            name, token_type, category
        );
        
        if (!kw_registered) {
            return false;
        }
        
        // Add to command registry
        commands_.emplace_back(name, type, category, description);
        name_to_type_[std::string(name)] = type;
        type_to_handler_[type] = nullptr;
        
        return true;
    }
    
    /**
     * @brief Register a handler for a command type
     */
    void register_handler(SystemCommandType type, SystemCommandHandler handler) {
        type_to_handler_[type] = handler;
    }
    
    // =========================================================================
    // Lookup
    // =========================================================================
    
    /**
     * @brief Get command type by name
     */
    SystemCommandType get_type(std::string_view name) const {
        auto it = name_to_type_.find(std::string(name));
        if (it != name_to_type_.end()) {
            return it->second;
        }
        return SystemCommandType::None;
    }
    
    /**
     * @brief Get command type from TokenType
     */
    SystemCommandType get_type_from_token(TokenType token_type) const {
        // Calculate offset from Kw_Origin
        uint16_t token_val = static_cast<uint16_t>(token_type);
        uint16_t origin_val = static_cast<uint16_t>(TokenType::Kw_Origin);
        
        if (token_val < origin_val) {
            return SystemCommandType::None;
        }
        
        uint8_t offset = static_cast<uint8_t>(token_val - origin_val);
        if (offset >= static_cast<uint8_t>(SystemCommandType::Count)) {
            return SystemCommandType::None;
        }
        
        return static_cast<SystemCommandType>(offset);
    }
    
    /**
     * @brief Check if token is a system command
     */
    bool is_system_command(TokenType type) const {
        return KeywordCategoryLookup::is_system_command(type);
    }
    
    /**
     * @brief Handle system command by type
     */
    void handle(SystemCommandType type, void* context = nullptr) {
        auto it = type_to_handler_.find(type);
        if (it != type_to_handler_.end() && it->second) {
            it->second(context);
        }
    }
    
    // =========================================================================
    // Iteration
    // =========================================================================
    
    /**
     * @brief Get all registered commands
     */
    const std::vector<SystemCommandEntry>& get_commands() const {
        return commands_;
    }
    
    /**
     * @brief Get number of registered commands
     */
    size_t size() const { return commands_.size(); }

private:
    /**
     * @brief Private constructor
     */
    SystemCommandRegistry() {
        initialize_default_commands();
    }
    
    /**
     * @brief Initialize default system commands
     */
    void initialize_default_commands() {
        // Register default commands
        register_command("origin", SystemCommandType::Origin, 
                        KeywordCategory::SystemCommand, "Assembly code block");
        register_command("flow", SystemCommandType::Flow,
                        KeywordCategory::SystemCommand, "Move data");
        register_command("sink", SystemCommandType::Sink,
                        KeywordCategory::SystemCommand, "Push to stack");
        register_command("rise", SystemCommandType::Rise,
                        KeywordCategory::SystemCommand, "Pop from stack");
        register_command("summon", SystemCommandType::Summon,
                        KeywordCategory::SystemCommand, "Function call");
        register_command("resoul", SystemCommandType::Resoul,
                        KeywordCategory::SystemCommand, "Return from function");
        register_command("leap", SystemCommandType::Leap,
                        KeywordCategory::SystemCommand, "Jump");
        register_command("halt", SystemCommandType::Halt,
                        KeywordCategory::SystemCommand, "Interrupt");
        register_command("petition", SystemCommandType::Petition,
                        KeywordCategory::SystemCommand, "System call");
        register_command("inhale", SystemCommandType::Inhale,
                        KeywordCategory::SystemCommand, "Input");
        register_command("exhale", SystemCommandType::Exhale,
                        KeywordCategory::SystemCommand, "Output");
    }
    
    // Data members
    std::vector<SystemCommandEntry> commands_;
    std::unordered_map<std::string, SystemCommandType> name_to_type_;
    std::unordered_map<SystemCommandType, SystemCommandHandler> type_to_handler_;
};

/**
 * @brief Helper for checking system commands in parser
 */
class SystemCommandHelper {
public:
    /**
     * @brief Check if token is a system command
     */
    static bool is_system_command(TokenType type) {
        return KeywordCategoryLookup::is_system_command(type);
    }
    
    /**
     * @brief Check if text matches system command pattern
     */
    static bool is_system_command_text(std::string_view text) {
        return KeywordTable::instance().is_system_command(text);
    }
    
    /**
     * @brief Get system command type from token
     */
    static SystemCommandType get_type(TokenType type) {
        return SystemCommandRegistry::instance().get_type_from_token(type);
    }
    
    /**
     * @brief Check if this is the start of a system command block
     */
    static bool is_command_start(TokenType type) {
        return type == TokenType::Kw_Origin;
    }
};

} // namespace lexer
} // namespace veldocra

