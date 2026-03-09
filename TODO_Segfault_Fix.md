# Segfault Fix Plan - Phase 11

## Issues Identified

### 1. Null Pointer Dereference in VariableConstraint (Ada Layer)
- **Location**: `handle_let()` - `get_constraint()` can return nullptr
- **Fix**: Add null check before accessing constraint methods

### 2. Stack Management Issue (Nested Scope)
- **Location**: `OwnershipContext::exit_scope()` 
- **Issue**: Potential issue when `scope_variables_.empty()` check fails
- **Fix**: Add proper guard to prevent stack underflow

### 3. AST Walking Safety
- **Location**: `OwnershipChecker::walk_node()` and handlers
- **Fix**: Add additional null safety checks

## Implementation Steps

1. Fix `handle_let()` - Add null check for constraint
2. Fix `OwnershipContext::exit_scope()` - Add proper guards
3. Add debug output to identify exact crash location
4. Rebuild and test

## Files to Edit
- `/home/dr-bright-rathalus/VeldoCra/src/middle/ownership/ownership.cpp`

