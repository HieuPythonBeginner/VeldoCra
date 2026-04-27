/**
 * @file vm.cpp
 * @brief VeldoCra Register-Based Virtual Machine Implementation
 * @author Dr. Bright
 * 
 * Register-based VM with switch dispatch.
 */

#include "vm.h"
#include <iostream>
#include <cassert>

namespace veldanava {
namespace vm {

VM::VM(size_t register_count)
    : registers_(register_count)
    , program_(nullptr)
    , pc_(0)
    , running_(false)
    , debug_(false)
    , zero_flag_(false)
    , sign_flag_(false)
    , overflow_flag_(false)
    , loop_iterations_(0)
{
    register_stdlib_functions();
}

VM::~VM() {
    // Free all allocated memory blocks
    for (void* ptr : allocated_blocks_) {
        std::free(ptr);
    }
    allocated_blocks_.clear();
}

VM::VM(VM&&) noexcept = default;
VM& VM::operator=(VM&&) noexcept = default;

void VM::register_stdlib_functions() {
    // Register stdlib functions in function_table_
    // Builtin IDs >= 100 are handled by execute_builtin_function
    function_table_["print"] = 0;  // Dummy address (legacy)
    function_table_["read_line"] = 1;
    function_table_["concat"] = 2;
    function_table_["abs"] = 3;
    function_table_["scribe"] = 107;  // Print builtin (>=100 for direct dispatch)

    // Memory management functions (builtin)
    function_table_["forge"] = 100;   // alloc
    function_table_["clm"] = 101;     // malloc
    function_table_["pur"] = 102;     // calloc
    function_table_["rsz"] = 103;     // realloc
    function_table_["ee"] = 104;      // free
    function_table_["touch"] = 105;   // deref
    function_table_["ee_inf_layers"] = 106; // delete (destructor + free)
}

void VM::execute_builtin_function(uint32_t func_id) {
    switch (func_id) {
        case 100: { // forge(size) - alloc
            size_t size = registers_[1].to_int();
            void* ptr = std::malloc(size);
            if (ptr) allocated_blocks_.push_back(ptr);
            registers_[0] = Value::from_pointer(ptr);
            break;
        }
        case 101: { // clm(size) - malloc
            size_t size = registers_[1].to_int();
            void* ptr = std::malloc(size);
            if (ptr) allocated_blocks_.push_back(ptr);
            registers_[0] = Value::from_pointer(ptr);
            break;
        }
        case 102: { // pur(num, size) - calloc
            size_t num = registers_[1].to_int();
            size_t size = registers_[2].to_int();
            void* ptr = std::calloc(num, size);
            if (ptr) allocated_blocks_.push_back(ptr);
            registers_[0] = Value::from_pointer(ptr);
            break;
        }
        case 103: { // rsz(ptr, size) - realloc
            void* ptr = registers_[1].to_pointer();
            size_t size = registers_[2].to_int();
            void* new_ptr = std::realloc(ptr, size);
            if (new_ptr) {
                // Update allocated_blocks_ if pointer changed
                auto it = std::find(allocated_blocks_.begin(), allocated_blocks_.end(), ptr);
                if (it != allocated_blocks_.end()) {
                    *it = new_ptr;
                }
                registers_[0] = Value::from_pointer(new_ptr);
            } else {
                registers_[0] = Value::from_pointer(nullptr);
            }
            break;
        }
        case 104: { // ee(ptr) - free
            void* ptr = registers_[1].to_pointer();
            auto it = std::find(allocated_blocks_.begin(), allocated_blocks_.end(), ptr);
            if (it != allocated_blocks_.end()) {
                std::free(ptr);
                allocated_blocks_.erase(it);
            }
            break;
        }
        case 105: { // touch(ptr) - deref
            void* ptr = registers_[1].to_pointer();
            // For now, just return the pointer itself
            // TODO: Implement proper dereferencing when we have pointer types
            registers_[0] = Value::from_pointer(ptr);
            break;
        }
        case 106: { // ee_inf_layers(ptr) - delete (destructor + free)
            void* ptr = registers_[1].to_pointer();
            // TODO: Call destructor if object has one
            // For now, just free the memory
            auto it = std::find(allocated_blocks_.begin(), allocated_blocks_.end(), ptr);
            if (it != allocated_blocks_.end()) {
                std::free(ptr);
                allocated_blocks_.erase(it);
            }
            break;
        }
        case 0: { // scribe(str_idx) - print string constant index
            // Argument: string index in register 1 (R1)
            size_t str_idx = static_cast<size_t>(registers_[1].int_val);
            if (program_ && str_idx < program_->string_count()) {
                std::cout << program_->get_string(str_idx) << std::flush;
            } else {
                error_ = "Invalid string index in scribe";
                running_ = false;
            }
            break;
        }
        default:
            error_ = "Unknown builtin function: " + std::to_string(func_id);
            running_ = false;
            break;
    }
}

void VM::load(Program* program) {
    program_ = program;
    pc_ = 0;
    running_ = true;
    
    for (auto& reg : registers_) {
        reg = Value{};
    }
}

void VM::reset() {
    pc_ = 0;
    running_ = false;
    error_.clear();
    loop_iterations_ = 0;

    while (!stack_.empty()) stack_.pop();
    while (!call_stack_.empty()) call_stack_.pop();

    for (auto& reg : registers_) {
        reg = Value{};
    }

    // Free all allocated memory blocks
    for (void* ptr : allocated_blocks_) {
        std::free(ptr);
    }
    allocated_blocks_.clear();
}

int VM::execute() {
    if (!program_) {
        error_ = "No program loaded";
        return -1;
    }
    
    load(program_);
    
    if (debug_) {
        std::cerr << "[VM] Starting execution, program size: " << program_->size() << "\n";
    }
    
    // Main execution loop with switch dispatch
    size_t inst_index = 0;
    while (running_ && inst_index < program_->size()) {
        loop_iterations_++;
        if (loop_iterations_ > 100000) {
            error_ = "Loop limit reached";
            running_ = false;
            break;
        }

        const Instruction& inst = program_->get(inst_index);
        pc_ = inst_index;  // Keep pc_ in sync with inst_index for jump calculations

        std::cerr << "[VM] PC=" << inst_index << " OP=" << opcode_to_string(inst.op) << " (" << (int)inst.op << ")\n";
        
        switch (inst.op) {
            case Opcode::MOV: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                registers_[rd] = registers_[ra];
                inst_index++;
                break;
            }
                
            case Opcode::MOVI: {
                uint8_t rd = inst.rd();
                int32_t imm = inst.imm32();
                if (debug_) std::cerr << "[VM-DEBUG] MOVI rd=" << (int)rd << " imm=" << imm << "\n";
                registers_[rd] = Value::from_int(imm);
                inst_index++;
                break;
            }
                
            case Opcode::ADD: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val + registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::SUB: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val - registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::MUL: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val * registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::DIV: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                if (registers_[rb].int_val == 0) {
                    error_ = "Division by zero";
                    running_ = false;
                    break;
                }
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val / registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::MOD: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                if (registers_[rb].int_val == 0) {
                    error_ = "Modulo by zero";
                    running_ = false;
                    break;
                }
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val % registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::NEG: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = -registers_[ra].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::AND: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val & registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::OR: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val | registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::XOR: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val ^ registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::NOT: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = ~registers_[ra].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::SHL: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val << registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::SHR: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val >> registers_[rb].int_val;
                inst_index++;
                break;
            }
                
            case Opcode::CMP: {
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                int32_t a = registers_[ra].int_val;
                int32_t b = registers_[rb].int_val;
                int32_t diff = a - b;
                zero_flag_ = (diff == 0);
                sign_flag_ = (diff < 0);
                int64_t temp = (int64_t)a - (int64_t)b;
                overflow_flag_ = (temp > INT32_MAX || temp < INT32_MIN);
                if (debug_) std::cerr << "[VM-DEBUG] CMP ra=" << (int)ra << " rb=" << (int)rb << " a=" << a << " b=" << b << " diff=" << diff << " zero=" << zero_flag_ << " sign=" << sign_flag_ << " overflow=" << overflow_flag_ << std::endl;
                inst_index++;
                break;
            }
                
            case Opcode::JMP: {
                pc_ = inst.target();
                inst_index = pc_;
                break;
            }
                
            case Opcode::JZ: {
                uint8_t rd = inst.rd();
                uint32_t target = inst.target();
                if (debug_) std::cerr << "[VM-DEBUG] JZ rd=" << (int)rd << " val=" << registers_[rd].int_val << " target=" << target << std::endl;
                if (registers_[rd].int_val == 0) {
                    pc_ = target;
                    inst_index = pc_;
                } else {
                    inst_index++;
                }
                break;
            }

            case Opcode::JNZ: {
                uint32_t target = inst.target();
                pc_ = !zero_flag_ ? target : pc_ + 1;
                inst_index = pc_;
                break;
            }

            case Opcode::JE: {
                uint32_t target = inst.target();
                if (debug_) std::cerr << "[VM-DEBUG] JE: zero=" << zero_flag_ << " target=" << target << " next=" << pc_+inst.size() << "\n";
                pc_ = zero_flag_ ? target : pc_ + 1;
                inst_index = pc_;
                break;
            }

            case Opcode::JNE: {
                uint32_t target = inst.target();
                pc_ = !zero_flag_ ? target : pc_ + 1;
                inst_index = pc_;
                break;
            }

            case Opcode::JG: {
                uint32_t target = inst.target();
                bool jump = !zero_flag_ && (sign_flag_ == overflow_flag_);
                if (debug_) std::cerr << "[VM-DEBUG] JG zero=" << zero_flag_ << " sign=" << sign_flag_ << " overflow=" << overflow_flag_ << " jump=" << (jump ? "yes" : "no") << " target=" << target << std::endl;
                if (jump) {
                    pc_ = target;
                    inst_index = pc_;
                } else {
                    inst_index++;
                }
                break;
            }

            case Opcode::JGE: {
                uint32_t target = inst.target();
                pc_ = !sign_flag_ ? target : pc_ + 1;
                inst_index = pc_;
                break;
            }

            case Opcode::JL: {
                uint32_t target = inst.target();
                bool cond = sign_flag_ && !zero_flag_;
                pc_ = cond ? target : pc_ + 1;
                inst_index = pc_;
                break;
            }

            case Opcode::JLE: {
                uint32_t target = inst.target();
                bool cond = zero_flag_ || sign_flag_;
                pc_ = cond ? target : pc_ + 1;
                inst_index = pc_;
                break;
            }
                
            case Opcode::CALL: {
                uint32_t target = inst.target();
                std::cerr << "[VM] CALL target=" << target << "\n";
                // Check if it's a builtin function call (target < 100)
                if (target < 100) {
                    // Builtin function dispatch
                    std::cerr << "[VM] Executing builtin function " << target << "\n";
                    execute_builtin_function(target);
                    inst_index++;
                    break;
                }
                call_stack_.push(pc_ + 1);
                pc_ = target;
                inst_index = pc_;
                break;
            }
                
            case Opcode::RET: {
                if (call_stack_.empty()) {
                    running_ = false;
                    break;
                }
                pc_ = call_stack_.top();
                call_stack_.pop();
                inst_index = pc_;
                break;
            }

            case Opcode::PRINT: {
                uint8_t rd = inst.rd();
                Value& val = registers_[rd];
                switch (val.type) {
                    case ValueType::Int:
                        std::cout << val.int_val << '\n';
                        break;
                    case ValueType::Float:
                        std::cout << val.float_val << '\n';
                        break;
                    case ValueType::Bool:
                        std::cout << (val.bool_val ? "true" : "false") << '\n';
                        break;
                    case ValueType::Char:
                        std::cout << val.char_val << '\n';
                        break;
                    case ValueType::String:
                        std::cout << (val.string_val ? val.string_val : "(null)") << '\n';
                        break;
                    default:
                        std::cout << "unknown type" << '\n';
                        break;
                }
                inst_index++;
                break;
            }
                
            case Opcode::PRINTS: {
                uint8_t rd = inst.rd();
                size_t str_idx = registers_[rd].int_val;
                int32_t imm = inst.imm32();
                if (imm >= 0) str_idx = static_cast<size_t>(imm);
                if (debug_) std::cerr << "[VM-DEBUG] PRINTS rd=" << (int)rd << " str_idx=" << str_idx << " imm=" << imm << "\n";
                if (debug_) std::cerr << "[VM-DEBUG] PRINTS rd=" << (int)rd << " str_idx=" << str_idx << "\n";
                if (program_ && str_idx < program_->string_count()) {
                    std::cout << program_->get_string(str_idx) << std::endl << std::flush;
                } else {
                    error_ = "Invalid string index in PRINTS";
                    running_ = false;
                    break;
                }
                inst_index++;
                break;
            }

            case Opcode::CONCAT: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                std::string a = program_->get_string(registers_[ra].int_val);
                std::string b = program_->get_string(registers_[rb].int_val);
                std::string result = a + b;
                size_t str_idx = program_->add_string(result);
                registers_[rd] = Value::from_int(str_idx);
                inst_index++;
                break;
            }

            case Opcode::TO_STR: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                std::string str = std::to_string(registers_[ra].int_val);
                size_t str_idx = program_->add_string(str);
                registers_[rd] = Value::from_int(str_idx);
                inst_index++;
                break;
            }

            case Opcode::READ: {
                uint8_t rd = inst.rd();
                int64_t val;
                std::cin >> val;
                registers_[rd] = Value::from_int(val);
                inst_index++;
                break;
            }
                
            case Opcode::PUSH: {
                uint8_t rd = inst.rd();
                stack_.push(registers_[rd]);
                inst_index++;
                break;
            }
                
            case Opcode::POP: {
                uint8_t rd = inst.rd();
                if (stack_.empty()) {
                    error_ = "Stack underflow";
                    running_ = false;
                    break;
                }
                registers_[rd] = stack_.top();
                stack_.pop();
                inst_index++;
                break;
            }
                
            case Opcode::PEEK: {
                uint8_t rd = inst.rd();
                if (stack_.empty()) {
                    error_ = "Stack underflow";
                    running_ = false;
                    break;
                }
                registers_[rd] = stack_.top();
                inst_index++;
                break;
            }
                
            case Opcode::ALLOC: {
                // Allocate memory: rd = alloc(size)
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                
                size_t size = static_cast<size_t>(registers_[ra].int_val);
                if (size == 0) {
                    error_ = "Cannot allocate zero bytes";
                    running_ = false;
                    break;
                }
                
                // Allocate memory and track it
                void* ptr = std::malloc(size);
                if (!ptr) {
                    error_ = "Memory allocation failed";
                    running_ = false;
                    break;
                }
                
                std::memset(ptr, 0, size);
                allocated_blocks_.push_back(ptr);
                registers_[rd] = Value::from_int(reinterpret_cast<int64_t>(ptr));
                
                if (debug_) {
                    std::cerr << "[VM] ALLOC: r" << (int)rd << " = malloc(" << size << ") = " << ptr << "\n";
                }
                
                inst_index++;
                break;
            }
            
            case Opcode::STORE: {
                // Store: mem[rd + offset] = ra
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                
                void* base_ptr = reinterpret_cast<void*>(registers_[rd].int_val);
                size_t offset = static_cast<size_t>(registers_[ra].int_val);
                int64_t value = registers_[rb].int_val;
                
                if (!base_ptr) {
                    error_ = "Null pointer dereference in STORE";
                    running_ = false;
                    break;
                }
                
                void* target = static_cast<char*>(base_ptr) + offset;
                *static_cast<int64_t*>(target) = value;
                
                if (debug_) {
                    std::cerr << "[VM] STORE: mem[r" << (int)rd << " + r" << (int)ra << "] = r" << (int)rb << " (" << value << ")\n";
                }
                
                inst_index++;
                break;
            }
            
            case Opcode::LOAD: {
                // Load: rd = mem[ra + offset]
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                
                void* base_ptr = reinterpret_cast<void*>(registers_[ra].int_val);
                size_t offset = static_cast<size_t>(registers_[rb].int_val);
                
                if (!base_ptr) {
                    error_ = "Null pointer dereference in LOAD";
                    running_ = false;
                    break;
                }
                
                void* source = static_cast<char*>(base_ptr) + offset;
                int64_t value = *static_cast<int64_t*>(source);
                registers_[rd] = Value::from_int(value);
                
                if (debug_) {
                    std::cerr << "[VM] LOAD: r" << (int)rd << " = mem[r" << (int)ra << " + r" << (int)rb << "] (" << value << ")\n";
                }
                
                inst_index++;
                break;
            }
            
            case Opcode::STRIDX: {
                // String index: rd = string[ra]
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                
                // Get string index from register ra
                size_t str_idx = registers_[ra].int_val;
                
                // Get character index from register rb
                uint8_t rb = inst.rb();
                size_t char_idx = registers_[rb].int_val;
                
                // Get string from program
                if (program_ && str_idx < program_->string_count()) {
                    const std::string& str = program_->get_string(str_idx);
                    
                    // Bounds check
                    if (char_idx < str.size()) {
                        // Return character as integer
                        registers_[rd] = Value::from_int(static_cast<int64_t>(str[char_idx]));
                    } else {
                        error_ = "String index out of bounds";
                        running_ = false;
                        break;
                    }
                } else {
                    error_ = "Invalid string index";
                    running_ = false;
                    break;
                }

                inst_index++;
                break;
            }

            case Opcode::NEW_CLASS: {
                // Create new class: rd = new_class(name_index)
                uint8_t rd = inst.rd();
                uint8_t name_idx = inst.ra();

                if (program_ && name_idx < program_->string_count()) {
                    const std::string& class_name = program_->get_string(name_idx);
                    auto* new_class = new VMClass(class_name.c_str());
                    classes_[class_name] = new_class;
                    registers_[rd] = Value::from_class(new_class);
                }
                inst_index++;
                break;
            }

            case Opcode::NEW_OBJECT: {
                // Create new object: rd = new_object(class_reg)
                uint8_t rd = inst.rd();
                uint8_t class_reg = inst.ra();

                const Value& class_val = registers_[class_reg];
                if (class_val.type == ValueType::Class && class_val.class_val) {
                    auto* new_object = new VMObject(class_val.class_val);
                    objects_.emplace_back(new_object);
                    registers_[rd] = Value::from_object(new_object);
                }
                inst_index++;
                break;
            }

            case Opcode::GET_FIELD: {
                // Get object field: rd = object[ra].field_index
                uint8_t rd = inst.rd();
                uint8_t obj_reg = inst.ra();
                uint8_t field_idx = inst.rb();

                const Value& obj_val = registers_[obj_reg];
                if (obj_val.type == ValueType::Object && obj_val.object_val) {
                    registers_[rd] = obj_val.object_val->get_field(field_idx);
                }
                inst_index++;
                break;
            }

            case Opcode::SET_FIELD: {
                // Set object field: object[rd].field_index = ra
                uint8_t obj_reg = inst.rd();
                uint8_t field_idx = inst.ra();
                uint8_t value_reg = inst.rb();

                const Value& obj_val = registers_[obj_reg];
                if (obj_val.type == ValueType::Object && obj_val.object_val) {
                    obj_val.object_val->set_field(field_idx, registers_[value_reg]);
                }
                inst_index++;
                break;
            }

            case Opcode::CALL_METHOD: {
                // Call virtual method: rd = object.call_method(name_index, arg_regs...)
                uint8_t rd = inst.rd();
                uint8_t obj_reg = inst.ra();
                uint8_t method_name_idx = inst.rb();

                const Value& obj_val = registers_[obj_reg];
                if (obj_val.type == ValueType::Object && obj_val.object_val &&
                    program_ && method_name_idx < program_->string_count()) {

                    const std::string& method_name = program_->get_string(method_name_idx);
                    const VirtualMethod* method = obj_val.object_val->class_def->find_method(method_name.c_str());

                    if (method) {
                        // For now, just call the function directly
                        // In a full implementation, this would handle argument passing
                        call_stack_.push(pc_);
                        pc_ = method->function_index;
                        inst_index = pc_;
                        registers_[rd] = Value::from_int(0); // Placeholder return value
                    }
                }
                inst_index++;
                break;
            }

            case Opcode::MOVSTR: {
                uint8_t rd = inst.rd();
                int32_t str_idx = inst.imm32();
                if (program_ && str_idx >= 0 && static_cast<size_t>(str_idx) < program_->string_count()) {
                    registers_[rd].type = ValueType::String;
                    // Store the C-string pointer (program strings are stable once loaded)
                    registers_[rd].string_val = program_->get_string(static_cast<size_t>(str_idx)).c_str();
                } else {
                    error_ = "Invalid string index in MOVSTR";
                    running_ = false;
                    break;
                }
                inst_index++;
                break;
            }

            case Opcode::HLT: {
                // Halt execution
                running_ = false;
                inst_index++;
                break;
            }

            case Opcode::NOP: {
                // No operation
                inst_index++;
                break;
            }

            default:
                error_ = "Unknown opcode";
                running_ = false;
                break;
        }
    }
    
    // Return value in R0 (register 0)
    return registers_[0].int_val;
}

int VM::execute_from_ast(void* ast_root) {
    (void)ast_root;
    error_ = "AST execution not yet implemented";
    return -1;
}

Value VM::get_register(uint8_t reg) const {
    if (reg < registers_.size()) {
        return registers_[reg];
    }
    return Value{};
}

void VM::set_register(uint8_t reg, const Value& val) {
    if (reg < registers_.size()) {
        registers_[reg] = val;
    }
}

} // namespace vm
} // namespace veldanava

