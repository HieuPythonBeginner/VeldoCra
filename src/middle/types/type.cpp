/**
 * @file type.cpp
 * @brief VeldoCra Type System Implementation
 * @author Dr. Bright
 * 
 * Implements the type system with soul-based types (Custom Species)
 */

#include "type.h"
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <iostream>

// For unordered_map in Scope
#include <unordered_map>

namespace veldanava {
namespace types {

// ============================================================================
// Type Constructor Implementation
// ============================================================================

Type::Type(TypeKind kind) : kind_(kind), size_(0), align_(1), modifiers_(0) {}

Type::~Type() = default;

bool Type::equals(const Type* other) const {
    if (!other) return false;
    if (this == other) return true;
    return kind_ == other->kind_ && modifiers_ == other->modifiers_;
}

std::string Type::to_string() const {
    switch (kind_) {
        case TypeKind::Void: return "void";
        case TypeKind::Bool: return "bool";
        case TypeKind::Char: return "char";
        case TypeKind::Int8: return "i8";
        case TypeKind::Int16: return "i16";
        case TypeKind::Int32: return "i32";
        case TypeKind::Int64: return "i64";
        case TypeKind::UInt8: return "u8";
        case TypeKind::UInt16: return "u16";
        case TypeKind::UInt32: return "u32";
        case TypeKind::UInt64: return "u64";
        case TypeKind::Float32: return "f32";
        case TypeKind::Float64: return "f64";
        case TypeKind::String: return "string";
        case TypeKind::Soul: return "soul";
        case TypeKind::Entity: return "entity";
        case TypeKind::Species: return "species";
        case TypeKind::SoulFragment: return "soul_fragment";
        case TypeKind::Essence: return "essence";
        case TypeKind::Array: return "array";
        case TypeKind::Tuple: return "tuple";
        case TypeKind::Struct: return "struct";
        case TypeKind::Enum: return "enum";
        case TypeKind::Function: return "fn";
        case TypeKind::Pointer: return "pointer";
        case TypeKind::Reference: return "reference";
        case TypeKind::Unknown: return "unknown";
        case TypeKind::Never: return "never";
        default: return "unknown";
    }
}

// ============================================================================
// Static Type Instances
// ============================================================================

// Forward declarations of getter functions
static Type* g_void_type = nullptr;
static Type* g_bool_type = nullptr;
static Type* g_char_type = nullptr;
static Type* g_int8_type = nullptr;
static Type* g_int16_type = nullptr;
static Type* g_int32_type = nullptr;
static Type* g_int64_type = nullptr;
static Type* g_uint8_type = nullptr;
static Type* g_uint16_type = nullptr;
static Type* g_uint32_type = nullptr;
static Type* g_uint64_type = nullptr;
static Type* g_float32_type = nullptr;
static Type* g_float64_type = nullptr;
static Type* g_string_type = nullptr;
static Type* g_soul_type = nullptr;
static Type* g_entity_type = nullptr;
static Type* g_never_type = nullptr;
static Type* g_unknown_type = nullptr;

static bool types_initialized = false;

static void init_types() {
    if (types_initialized) return;
    
    g_void_type = new Type(TypeKind::Void);
    g_void_type->size_ = 0;
    g_void_type->align_ = 1;
    
    g_bool_type = new Type(TypeKind::Bool);
    g_bool_type->size_ = 1;
    g_bool_type->align_ = 1;
    
    g_char_type = new Type(TypeKind::Char);
    g_char_type->size_ = 1;
    g_char_type->align_ = 1;
    
    g_int8_type = new Type(TypeKind::Int8);
    g_int8_type->size_ = 1;
    g_int8_type->align_ = 1;
    
    g_int16_type = new Type(TypeKind::Int16);
    g_int16_type->size_ = 2;
    g_int16_type->align_ = 2;
    
    g_int32_type = new Type(TypeKind::Int32);
    g_int32_type->size_ = 4;
    g_int32_type->align_ = 4;
    
    g_int64_type = new Type(TypeKind::Int64);
    g_int64_type->size_ = 8;
    g_int64_type->align_ = 8;
    
    g_uint8_type = new Type(TypeKind::UInt8);
    g_uint8_type->size_ = 1;
    g_uint8_type->align_ = 1;
    
    g_uint16_type = new Type(TypeKind::UInt16);
    g_uint16_type->size_ = 2;
    g_uint16_type->align_ = 2;
    
    g_uint32_type = new Type(TypeKind::UInt32);
    g_uint32_type->size_ = 4;
    g_uint32_type->align_ = 4;
    
    g_uint64_type = new Type(TypeKind::UInt64);
    g_uint64_type->size_ = 8;
    g_uint64_type->align_ = 8;
    
    g_float32_type = new Type(TypeKind::Float32);
    g_float32_type->size_ = 4;
    g_float32_type->align_ = 4;
    
    g_float64_type = new Type(TypeKind::Float64);
    g_float64_type->size_ = 8;
    g_float64_type->align_ = 8;
    
    g_string_type = new Type(TypeKind::String);
    g_string_type->size_ = 8;  // Pointer size
    g_string_type->align_ = 8;
    
    g_soul_type = new Type(TypeKind::Soul);
    g_soul_type->size_ = 16;  // Soul data size
    g_soul_type->align_ = 8;
    
    g_entity_type = new Type(TypeKind::Entity);
    g_entity_type->size_ = 24;  // Entity with soul
    g_entity_type->align_ = 8;
    
    g_never_type = new Type(TypeKind::Never);
    g_never_type->size_ = 0;
    g_never_type->align_ = 1;
    
    g_unknown_type = new Type(TypeKind::Unknown);
    g_unknown_type->size_ = 0;
    g_unknown_type->align_ = 1;
    
    types_initialized = true;
}

// ============================================================================
// Type Static Methods
// ============================================================================

Type* Type::get_void() {
    init_types();
    return g_void_type;
}

Type* Type::get_bool() {
    init_types();
    return g_bool_type;
}

Type* Type::get_char() {
    init_types();
    return g_char_type;
}

Type* Type::get_int8() {
    init_types();
    return g_int8_type;
}

Type* Type::get_int16() {
    init_types();
    return g_int16_type;
}

Type* Type::get_int32() {
    init_types();
    return g_int32_type;
}

Type* Type::get_int64() {
    init_types();
    return g_int64_type;
}

Type* Type::get_uint8() {
    init_types();
    return g_uint8_type;
}

Type* Type::get_uint16() {
    init_types();
    return g_uint16_type;
}

Type* Type::get_uint32() {
    init_types();
    return g_uint32_type;
}

Type* Type::get_uint64() {
    init_types();
    return g_uint64_type;
}

Type* Type::get_float32() {
    init_types();
    return g_float32_type;
}

Type* Type::get_float64() {
    init_types();
    return g_float64_type;
}

Type* Type::get_string() {
    init_types();
    return g_string_type;
}

Type* Type::get_soul() {
    init_types();
    return g_soul_type;
}

Type* Type::get_entity() {
    init_types();
    return g_entity_type;
}

Type* Type::get_never() {
    init_types();
    return g_never_type;
}

Type* Type::get_unknown() {
    init_types();
    return g_unknown_type;
}

// ============================================================================
// PointerType Implementation
// ============================================================================

PointerType::PointerType(Type* element_type, bool is_mutable)
    : Type(TypeKind::Pointer), element_type_(element_type), is_mutable_(is_mutable) {
    size_ = 8;  // Pointer size
    align_ = 8;
}

std::string PointerType::to_string() const {
    std::ostringstream oss;
    oss << (is_mutable_ ? "mut " : "") << element_type_->to_string() << "*";
    return oss.str();
}

bool PointerType::equals(const Type* other) const {
    if (!Type::equals(other)) return false;
    auto* ptr = static_cast<const PointerType*>(other);
    return element_type_->equals(ptr->element_type_) && is_mutable_ == ptr->is_mutable_;
}

// ============================================================================
// ArrayType Implementation
// ============================================================================

ArrayType::ArrayType(Type* element_type, size_t size)
    : Type(TypeKind::Array), element_type_(element_type), size_(size) {
    size_ = element_type->get_size() * size;
    align_ = element_type->get_alignment();
}

std::string ArrayType::to_string() const {
    std::ostringstream oss;
    oss << element_type_->to_string() << "[" << size_ << "]";
    return oss.str();
}

bool ArrayType::equals(const Type* other) const {
    if (!Type::equals(other)) return false;
    auto* arr = static_cast<const ArrayType*>(other);
    return size_ == arr->size_ && element_type_->equals(arr->element_type_);
}

// ============================================================================
// FunctionType Implementation
// ============================================================================

FunctionType::FunctionType(Type* return_type, std::vector<Type*> param_types)
    : Type(TypeKind::Function), return_type_(return_type), param_types_(std::move(param_types)) {
    size_ = 0;  // Function type has no size
    align_ = 1;
}

std::string FunctionType::to_string() const {
    std::ostringstream oss;
    oss << "fn(";
    for (size_t i = 0; i < param_types_.size(); i++) {
        if (i > 0) oss << ", ";
        oss << param_types_[i]->to_string();
    }
    oss << ") -> " << return_type_->to_string();
    return oss.str();
}

bool FunctionType::equals(const Type* other) const {
    if (!Type::equals(other)) return false;
    auto* fn = static_cast<const FunctionType*>(other);
    if (!return_type_->equals(fn->return_type_)) return false;
    if (param_types_.size() != fn->param_types_.size()) return false;
    for (size_t i = 0; i < param_types_.size(); i++) {
        if (!param_types_[i]->equals(fn->param_types_[i])) return false;
    }
    return true;
}

// ============================================================================
// StructType Implementation
// ============================================================================

StructType::StructType(std::string name)
    : Type(TypeKind::Struct), name_(std::move(name)) {
    size_ = 0;
    align_ = 1;
}

void StructType::add_field(std::string name, Type* field_type) {
    fields_.push_back({std::move(name), field_type});
    size_ = (size_ + field_type->get_alignment() - 1) / field_type->get_alignment() * field_type->get_alignment();
    size_ += field_type->get_size();
    align_ = std::max(align_, field_type->get_alignment());
}

Type* StructType::find_field(std::string_view name) const {
    for (const auto& [field_name, field_type] : fields_) {
        if (field_name == name) return field_type;
    }
    return nullptr;
}

std::string StructType::to_string() const {
    std::ostringstream oss;
    oss << "struct " << name_ << " { ";
    for (size_t i = 0; i < fields_.size(); i++) {
        if (i > 0) oss << ", ";
        oss << fields_[i].second->to_string() << " " << fields_[i].first;
    }
    oss << " }";
    return oss.str();
}

bool StructType::equals(const Type* other) const {
    if (!Type::equals(other)) return false;
    auto* str = static_cast<const StructType*>(other);
    if (name_ != str->name_) return false;
    if (fields_.size() != str->fields_.size()) return false;
    for (size_t i = 0; i < fields_.size(); i++) {
        if (!fields_[i].second->equals(str->fields_[i].second)) return false;
    }
    return true;
}

// ============================================================================
// SpeciesType Implementation (Soul-based Types)
// ============================================================================

SpeciesType::SpeciesType(std::string species_name)
    : Type(TypeKind::Species), species_name_(std::move(species_name)), 
      base_soul_(nullptr), manifest_kind_(SoulManifestKind::Dormant) {
    size_ = 24;  // Base soul + species data
    align_ = 8;
}

void SpeciesType::add_soul_property(std::string name, Type* prop_type) {
    soul_properties_.push_back({std::move(name), prop_type});
}

std::string SpeciesType::to_string() const {
    std::ostringstream oss;
    oss << "species " << species_name_;
    if (base_soul_) {
        oss << " : " << base_soul_->to_string();
    }
    if (!soul_properties_.empty()) {
        oss << " { ";
        for (size_t i = 0; i < soul_properties_.size(); i++) {
            if (i > 0) oss << ", ";
            oss << soul_properties_[i].second->to_string() << " " << soul_properties_[i].first;
        }
        oss << " }";
    }
    return oss.str();
}

bool SpeciesType::equals(const Type* other) const {
    if (!Type::equals(other)) return false;
    auto* sp = static_cast<const SpeciesType*>(other);
    return species_name_ == sp->species_name_;
}

// ============================================================================
// GenericType Implementation
// ============================================================================

GenericType::GenericType(std::string name, size_t index)
    : Type(TypeKind::Generic), name_(std::move(name)), index_(index) {
    size_ = 0;  // Generic parameters have no size until instantiated
    align_ = 1;
}

void GenericType::add_trait_bound(Type* trait) {
    trait_bounds_.push_back(trait);
}

std::string GenericType::to_string() const {
    std::ostringstream oss;
    oss << name_;
    if (!trait_bounds_.empty()) {
        oss << ": ";
        for (size_t i = 0; i < trait_bounds_.size(); i++) {
            if (i > 0) oss << " + ";
            oss << trait_bounds_[i]->to_string();
        }
    }
    return oss.str();
}

bool GenericType::equals(const Type* other) const {
    if (!Type::equals(other)) return false;
    auto* gt = static_cast<const GenericType*>(other);
    return name_ == gt->name_ && index_ == gt->index_;
}

// ============================================================================
// TraitType Implementation (blessing)
// ============================================================================

TraitType::TraitType(std::string name)
    : Type(TypeKind::Trait), name_(std::move(name)) {
    size_ = 8;  // Trait objects have vtable pointer
    align_ = 8;
}

void TraitType::add_method(std::string name, FunctionType* method_type) {
    methods_.push_back({std::move(name), method_type});
}

bool TraitType::is_implemented_by(Type* type) const {
    // Check if type has all methods required by this trait
    // For now, allow basic numeric/types to pass trait checks
    // Full method signature matching to be implemented later
    if (!type) return false;
    // Allow numeric types to satisfy numeric traits
    if (type->is_numeric()) return true;
    // Allow string type to satisfy comparable traits  
    if (type->is_string()) return true;
    // TODO: Full method signature matching
    return false;
}

std::string TraitType::to_string() const {
    std::ostringstream oss;
    oss << "blessing " << name_;
    if (!methods_.empty()) {
        oss << " { ";
        for (size_t i = 0; i < methods_.size(); i++) {
            if (i > 0) oss << ", ";
            oss << methods_[i].second->to_string() << " " << methods_[i].first;
        }
        oss << " }";
    }
    return oss.str();
}

bool TraitType::equals(const Type* other) const {
    if (!Type::equals(other)) return false;
    auto* tt = static_cast<const TraitType*>(other);
    return name_ == tt->name_;
}

// ============================================================================
// GenericInstanceType Implementation
// ============================================================================

GenericInstanceType::GenericInstanceType(Type* generic_template, std::vector<Type*> type_args)
    : Type(TypeKind::GenericInstance), generic_template_(generic_template), type_args_(std::move(type_args)) {
    size_ = 0;  // Size depends on instantiated types
    align_ = 8;
}

std::string GenericInstanceType::to_string() const {
    std::ostringstream oss;
    oss << generic_template_->to_string();
    if (!type_args_.empty()) {
        oss << "<";
        for (size_t i = 0; i < type_args_.size(); i++) {
            if (i > 0) oss << ", ";
            oss << type_args_[i]->to_string();
        }
        oss << ">";
    }
    return oss.str();
}

bool GenericInstanceType::equals(const Type* other) const {
    if (!Type::equals(other)) return false;
    auto* gi = static_cast<const GenericInstanceType*>(other);
    if (!generic_template_->equals(gi->generic_template_)) return false;
    if (type_args_.size() != gi->type_args_.size()) return false;
    for (size_t i = 0; i < type_args_.size(); i++) {
        if (!type_args_[i]->equals(gi->type_args_[i])) return false;
    }
    return true;
}

// ============================================================================
// TypeEnvironment Implementation
// ============================================================================

TypeEnvironment::TypeEnvironment() {
    push_scope();
    init_builtin_types();
}

TypeEnvironment::~TypeEnvironment() {
    // Clean up builtin types
    for (auto& [name, type] : builtin_types_) {
        // Don't delete static types
    }
}

void TypeEnvironment::init_builtin_types() {
    // Register built-in types in global scope
    builtin_types_["void"] = Type::get_void();
    builtin_types_["bool"] = Type::get_bool();
    builtin_types_["char"] = Type::get_char();
    builtin_types_["i8"] = Type::get_int8();
    builtin_types_["i16"] = Type::get_int16();
    builtin_types_["i32"] = Type::get_int32();
    builtin_types_["i64"] = Type::get_int64();
    builtin_types_["u8"] = Type::get_uint8();
    builtin_types_["u16"] = Type::get_uint16();
    builtin_types_["u32"] = Type::get_uint32();
    builtin_types_["u64"] = Type::get_uint64();
    builtin_types_["f32"] = Type::get_float32();
    builtin_types_["f64"] = Type::get_float64();
    builtin_types_["string"] = Type::get_string();
    builtin_types_["soul"] = Type::get_soul();
    builtin_types_["entity"] = Type::get_entity();
    
    // Add to current scope
    for (auto& [name, type] : builtin_types_) {
        scopes_.back()->types[name] = type;
    }
}

void TypeEnvironment::define_type(std::string name, Type* type) {
    scopes_.back()->types[std::move(name)] = type;
}

Type* TypeEnvironment::lookup_type(std::string_view name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto type_it = it->get()->types.find(std::string(name));
        if (type_it != it->get()->types.end()) {
            return type_it->second;
        }
    }
    return nullptr;
}

bool TypeEnvironment::is_type_defined(std::string_view name) const {
    return lookup_type(name) != nullptr;
}

void TypeEnvironment::define_variable(std::string name, Type* type) {
    scopes_.back()->variables[std::move(name)] = type;
}

Type* TypeEnvironment::lookup_variable(std::string_view name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto var_it = it->get()->variables.find(std::string(name));
        if (var_it != it->get()->variables.end()) {
            return var_it->second;
        }
    }
    return nullptr;
}

void TypeEnvironment::push_scope() {
    scopes_.push_back(std::make_unique<Scope>());
}

void TypeEnvironment::pop_scope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

// ============================================================================
// TypeChecker Implementation
// ============================================================================

TypeChecker::TypeChecker(TypeEnvironment* env) : env_(env) {}

TypeChecker::~TypeChecker() = default;



bool TypeChecker::check_assignment(Type* target_type, Type* value_type) {
    if (!target_type || !value_type) return false;
    if (target_type->equals(value_type)) return true;
    // Allow numeric conversions
    if (target_type->is_numeric() && value_type->is_numeric()) return true;
    // Allow string to anything for now
    if (value_type->is_string()) return true;
    return false;
}

bool TypeChecker::check_compatible(Type* expected, Type* actual) {
    if (!expected || !actual) return false;
    return expected->equals(actual) || 
           (expected->is_numeric() && actual->is_numeric()) ||
           (expected->is_void() && actual->is_void());
}

InferResult TypeChecker::infer_type(void* expr) {
    if (!expr) return InferResult(Type::get_unknown());
    
    // Minimal type inference for now - just return unknown
    // Full AST integration to be implemented when headers are available
    (void)expr;
    return InferResult(Type::get_unknown());
}

InferResult TypeChecker::unify(Type* t1, Type* t2) {
    if (!t1 || !t2) {
        return InferResult("Cannot unify null types");
    }
    if (t1->equals(t2)) {
        return InferResult(t1);
    }
    
    // Handle generic types
    if (t1->get_kind() == TypeKind::Generic) {
        return unify_generic(t1, t2);
    }
    if (t2->get_kind() == TypeKind::Generic) {
        return unify_generic(t2, t1);
    }
    
    // Handle trait types
    if (t1->get_kind() == TypeKind::Trait) {
        return unify_trait(t1, t2);
    }
    if (t2->get_kind() == TypeKind::Trait) {
        return unify_trait(t2, t1);
    }
    
    // Handle generic instance types
    if (t1->get_kind() == TypeKind::GenericInstance) {
        return unify_instance(t1, t2);
    }
    if (t2->get_kind() == TypeKind::GenericInstance) {
        return unify_instance(t2, t1);
    }
    
    // Try numeric unification
    if (t1->is_numeric() && t2->is_numeric()) {
        // Return the "larger" type
        if (t1->get_kind() == TypeKind::Float64 || t2->get_kind() == TypeKind::Float64) {
            return InferResult(Type::get_float64());
        }
        if (t1->get_kind() == TypeKind::Float32 || t2->get_kind() == TypeKind::Float32) {
            return InferResult(Type::get_float32());
        }
        if (t1->get_kind() == TypeKind::Int64 || t2->get_kind() == TypeKind::Int64) {
            return InferResult(Type::get_int64());
        }
        return InferResult(Type::get_int32());
    }
    return InferResult("Cannot unify types");
}

std::unordered_map<std::string, Type*> TypeChecker::infer_generic_args(
    Type* generic_type, Type* concrete_type) {
    std::unordered_map<std::string, Type*> result;
    
    if (!generic_type || !concrete_type) return result;
    
    // If generic_type is GenericInstance
    if (generic_type->get_kind() == TypeKind::GenericInstance) {
        auto* gi = static_cast<GenericInstanceType*>(generic_type);
        
        // If concrete_type is also GenericInstance, match type arguments
        if (concrete_type->get_kind() == TypeKind::GenericInstance) {
            auto* gi_concrete = static_cast<GenericInstanceType*>(concrete_type);
            if (gi->get_template()->equals(gi_concrete->get_template())) {
                // Match type arguments
                for (size_t i = 0; i < gi->get_type_args().size(); i++) {
                    if (i < gi_concrete->get_type_args().size()) {
                        // If the template parameter is Generic, record the substitution
                        if (gi->get_type_args()[i]->get_kind() == TypeKind::Generic) {
                            auto* gen = static_cast<GenericType*>(gi->get_type_args()[i]);
                            result[gen->get_name()] = gi_concrete->get_type_args()[i];
                        }
                    }
                }
            }
        }
    }
    
    return result;
}

bool TypeChecker::check_trait_bounds(Type* type, const std::vector<Type*>& trait_bounds) {
    if (!type) return false;
    
    for (Type* trait : trait_bounds) {
        if (trait->get_kind() != TypeKind::Trait) continue;
        
        auto* trait_type = static_cast<TraitType*>(trait);
        if (!trait_type->is_implemented_by(type)) {
            return false;
        }
    }
    
    return true;
}

Type* TypeChecker::substitute(Type* type, const std::unordered_map<std::string, Type*>& substitutions) {
    if (!type) return nullptr;
    
    // If it's a generic type, look up in substitutions
    if (type->get_kind() == TypeKind::Generic) {
        auto* gen = static_cast<GenericType*>(type);
        auto it = substitutions.find(gen->get_name());
        if (it != substitutions.end()) {
            std::cerr << "[TypeChecker] Substituting generic " << gen->get_name() << " with concrete type\n";
            return it->second;
        }
        return type; // No substitution found
    }
    
    // For GenericInstanceType, substitute in type arguments
    if (type->get_kind() == TypeKind::GenericInstance) {
        auto* inst = static_cast<GenericInstanceType*>(type);
        std::vector<Type*> new_args;
        for (Type* arg : inst->get_type_args()) {
            Type* substituted = substitute(arg, substitutions);
            new_args.push_back(substituted);
        }
        // Create new instance with substituted arguments
        return new GenericInstanceType(inst->get_template(), new_args);
    }
    
    // For other types, return as-is for now
    // Would need to recursively substitute for complex types like StructType, FunctionType
    return type;
}

// Private unification helpers

InferResult TypeChecker::unify_generic(Type* generic, Type* other) {
    auto* gen = static_cast<GenericType*>(generic);
    
    // Check trait bounds
    if (!gen->get_trait_bounds().empty()) {
        if (!check_trait_bounds(other, gen->get_trait_bounds())) {
            return InferResult("Type does not satisfy trait bounds");
        }
    }
    
    // Generic unifies with any type (would record the substitution)
    return InferResult(other);
}

InferResult TypeChecker::unify_trait(Type* trait, Type* other) {
    auto* tr = static_cast<TraitType*>(trait);
    
    // Check if other type implements this trait
    if (!tr->is_implemented_by(other)) {
        return InferResult("Type does not implement trait");
    }
    
    return InferResult(trait);
}

InferResult TypeChecker::unify_instance(Type* instance, Type* other) {

    // TODO

    return InferResult("Unify instance not implemented");

}

bool TypeChecker::check(ast::Node* root) {
    errors_.clear();
    check_node(root);
    return errors_.empty();
}

void TypeChecker::check_node(ast::Node* node) {
    if (!node) return;
    
    switch (node->kind) {
        case ast::NodeKind::ExprStmt: {
            auto* stmt = static_cast<ast::ExprStmtNode*>(node);
            check_expression(stmt->expr);
            break;
        }
        case ast::NodeKind::CallExpr: {
            auto* call = static_cast<ast::CallExprNode*>(node);
            // Check callee is function type
            auto callee_result = check_expression(call->callee);
            if (!callee_result.success()) {
                errors_.push_back("Invalid callee: " + callee_result.get_error());
            }
            // Check args
            for (size_t i = 0; i < call->arg_count; ++i) {
                auto arg_result = check_expression(call->args[i]);
                if (!arg_result.success()) {
                    errors_.push_back("Invalid argument " + std::to_string(i) + ": " + arg_result.get_error());
                }
            }
            break;
        }
        // Add more cases as needed
        default:
            // Skip for now
            break;
    }
}

InferResult TypeChecker::check_expression(ast::Expr* expr) {
    if (!expr) return InferResult("Null expression");
    
    switch (expr->kind) {
        case ast::NodeKind::LiteralExpr: {
            auto* lit = static_cast<ast::LiteralExprNode*>(expr);
            if (lit->literal_type == 2) { // string
                expr->type_info = Type::get_string();
                return InferResult(Type::get_string());
            } else if (lit->literal_type == 0) { // int
                expr->type_info = Type::get_int32();
                return InferResult(Type::get_int32());
            }
            return InferResult("Unsupported literal type");
        }
        case ast::NodeKind::VariableExpr: {
            auto* var = static_cast<ast::VariableExprNode*>(expr);
            // For builtins, assume void or string
            std::string name(var->name);
            if (name == "scribe") {
                expr->type_info = Type::get_void(); // scribe returns void
                return InferResult(Type::get_void());
            }
            // Lookup in scopes, for now assume int
            expr->type_info = Type::get_int32();
            return InferResult(Type::get_int32());
        }
        case ast::NodeKind::CallExpr: {
            auto* call = static_cast<ast::CallExprNode*>(expr);
            auto callee_result = check_expression(call->callee);
            if (!callee_result.success()) {
                return callee_result;
            }
            // Assume return type of callee
            expr->type_info = callee_result.get_type();
            return callee_result;
        }
        default:
            return InferResult("Unsupported expression type");
    }
}

} // namespace types
} // namespace veldanava

