/**
 * @file codegen_fixed.cpp
 * @brief Fixed version of codegen.cpp - compilation error resolved
 */

#include "backend/codegen/codegen.h"
#include "backend/vm/instruction.h"
#include "frontend/ast/ast.h"
#include "frontend/lexer/token.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace veldanava {
namespace codegen {

// Forward declarations
uint8_t generate_node(vm::Program& program, ast::Node* node, 
    std::unordered_map<std::string, uint8_t>& variables, 
    size_t& reg_counter,
    std::unordered_map<std::string, size_t>& function_addresses,
    std::vector<std::pair<size_t, std::string>>& calls_to_fix);

void generate_block(vm::Program& program, ast::BlockStmtNode* block,
    std::unordered_map<std::string, uint8_t>& variables,
    size_t& reg_counter,
    std::unordered_map<std::string, size_t>& function_addresses,
    std::vector<std::pair<size_t, std::string>>& calls_to_fix,
    bool is_root = false);

void generate_function(vm::Program& program, ast::FnStmtNode* fn,
    std::unordered_map<std::string, uint8_t>& variables,
    size_t& reg_counter,
    std::unordered_map<std::string, size_t>& function_addresses,
    std::vector<std::pair<size_t, std::string>>& calls_to_fix) {
    if (!fn || !fn->body) return;
    function_addresses[fn->name] = program.size();
    for (size_t i = 0; i < fn->param_count; ++i) {
        variables[fn->params[i]->name] = static_cast<uint8_t>(i + 1);
    }
    generate_block(program, static_cast<ast::BlockStmtNode*>(fn->body), 
        variables, reg_counter, function_addresses, calls_to_fix, false);
}

void generate_block(vm::Program& program, ast::BlockStmtNode* block,
    std::unordered_map<std::string, uint8_t>& variables,
    size_t& reg_counter,
    std::unordered_map<std::string, size_t>& function_addresses,
    std::vector<std::pair<size_t, std::string>>& calls_to_fix,
    bool is_root) {
    std::cerr << "[CodeGen] generate_block start, is_root=" << is_root 
              << ", program.size=" << program.size() << "\n";
    if (!block || block->statement_count == 0 || !block->statements) {
        std::cerr << "[CodeGen] generate_block null or empty\n";
        return;
    }
    std::vector<ast::Node*> main_stmts;
    std::vector<ast::FnStmtNode*> fn_stmts;
    for (size_t i = 0; i < block->statement_count; i++) {
        if (block->statements[i]->kind == ast::NodeKind::FnStmt) {
            fn_stmts.push_back(static_cast<ast::FnStmtNode*>(block->statements[i]));
        } else {
            main_stmts.push_back(block->statements[i]);
        }
    }
    std::cerr << "[CodeGen] generate_block before main statements, program.size=" 
              << program.size() << "\n";
    for (auto* stmt : main_stmts) {
        generate_node(program, stmt, variables, reg_counter, function_addresses, calls_to_fix);
    }
    for (auto* fn : fn_stmts) {
        generate_function(program, fn, variables, reg_counter, function_addresses, calls_to_fix);
    }
}

vm::Program generate_code(ast::Node* root) {
    vm::Program program;
    if (!root) {
        std::cerr << "[CodeGen] Warning: null root, adding HLT\n";
        program.add(vm::Opcode::HLT);
        return program;
    }
    std::unordered_map<std::string, uint8_t> variables;
    std::unordered_map<std::string, size_t> function_addresses;
    std::vector<std::pair<size_t, std::string>> calls_to_fix;
    size_t reg_counter = 1;
    if (root->kind == ast::NodeKind::BlockStmt) {
        auto* block = static_cast<ast::BlockStmtNode*>(root);
        if (block) generate_block(program, block, variables, reg_counter, function_addresses, calls_to_fix, true);
    } else {
        std::cerr << "[CodeGen] Warning: root not BlockStmt kind " 
                  << static_cast<int>(root->kind) << "\n";
        program.add(vm::Opcode::HLT);
    }
    std::cerr << "[CodeGen] Final program size: " << program.size() << "\n";
    for (size_t i = 0; i < program.size(); i++) {
        auto& inst = program.get(i);
        std::cerr << "[CodeGen] Inst " << i << ": " << opcode_to_string(inst.op) << "\n";
    }
    for (auto& p : calls_to_fix) {
        auto it = function_addresses.find(p.second);
        if (it != function_addresses.end()) {
            program.set_jump_target(p.first, static_cast<uint32_t>(it->second));
        } else {
            std::cerr << "[CodeGen] Error: Function '" << p.second << "' not found for CALL fix\n";
        }
    }
    std::cerr << "[CodeGen] Program size before HLT: " << program.size() << "\n";
    program.add(vm::Opcode::HLT);
    std::cerr << "[CodeGen] Program size after HLT: " << program.size() << "\n";
    return program;
}

uint8_t generate_node(vm::Program& program, ast::Node* node,
    std::unordered_map<std::string, uint8_t>& variables,
    size_t& reg_counter,
    std::unordered_map<std::string, size_t>& function_addresses,
    std::vector<std::pair<size_t, std::string>>& calls_to_fix) {
    if (!node) {
        std::cerr << "[CodeGen] Warning: null node\n";
        return 0;
    }
    std::cerr << "[CodeGen] generate_node entered, node kind: " << static_cast<int>(node->kind) << "\n";
    uint8_t next_reg = static_cast<uint8_t>((reg_counter++) % 16);
    std::cerr << "[CodeGen] reg_counter=" << reg_counter << " next_reg=" << (int)next_reg << "\n";
    switch (node->kind) {
        case ast::NodeKind::LiteralExpr: {
            auto* literal = static_cast<ast::LiteralExprNode*>(node);
            if (!literal) return 0;
            uint8_t reg = next_reg;
            std::cerr << "[CodeGen] LiteralExpr reg=" << (int)reg << " literal_type=" 
                      << (int)literal->literal_type << " val=" << literal->value.int_val << "\n";

            if (literal->literal_type == 0) {
                program.add(vm::Opcode::MOVI, reg, static_cast<int32_t>(literal->value.int_val));
            } else if (literal->literal_type == 1) {
                int32_t float_bits;
                std::memcpy(&float_bits, &literal->value.float_val, sizeof(float_bits));
                program.add(vm::Opcode::MOVI, reg, float_bits);
            } else if (literal->literal_type == 2) {
                size_t str_id = program.add_string(literal->value.string_val);
                std::cerr << "[CodeGen] String literal: idx=" << str_id << " val=" << literal->value.string_val << "\n";
                program.add(vm::Opcode::PRINTS, 0, static_cast<int32_t>(str_id));
                return 0; // String literals print directly
            } else if (literal->literal_type == 3) {
                program.add(vm::Opcode::MOVI, reg, literal->value.bool_val ? 1 : 0);
            } else if (literal->literal_type == 4) {
                program.add(vm::Opcode::MOVI, reg, static_cast<int32_t>(literal->value.char_val));
            } else if (literal->literal_type == 5) {
                program.add(vm::Opcode::MOVI, reg, 0);
            }
            return reg;
        }
        case ast::NodeKind::VariableExpr: {
            auto* var = static_cast<ast::VariableExprNode*>(node);
            auto it = variables.find(var->name);
            if (it != variables.end()) {
                return it->second;
            }
            std::cerr << "[CodeGen] Error: Undefined variable '" << var->name << "'\n";
            return 0;
        }
        case ast::NodeKind::BinaryExpr: {
            auto* binary = static_cast<ast::BinaryExprNode*>(node);
            if (binary->op == static_cast<int>(lexer::TokenType::Assign)) {
                if (binary->left->kind == ast::NodeKind::VariableExpr) {
                    auto* var = static_cast<ast::VariableExprNode*>(binary->left);
                    auto it = variables.find(var->name);
                    if (it != variables.end()) {
                        uint8_t var_reg = it->second;
                        uint8_t reg_right = generate_node(program, binary->right, variables, reg_counter, function_addresses, calls_to_fix);
                        program.add(vm::Opcode::MOV, var_reg, reg_right);
                        return var_reg;
                    }
                    std::cerr << "[CodeGen] Error: Undefined variable in assignment '" << var->name << "'\n";
                } else {
                    std::cerr << "[CodeGen] Error: Invalid assignment target\n";
                }
                return 0;
            }
            uint8_t reg_left = generate_node(program, binary->left, variables, reg_counter, function_addresses, calls_to_fix);
            uint8_t reg_right = generate_node(program, binary->right, variables, reg_counter, function_addresses, calls_to_fix);
            uint8_t result_reg = static_cast<uint8_t>(reg_counter % 16);
            reg_counter++;
            if (binary->op >= static_cast<int>(lexer::TokenType::Eq) && binary->op <= static_cast<int>(lexer::TokenType::Gte)) {
                program.add(vm::Opcode::MOVI, result_reg, 0);
                program.add(vm::Opcode::CMP, 0, reg_left, reg_right);
                vm::Opcode jump_op = vm::Opcode::INVALID;
                switch (binary->op) {
                    case static_cast<int>(lexer::TokenType::Eq): jump_op = vm::Opcode::JE; break;
                    case static_cast<int>(lexer::TokenType::Neq): jump_op = vm::Opcode::JNE; break;
                    case static_cast<int>(lexer::TokenType::Lt): jump_op = vm::Opcode::JL; break;
                    case static_cast<int>(lexer::TokenType::Gt): jump_op = vm::Opcode::JG; break;
                    case static_cast<int>(lexer::TokenType::Lte): jump_op = vm::Opcode::JLE; break;
                    case static_cast<int>(lexer::TokenType::Gte): jump_op = vm::Opcode::JGE; break;
                }
                if (jump_op != vm::Opcode::INVALID) {
                    size_t jump_to_true = program.add(jump_op, 0);
                    size_t jump_to_end = program.add(vm::Opcode::JMP, 0);
                    size_t true_label = program.size();
                    program.add(vm::Opcode::MOVI, result_reg, 1);
                    size_t end_label = program.size();
                    program.set_jump_target(jump_to_true, true_label);
                    program.set_jump_target(jump_to_end, end_label);
                }
            } else {
                vm::Opcode op = vm::Opcode::INVALID;
                switch (binary->op) {
                    case static_cast<int>(lexer::TokenType::Plus): op = vm::Opcode::ADD; break;
                    case static_cast<int>(lexer::TokenType::Minus): op = vm::Opcode::SUB; break;
                    case static_cast<int>(lexer::TokenType::Star): op = vm::Opcode::MUL; break;
                    case static_cast<int>(lexer::TokenType::Slash): op = vm::Opcode::DIV; break;
                    case static_cast<int>(lexer::TokenType::Percent): op = vm::Opcode::MOD; break;
                    case static_cast<int>(lexer::TokenType::Ampersand): op = vm::Opcode::AND; break;
                    case static_cast<int>(lexer::TokenType::Pipe): op = vm::Opcode::OR; break;
                }
                if (op != vm::Opcode::INVALID) {
                    program.add(op, result_reg, reg_left, reg_right);
                }
            }
            return result_reg;
        }
        case ast::NodeKind::UnaryExpr: {
            auto* unary = static_cast<ast::UnaryExprNode*>(node);
            if (!unary || !unary->operand) return 0;
            if (unary->op >= 1000 && unary->op <= 1005) {
                uint8_t operand_reg = generate_node(program, unary->operand, variables, reg_counter, function_addresses, calls_to_fix);
                uint32_t builtin_id = 0;
                if (unary->op == 1000) { builtin_id = 106; }
                if (builtin_id > 0) {
                    program.add(vm::Opcode::CALL, builtin_id);
                }
                return 0;
            }
            std::cerr << "[CodeGen] UnaryExpr op=" << unary->op << " not implemented\n";
            return generate_node(program, unary->operand, variables, reg_counter, function_addresses, calls_to_fix);
        }
        case ast::NodeKind::CallExpr: {
            auto* call = static_cast<ast::CallExprNode*>(node);
            if (!call || !call->callee) return 0;

            // Handle different callee types
            if (call->callee->kind == ast::NodeKind::VariableExpr) {
                auto* var = static_cast<ast::VariableExprNode*>(call->callee);
                std::string func_name(var->name);

                // Builtin print functions
                if (func_name == "print" || func_name == "scribe" || func_name == "flow") {
                    for (size_t i = 0; i < call->arg_count; i++) {
                        ast::Expr* arg = call->args[i];
                        // Check for string literal
                        if (arg->kind == ast::NodeKind::LiteralExpr) {
                            auto* lit = static_cast<ast::LiteralExprNode*>(arg);
                            if (lit->literal_type == 2) { // string
                                // Add string to program's string table and use PRINTS
                                size_t str_idx = program.add_string(lit->value.string_val);
                                program.add(vm::Opcode::PRINTS, 0, static_cast<int32_t>(str_idx));
                                continue;
                            }
                        }
                        // For non-string arguments: generate code and PRINT
                        uint8_t arg_reg = generate_node(program, arg, variables, reg_counter, function_addresses, calls_to_fix);
                        program.add(vm::Opcode::PRINT, arg_reg, 0);
                    }
                    return 0;
                }

                // Regular function call - generate CALL with placeholder address
                // Generate code for arguments first (store in registers 1,2,3,...)
                for (size_t i = 0; i < call->arg_count; i++) {
                    uint8_t arg_reg = generate_node(program, call->args[i], variables, reg_counter, function_addresses, calls_to_fix);
                    program.add(vm::Opcode::MOV, static_cast<uint8_t>(i + 1), arg_reg);
                }

                std::cerr << "[CodeGen] Calling function: " << func_name << "\n";
                size_t call_inst = program.add(vm::Opcode::CALL, static_cast<uint32_t>(0));
                calls_to_fix.push_back({call_inst, func_name});

                // Return value will be in R0 after function returns
                return static_cast<uint8_t>(vm::Register::R0);
            }
            return 0;
        }
        case ast::NodeKind::LetStmt: {
            auto* let = static_cast<ast::LetStmtNode*>(node);
            if (let && let->name && let->initializer) {
                uint8_t reg = generate_node(program, let->initializer, variables, reg_counter, function_addresses, calls_to_fix);
                variables[let->name] = reg;
                return reg;
            }
            return 0;
        }
        case ast::NodeKind::FnStmt: {
            return 0;
        }
        case ast::NodeKind::IfStmt: {
            auto* if_stmt = static_cast<ast::IfStmtNode*>(node);
            if (!if_stmt || !if_stmt->condition) return 0;
            uint8_t cond_reg = generate_node(program, if_stmt->condition, variables, reg_counter, function_addresses, calls_to_fix);
            size_t jz_inst = program.add(vm::Opcode::JZ, cond_reg, 0, 0);
            if (if_stmt->then_branch) {
                generate_block(program, static_cast<ast::BlockStmtNode*>(if_stmt->then_branch), variables, reg_counter, function_addresses, calls_to_fix, false);
            }
            if (if_stmt->else_branch) {
                size_t jmp_inst = program.add(vm::Opcode::JMP, 0);
                size_t after_then = program.size();
                program.set_jump_target(jz_inst, static_cast<uint32_t>(after_then));
                generate_block(program, static_cast<ast::BlockStmtNode*>(if_stmt->else_branch), variables, reg_counter, function_addresses, calls_to_fix, false);
                size_t after_else = program.size();
                program.set_jump_target(jmp_inst, static_cast<uint32_t>(after_else));
            } else {
                size_t after_then = program.size();
                program.set_jump_target(jz_inst, static_cast<uint32_t>(after_then));
            }
            return 0;
        }
        case ast::NodeKind::WhileStmt: {
            auto* while_stmt = static_cast<ast::WhileStmtNode*>(node);
            if (!while_stmt || !while_stmt->condition || !while_stmt->body) return 0;
            size_t loop_start = program.size();
            uint8_t cond_reg = generate_node(program, while_stmt->condition, variables, reg_counter, function_addresses, calls_to_fix);
            uint8_t zero_reg = static_cast<uint8_t>(reg_counter % 16); reg_counter++;
            program.add(vm::Opcode::MOVI, zero_reg, (int32_t)0);
            // Compare cond_reg with zero: CMP with rd=0 dummy
            program.add(vm::Opcode::CMP, 0, cond_reg, zero_reg);
            // Jump to exit if condition is false (zero) using JE (jump if equal/zero)
            size_t jump_out = program.add(vm::Opcode::JE, (uint32_t)0);
            generate_block(program, static_cast<ast::BlockStmtNode*>(while_stmt->body), variables, reg_counter, function_addresses, calls_to_fix, false);
            size_t jmp_back = program.add(vm::Opcode::JMP, (uint32_t)0);
            program.set_jump_target(jmp_back, static_cast<uint32_t>(loop_start));
            // Exit point (fall through)
            size_t exit_pc = program.size();
            program.set_jump_target(jump_out, static_cast<uint32_t>(exit_pc));
            return 0;
        }
        case ast::NodeKind::ForStmt: {
            auto* for_stmt = static_cast<ast::ForStmtNode*>(node);
            if (!for_stmt || !for_stmt->iterator_name || !for_stmt->iterable || !for_stmt->body) return 0;

            // Evaluate iterable (limit) once
            uint8_t limit_reg = generate_node(program, for_stmt->iterable, variables, reg_counter, function_addresses, calls_to_fix);
            // Allocate iterator register
            uint8_t iter_reg = static_cast<uint8_t>(reg_counter % 16); reg_counter++;
            // iter = 0
            program.add(vm::Opcode::MOVI, iter_reg, (int32_t)0);

            // Save iterator variable in current scope
            bool had_iter = variables.find(for_stmt->iterator_name) != variables.end();
            uint8_t old_iter_reg = 0;
            if (had_iter) {
                old_iter_reg = variables[for_stmt->iterator_name];
            }
            variables[for_stmt->iterator_name] = iter_reg;

            // Loop start
            size_t loop_start = program.size();
            // Compare iter with limit: CMP 0, iter_reg, limit_reg
            program.add(vm::Opcode::CMP, 0, iter_reg, limit_reg);
            // If iter >= limit, exit loop (use JGE)
            size_t exit_jump = program.add(vm::Opcode::JGE, (uint32_t)0);

            // Loop body
            generate_block(program, static_cast<ast::BlockStmtNode*>(for_stmt->body), variables, reg_counter, function_addresses, calls_to_fix, false);

            // Increment: iter = iter + 1
            uint8_t one_reg = static_cast<uint8_t>(reg_counter % 16); reg_counter++;
            program.add(vm::Opcode::MOVI, one_reg, (int32_t)1);
            program.add(vm::Opcode::ADD, iter_reg, iter_reg, one_reg);

            // Jump back to loop start
            program.add(vm::Opcode::JMP, (uint32_t)loop_start);
            // Exit point (fall through)
            size_t exit_pc = program.size();
            program.set_jump_target(exit_jump, static_cast<uint32_t>(exit_pc));

            // Restore iterator variable scope
            if (had_iter) {
                variables[for_stmt->iterator_name] = old_iter_reg;
            } else {
                variables.erase(for_stmt->iterator_name);
            }
            return 0;
        }
        case ast::NodeKind::ReturnStmt: {
            auto* ret = static_cast<ast::ReturnStmtNode*>(node);
            if (ret && ret->value) {
                uint8_t reg = generate_node(program, ret->value, variables, reg_counter, function_addresses, calls_to_fix);
                program.add(vm::Opcode::MOV, static_cast<uint8_t>(vm::Register::R0), reg);
            }
            program.add(vm::Opcode::RET);
            return 0;
        }
        case ast::NodeKind::BlockStmt: {
            std::cerr << "[CodeGen] BlockStmt\n";
            auto* block = static_cast<ast::BlockStmtNode*>(node);
            if (block) generate_block(program, block, variables, reg_counter, function_addresses, calls_to_fix, false);
            return 0;
        }
        case ast::NodeKind::MatchStmt: {
            auto* match = static_cast<ast::MatchStmtNode*>(node);
            if (!match || !match->expression || !match->arms) {
                std::cerr << "[CodeGen] Invalid MatchStmt\n";
                return 0;
            }
            
            std::cerr << "[CodeGen] MatchStmt: generating code\n";
            
            // Generate code for the expression to match
            uint8_t expr_reg = generate_node(program, match->expression, variables, reg_counter, function_addresses, calls_to_fix);
            
            // Store expression value in a register for comparisons
            uint8_t match_reg = static_cast<uint8_t>(reg_counter++ % 16);
            program.add(vm::Opcode::MOV, match_reg, expr_reg);
            
            // Collect jump instructions that need patching
            std::vector<size_t> arm_fail_jumps;  // Jump to next arm if pattern doesn't match
            std::vector<size_t> body_end_jumps;  // Jump to end after body execution
            std::vector<size_t> arm_starts;  // Start positions of each arm
            
            // Generate code for each arm
            for (size_t i = 0; i < match->arm_count; i++) {
                auto* arm = match->arms[i];
                if (!arm || !arm->pattern) continue;
                
                arm_starts.push_back(program.size());
                
                // Handle pattern matching
                switch (arm->pattern->pattern_kind) {
                    case ast::PatternKind::Wildcard: {
                        // Wildcard _ matches everything, no comparison needed
                        std::cerr << "[CodeGen] Wildcard pattern\n";
                        break;
                    }
                    
                    case ast::PatternKind::Literal: {
                        auto* lit_pattern = static_cast<ast::LiteralPatternNode*>(arm->pattern);
                        if (!lit_pattern || !lit_pattern->value) break;
                        
                        // Generate code for the literal value
                        uint8_t lit_reg = generate_node(program, lit_pattern->value, variables, reg_counter, function_addresses, calls_to_fix);
                        
                        // Compare match_reg with lit_reg
                        uint8_t cmp_reg = static_cast<uint8_t>(reg_counter++ % 16);
                        program.add(vm::Opcode::MOVI, cmp_reg, 0);
                        program.add(vm::Opcode::CMP, 0, match_reg, lit_reg);
                        
                        // Jump to next arm if not equal (if result is 0, meaning not equal)
                        size_t jne_inst = program.add(vm::Opcode::JNE, 0);
                        arm_fail_jumps.push_back(jne_inst);
                        
                        std::cerr << "[CodeGen] Literal pattern matched\n";
                        break;
                    }
                    
                    case ast::PatternKind::Variable: {
                        // Variable pattern binds the value to a variable
                        auto* var_pattern = static_cast<ast::VariablePatternNode*>(arm->pattern);
                        if (var_pattern && var_pattern->variable_name) {
                            variables[var_pattern->variable_name] = match_reg;
                            std::cerr << "[CodeGen] Variable pattern: " << var_pattern->variable_name << "\n";
                        }
                        break;
                    }
                    
                    default: {
                        std::cerr << "[CodeGen] Unhandled pattern kind: " << static_cast<int>(arm->pattern->pattern_kind) << "\n";
                        break;
                    }
                }
                
                // Generate body code
                if (arm->body) {
                    if (arm->body->kind == ast::NodeKind::BlockStmt) {
                        generate_block(program, static_cast<ast::BlockStmtNode*>(arm->body), variables, reg_counter, function_addresses, calls_to_fix, false);
                    } else {
                        generate_node(program, arm->body, variables, reg_counter, function_addresses, calls_to_fix);
                    }
                }
                
                // Jump to end after body execution (skip remaining arms)
                if (i < match->arm_count - 1) {
                    size_t jmp_end = program.add(vm::Opcode::JMP, 0);
                    body_end_jumps.push_back(jmp_end);
                }
                
                // Patch fail jumps from this arm to here (start of next arm)
                size_t next_arm = program.size();
                for (size_t j : arm_fail_jumps) {
                    program.set_jump_target(j, static_cast<uint32_t>(next_arm));
                }
                arm_fail_jumps.clear();
            }
            
            // Patch body jumps to here (end of match)
            size_t match_end = program.size();
            for (size_t j : body_end_jumps) {
                program.set_jump_target(j, static_cast<uint32_t>(match_end));
            }
            
            return match_reg;
        }
        default:
            std::cerr << "[CodeGen] Unhandled node kind: " << static_cast<int>(node->kind) << "\n";
            return 0;
    }
}

} // namespace codegen
} // namespace veldanava
