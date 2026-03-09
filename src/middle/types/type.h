/**
 * @file type.h
 * @brief VeldoCra Type System with Soul-based Types
 * @author Dr. Bright
 * 
 * Type system supporting:
 * - Primitive types (int, float, bool, etc.)
 * - Soul-based types (Custom Species)
 * - Type inference and checking
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <memory>
#include <variant>
#include <vector>
#include <optional>
#include <unordered_map>

namespace veldocra {
namespace types {

// Forward declarations
class Type;
class TypeEnvironment;

/**
 * @brief Type kind enumeration
 */
enum class TypeKind : uint8_t {
    // Primitive types
    Void = 0,
    Bool,
    Char,
    Int8, Int16, Int32, Int64,
    UInt8, UInt16, UInt32, UInt64,
    Float32, Float64,
    String,
    
    // Soul-based types (Custom Species)
    Soul,           // Base soul type
    Entity,         // Entity with soul
    Species,        // Custom species type
    SoulFragment,   // Fragment of soul
    Essence,        // Essence type
    
    // Compound types
    Array,
    Tuple,
    Struct,
    Enum,
    Function,
    Pointer,
    Reference,
    
    // Special
    Unknown,
    Never,
};

/**
 * @brief Soul manifest kinds (for soul-based types)
 */
enum class SoulManifestKind : uint8_t {
    Dormant = 0,    // Soul is dormant
    Awakened,       // Soul is awakened
    Manifested,     // Soul is manifested
    Transcended,    // Soul has transcended
};

/**
 * @brief Type modifier flags
 */
enum class TypeModifier : uint8_t {
    None = 0,
    Const = 1,
    Mutable = 2,
    Volatile = 4,
    Owned = 8,
    Borrowed = 16,
};

/**
 * @brief Base Type class
 */
class Type {
public:
    explicit Type(TypeKind kind);
    virtual ~Type();
    
    TypeKind get_kind() const { return kind_; }
    size_t get_size() const { return size_; }
    size_t get_alignment() const { return align_; }
    uint8_t get_modifiers() const { return modifiers_; }
    
    bool is_void() const { return kind_ == TypeKind::Void; }
    bool is_bool() const { return kind_ == TypeKind::Bool; }
    bool is_char() const { return kind_ == TypeKind::Char; }
    bool is_integer() const { return kind_ >= TypeKind::Int8 && kind_ <= TypeKind::UInt64; }
    bool is_signed_integer() const { return kind_ >= TypeKind::Int8 && kind_ <= TypeKind::Int64; }
    bool is_unsigned_integer() const { return kind_ >= TypeKind::UInt8 && kind_ <= TypeKind::UInt64; }
    bool is_float() const { return kind_ == TypeKind::Float32 || kind_ == TypeKind::Float64; }
    bool is_numeric() const { return is_integer() || is_float(); }
    bool is_string() const { return kind_ == TypeKind::String; }
    bool is_pointer() const { return kind_ == TypeKind::Pointer; }
    bool is_reference() const { return kind_ == TypeKind::Reference; }
    bool is_function() const { return kind_ == TypeKind::Function; }
    bool is_array() const { return kind_ == TypeKind::Array; }
    bool is_struct() const { return kind_ == TypeKind::Struct; }
    bool is_enum() const { return kind_ == TypeKind::Enum; }
    bool is_tuple() const { return kind_ == TypeKind::Tuple; }
    
    // Soul-based type checks
    bool is_soul_type() const { 
        return kind_ >= TypeKind::Soul && kind_ <= TypeKind::Essence; 
    }
    bool is_entity() const { return kind_ == TypeKind::Entity; }
    bool is_species() const { return kind_ == TypeKind::Species; }
    
    bool is_const() const { return (modifiers_ & static_cast<uint8_t>(TypeModifier::Const)) != 0; }
    bool is_mutable() const { return (modifiers_ & static_cast<uint8_t>(TypeModifier::Mutable)) != 0; }
    bool is_owned() const { return (modifiers_ & static_cast<uint8_t>(TypeModifier::Owned)) != 0; }
    bool is_borrowed() const { return (modifiers_ & static_cast<uint8_t>(TypeModifier::Borrowed)) != 0; }
    
    virtual std::string to_string() const;
    virtual bool equals(const Type* other) const;
    
    static Type* get_void();
    static Type* get_bool();
    static Type* get_char();
    static Type* get_int8();
    static Type* get_int16();
    static Type* get_int32();
    static Type* get_int64();
    static Type* get_uint8();
    static Type* get_uint16();
    static Type* get_uint32();
    static Type* get_uint64();
    static Type* get_float32();
    static Type* get_float64();
    static Type* get_string();
    static Type* get_soul();
    static Type* get_entity();
    static Type* get_never();
    static Type* get_unknown();
    
public:
    TypeKind kind_;
    size_t size_;
    size_t align_;
    uint8_t modifiers_;
};

/**
 * @brief Pointer type
 */
class PointerType : public Type {
public:
    PointerType(Type* element_type, bool is_mutable = true);
    
    Type* get_element_type() const { return element_type_; }
    bool is_mutable() const { return is_mutable_; }
    
    std::string to_string() const override;
    bool equals(const Type* other) const override;
    
private:
    Type* element_type_;
    bool is_mutable_;
};

/**
 * @brief Array type
 */
class ArrayType : public Type {
public:
    ArrayType(Type* element_type, size_t size);
    
    Type* get_element_type() const { return element_type_; }
    size_t get_size() const { return size_; }
    
    std::string to_string() const override;
    bool equals(const Type* other) const override;
    
private:
    Type* element_type_;
    size_t size_;
};

/**
 * @brief Function type
 */
class FunctionType : public Type {
public:
    FunctionType(Type* return_type, std::vector<Type*> param_types);
    
    Type* get_return_type() const { return return_type_; }
    const std::vector<Type*>& get_param_types() const { return param_types_; }
    
    std::string to_string() const override;
    bool equals(const Type* other) const override;
    
private:
    Type* return_type_;
    std::vector<Type*> param_types_;
};

/**
 * @brief Struct type
 */
class StructType : public Type {
public:
    StructType(std::string name);
    
    void add_field(std::string name, Type* field_type);
    const std::vector<std::pair<std::string, Type*>>& get_fields() const { return fields_; }
    Type* find_field(std::string_view name) const;
    
    std::string to_string() const override;
    bool equals(const Type* other) const override;
    
private:
    std::string name_;
    std::vector<std::pair<std::string, Type*>> fields_;
};

/**
 * @brief Soul species type (Custom Species)
 */
class SpeciesType : public Type {
public:
    SpeciesType(std::string species_name);
    
    void set_base_soul(Type* soul_type) { base_soul_ = soul_type; }
    Type* get_base_soul() const { return base_soul_; }
    
    void add_soul_property(std::string name, Type* prop_type);
    const std::vector<std::pair<std::string, Type*>>& get_soul_properties() const { return soul_properties_; }
    
    void set_manifest_kind(SoulManifestKind kind) { manifest_kind_ = kind; }
    SoulManifestKind get_manifest_kind() const { return manifest_kind_; }
    
    const std::string& get_species_name() const { return species_name_; }
    
    std::string to_string() const override;
    bool equals(const Type* other) const override;
    
private:
    std::string species_name_;
    Type* base_soul_;
    std::vector<std::pair<std::string, Type*>> soul_properties_;
    SoulManifestKind manifest_kind_;
};

/**
 * @brief Type environment (symbol table for types)
 */
class TypeEnvironment {
public:
    TypeEnvironment();
    ~TypeEnvironment();
    
    // Type definitions
    void define_type(std::string name, Type* type);
    Type* lookup_type(std::string_view name) const;
    bool is_type_defined(std::string_view name) const;
    
    // Variables with types
    void define_variable(std::string name, Type* type);
    Type* lookup_variable(std::string_view name) const;
    
    // Push/pop scope
    void push_scope();
    void pop_scope();
    
    // Built-in types
    void init_builtin_types();
    
private:
    struct Scope {
        std::unordered_map<std::string, Type*> types;
        std::unordered_map<std::string, Type*> variables;
    };
    
    std::vector<std::unique_ptr<Scope>> scopes_;
    std::unordered_map<std::string, Type*> builtin_types_;
};

/**
 * @brief Type inference result
 */
class InferResult {
public:
    InferResult() : type_(nullptr), error_() {}
    InferResult(Type* type) : type_(type), error_() {}
    InferResult(std::string error) : type_(nullptr), error_(std::move(error)) {}
    
    bool success() const { return type_ != nullptr && error_.empty(); }
    Type* get_type() const { return type_; }
    const std::string& get_error() const { return error_; }
    
private:
    Type* type_;
    std::string error_;
};

/**
 * @brief Type checker
 */
class TypeChecker {
public:
    explicit TypeChecker(TypeEnvironment* env);
    ~TypeChecker();
    
    // Type checking
    InferResult check_expression(void* expr);
    bool check_assignment(Type* target_type, Type* value_type);
    bool check_compatible(Type* expected, Type* actual);
    
    // Type inference
    InferResult infer_type(void* expr);
    
    // Unification
    InferResult unify(Type* t1, Type* t2);
    
private:
    TypeEnvironment* env_;
};

/**
 * @brief Helper to get common types
 */
namespace common_types {
    inline Type* bool_type() { return Type::get_bool(); }
    inline Type* int_type() { return Type::get_int32(); }
    inline Type* int64_type() { return Type::get_int64(); }
    inline Type* float_type() { return Type::get_float32(); }
    inline Type* double_type() { return Type::get_float64(); }
    inline Type* string_type() { return Type::get_string(); }
    inline Type* char_type() { return Type::get_char(); }
    inline Type* void_type() { return Type::get_void(); }
    inline Type* soul_type() { return Type::get_soul(); }
    inline Type* entity_type() { return Type::get_entity(); }
}

} // namespace types
} // namespace veldocra

