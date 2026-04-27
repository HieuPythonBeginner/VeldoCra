# Fix Plan - Veldanava Compiler Pipeline

## Critical Fixes (from must_done.md analysis)

### 1. parser.cpp - String Literal Bug
- [ ] Fix string literal: use `create_literal(text)` (std::string_view) instead of `create_literal(static_cast<const char*>(text.data()))`
- [ ] Create `IndexExprNode` in `parse_postfix()`
- [ ] Create `MemberExprNode` in `parse_postfix()`
- [ ] Use `parse_parameter()` in `parse_function_definition()`

### 2. vm.cpp - Conditional Jump Desync
- [ ] Fix `pc_` not updated for non-jump instructions → conditional jumps compute wrong target
- [ ] Ensure `pc_ = inst_index` before each jump calculation

### 3. Build & Test
- [ ] Build project
- [ ] Run tests

