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

namespace veldocra {
namespace vm {

VM::VM(size_t register_count)
    : registers_(register_count + 4)
    , program_(nullptr)
    , pc_(0)
    , running_(false)
    , debug_(false)
    , zero_flag_(false)
    , sign_flag_(false)
    , overflow_flag_(false)
{
}

VM::~VM() = default;

VM::VM(VM&&) noexcept = default;
VM& VM::operator=(VM&&) noexcept = default;

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
    
    while (!stack_.empty()) stack_.pop();
    while (!call_stack_.empty()) call_stack_.pop();
    
    for (auto& reg : registers_) {
        reg = Value{};
    }
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
    while (running_ && pc_ < program_->size()) {
        const Instruction& inst = program_->get(pc_);
        
        if (debug_) {
            std::cerr << "[VM] PC=" << pc_ << " OP=" << opcode_to_string(inst.op) << "\n";
        }
        
        switch (inst.op) {
            case Opcode::HLT:
                running_ = false;
                if (debug_) std::cerr << "[VM] HLT\n";
                break;
                
            case Opcode::NOP:
                pc_++;
                break;
                
            case Opcode::MOV: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                registers_[rd] = registers_[ra];
                pc_++;
                break;
            }
                
            case Opcode::MOVI: {
                uint8_t rd = inst.rd();
                int32_t imm = inst.imm32();
                registers_[rd] = Value::from_int(imm);
                pc_++;
                break;
            }
                
            case Opcode::ADD: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val + registers_[rb].int_val;
                pc_++;
                break;
            }
                
            case Opcode::SUB: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val - registers_[rb].int_val;
                pc_++;
                break;
            }
                
            case Opcode::MUL: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val * registers_[rb].int_val;
                pc_++;
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
                pc_++;
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
                pc_++;
                break;
            }
                
            case Opcode::NEG: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = -registers_[ra].int_val;
                pc_++;
                break;
            }
                
            case Opcode::AND: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val & registers_[rb].int_val;
                pc_++;
                break;
            }
                
            case Opcode::OR: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val | registers_[rb].int_val;
                pc_++;
                break;
            }
                
            case Opcode::XOR: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val ^ registers_[rb].int_val;
                pc_++;
                break;
            }
                
            case Opcode::NOT: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = ~registers_[ra].int_val;
                pc_++;
                break;
            }
                
            case Opcode::SHL: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val << registers_[rb].int_val;
                pc_++;
                break;
            }
                
            case Opcode::SHR: {
                uint8_t rd = inst.rd();
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                registers_[rd].type = ValueType::Int;
                registers_[rd].int_val = registers_[ra].int_val >> registers_[rb].int_val;
                pc_++;
                break;
            }
                
            case Opcode::CMP: {
                uint8_t ra = inst.ra();
                uint8_t rb = inst.rb();
                zero_flag_ = (registers_[ra].int_val == registers_[rb].int_val);
                sign_flag_ = (registers_[ra].int_val < registers_[rb].int_val);
                pc_++;
                break;
            }
                
            case Opcode::JMP: {
                uint32_t target = inst.target();
                pc_ = target;
                break;
            }
                
            case Opcode::JZ: {
                uint32_t target = inst.target();
                pc_ = zero_flag_ ? target : pc_ + 1;
                break;
            }
                
            case Opcode::JNZ: {
                uint32_t target = inst.target();
                pc_ = !zero_flag_ ? target : pc_ + 1;
                break;
            }
                
            case Opcode::JE: {
                uint32_t target = inst.target();
                pc_ = zero_flag_ ? target : pc_ + 1;
                break;
            }
                
            case Opcode::JNE: {
                uint32_t target = inst.target();
                pc_ = !zero_flag_ ? target : pc_ + 1;
                break;
            }
                
            case Opcode::JG: {
                uint32_t target = inst.target();
                bool cond = !zero_flag_ && !sign_flag_;
                pc_ = cond ? target : pc_ + 1;
                break;
            }
                
            case Opcode::JGE: {
                uint32_t target = inst.target();
                pc_ = !sign_flag_ ? target : pc_ + 1;
                break;
            }
                
            case Opcode::JL: {
                uint32_t target = inst.target();
                bool cond = sign_flag_ && !zero_flag_;
                pc_ = cond ? target : pc_ + 1;
                break;
            }
                
            case Opcode::JLE: {
                uint32_t target = inst.target();
                bool cond = zero_flag_ || sign_flag_;
                pc_ = cond ? target : pc_ + 1;
                break;
            }
                
            case Opcode::CALL: {
                uint32_t target = inst.target();
                call_stack_.push(pc_ + 1);
                pc_ = target;
                break;
            }
                
            case Opcode::RET: {
                if (call_stack_.empty()) {
                    running_ = false;
                    break;
                }
                pc_ = call_stack_.top();
                call_stack_.pop();
                break;
            }
                
            case Opcode::PRINT: {
                uint8_t rd = inst.rd();
                Value& val = registers_[rd];
                switch (val.type) {
                    case ValueType::Int:
                        std::cout << val.int_val;
                        break;
                    case ValueType::Float:
                        std::cout << val.float_val;
                        break;
                    case ValueType::Bool:
                        std::cout << (val.bool_val ? "true" : "false");
                        break;
                    case ValueType::Char:
                        std::cout << val.char_val;
                        break;
                    case ValueType::String:
                        std::cout << (val.string_val ? val.string_val : "(null)");
                        break;
                    default:
                        std::cout << "none";
                }
                pc_++;
                break;
            }
                
            case Opcode::PRINTS: {
                uint8_t rd = inst.rd();
                size_t str_idx = registers_[rd].int_val;
                if (program_ && str_idx < program_->string_count()) {
                    std::cout << program_->get_string(str_idx);
                }
                pc_++;
                break;
            }
                
            case Opcode::READ: {
                uint8_t rd = inst.rd();
                int64_t val;
                std::cin >> val;
                registers_[rd] = Value::from_int(val);
                pc_++;
                break;
            }
                
            case Opcode::PUSH: {
                uint8_t rd = inst.rd();
                stack_.push(registers_[rd]);
                pc_++;
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
                pc_++;
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
                pc_++;
                break;
            }
                
            default:
                error_ = "Unknown opcode";
                running_ = false;
                break;
        }
    }
    
    return 0;
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
} // namespace veldocra

