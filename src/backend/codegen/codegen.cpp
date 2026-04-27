/**
 * @file codegen.cpp
 * @brief Dummy codegen implementation
 */

#include "backend/codegen/codegen.h"

namespace veldanava {
namespace codegen {

vm::Program generate_code(ast::Node* root) {
    // Dummy implementation - return empty program with HLT
    vm::Program program;
    program.add(vm::Opcode::HLT);
    return program;
}

} // namespace codegen
} // namespace veldanava
