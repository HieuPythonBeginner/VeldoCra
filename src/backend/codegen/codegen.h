/**
 * @file codegen.h
 * @brief VeldoCra Code Generation - AST to VM Instructions
 * @author Dr. Bright
 */

#pragma once

#include "backend/vm/instruction.h"
#include "frontend/ast/ast.h"

namespace veldanava {
namespace codegen {

/**
 * @brief Generate VM program from AST
 * @param root AST root node
 * @return VM program with generated instructions
 */
vm::Program generate_code(ast::Node* root);

} // namespace codegen
} // namespace veldanava
