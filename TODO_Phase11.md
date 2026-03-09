# Phase 11: 5-Layer Defense Ownership System

## Task Overview
Implement the 5-Layer Defense System for VeldoCra's ownership system as requested by Overlord Heino.

## Information Gathered

### Current State
- **ownership.h/cpp**: Uses `void*` for AST nodes, has stub implementations
- **ast.h**: Defines proper AST node types (NodeKind, Node, Expr, Stmt, etc.)
- **driver.cpp**: Already calls `checker.check(ast_root)` but needs proper AST integration
- **Type system**: Supports Soul-based types (Soul, Entity, Species, etc.)

### Key AST Types
- `NodeKind` - Enum for all node types
- `Node` - Base class with kind, line, column, type_info
- `Expr` - Expression base with value_category
- `Stmt` - Statement base
- Specific nodes: `LetStmtNode`, `BlockStmtNode`, `VariableExprNode`, `CallExprNode`, etc.

## Plan

### 1. Rewrite ownership.h - Add 5-Layer Defense Types

**Changes:**
- Replace `void* ast_root` with `ast::Node*` in OwnershipChecker
- Add `VariableConstraint` class for Ada layer (range tracking)
- Add `SafetyHook` class for Python layer (runtime IR hooks)
- Add proper forward declarations for ast namespace
- Keep ZERO-COST: All checks are compile-time only

### 2. Rewrite ownership.cpp - Implement Full Logic

**C++ Layer (Scope-based Cleanup):**
- Implement `handle_block()` to call `exit_scope()` when leaving BlockStmtNode
- Add RAII-style cleanup - mark all Owned variables as Destroyed when scope ends

**Ada Layer (Range & Constraint Guard):**
- Add `VariableConstraint` tracking in OwnershipContext
- Parse range expressions from LetStmtNode type annotations
- Check assignments against constraints statically

**Python Layer (Safe Runtime Fallback):**
- Add `SafetyHook` generation for complex checks
- Generate IR instructions for boundary checks that can't be static
- Track which variables need runtime safety checks

**Full AST Integration:**
- Replace all `void*` with `ast::Node*`, `ast::Stmt*`, `ast::Expr*`
- Implement proper node kind checking using `node->kind`
- Full Use-after-move detection
- Full Illegal Borrow detection on Soul/Entity types

**Rust Layer (Zero-Cost Lifetimes):**
- Lifetime tracking is already compile-time only
- Ensure no runtime overhead in final binary

### 3. Files to Edit
1. `/home/dr-bright-rathalus/VeldoCra/src/middle/ownership/ownership.h` - Full rewrite
2. `/home/dr-bright-rathalus/VeldoCra/src/middle/ownership/ownership.cpp` - Full rewrite

### 4. Follow-up Steps
- Build the project: `cd build && make`
- Test with sample VeldoCra files
- Verify zero-cost: No runtime ownership overhead

## Dependent Files
- `src/frontend/ast/ast.h` - AST node definitions (read-only)
- `src/driver/driver.cpp` - Already integrated (no changes needed)

