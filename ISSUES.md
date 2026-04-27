# Veldanava - Issues & Tasks Tracking

## Critical Issues (Blockers)

### 1. Lexer Issues
- [ ] `peek_token()` - returns `next_token()` instead of peeking (src/frontend/lexer/lexer.cpp:114-115)
- [ ] `scan_string()` - missing first character of string (src/frontend/lexer/lexer.cpp:220-233)
- [ ] Inline ASM hot paths - missing SIMD for character classification (is_digit, is_alpha, is_whitespace)

### 2. Parser Issues
- [ ] `parse_type_expr()` - TODO, not implemented (parser.cpp:567, 612)
- [ ] Generic parameters - parsed but not stored in FnStmtNode (parser.cpp:714)
- [ ] Return type - ignored, need field in FnStmtNode (parser.cpp:750)
- [ ] Default parameters - stub only (parser.cpp:681-685)
- [ ] OOP definitions - `parse_oop_definition()` creates empty blocks only (parser.cpp:1630-1649)
- [ ] Import statements - declared but not implemented (parser.h:162)
- [ ] Memory statements - declared but not implemented (parser.h:175)
- [ ] Index expressions - TODO at line 1273
- [ ] Pattern node kinds - incorrectly set to MatchStmt (ast.cpp:532,544,557,570,601,630,664,679,724)

### 3. Type Checker
- [ ] Not integrated in Driver pipeline (driver.cpp:401-403)
- [ ] `TypeChecker::check_expression()` - returns Unknown (type.cpp:627-631)
- [ ] `TypeChecker::infer_type()` - stub (type.cpp:650-654)
- [ ] `check_trait_bounds()` - always false (type.cpp:474-479, 736-749)
- [ ] `substitute()` - only handles GenericType (type.cpp:751-780)
- [ ] Generic unification - placeholder returns (type.cpp:784-823)

### 4. IR Generation
- [ ] AST → IR visitor - doesn't exist
- [ ] `get_type_from_ast()` - always returns i32 (ir.cpp:552-556)
- [ ] SSA PHI nodes - not placed
- [ ] Control flow graph - not linked
- [ ] Optimization passes - not implemented

### 5. Codegen Issues
- [ ] String literals - uses PRINT instead of PRINTS (codegen_fixed.cpp:246-262)
- [ ] String truncation - first char missing (likely lexer or AST)
- [ ] Unhandled AST node kinds: MemberExpr, ArrayExpr, DictExpr, LambdaExpr, ClassStmt, StructStmt, etc. (codegen_fixed.cpp:439)
- [ ] Float arithmetic - only integer ops (codegen_fixed.cpp:141-144, 205-218)
- [ ] Function calls - builtin 'scribe' incorrectly handled, user functions lack prologue/epilogue (codegen_fixed.cpp:236-280)
- [ ] Return statements - no function entry/exit management (codegen_fixed.cpp:382-390)
- [ ] Loop limit - codegen generates wrong loops (codegen_fixed.cpp:334-380)
- [ ] HLT added twice - line 75 and line 113 (codegen_fixed.cpp)

### 6. Native Codegen
- [ ] AssemblerBackend uses own IR instead of `middle::ir` (assembler.cpp)
- [ ] Register allocator - simple counter overflows (assembler.cpp)
- [ ] LLVMBackend - returns empty vector (native_codegen.cpp:114-138)
- [ ] CraneliftBackend - not implemented (native_codegen.cpp:145-149)

### 7. VM Issues
- [ ] Switch dispatch - needs computed goto replacement (vm.cpp:201)
- [ ] PRINT/PRINTS - string type handling (vm.cpp:467-515)
- [ ] After HLT - still fetches instructions (vm.cpp:231-233)
- [ ] Loop limit - VM safeguard at 100k iterations (vm.cpp:191-195)
- [ ] GC integration - exists but not connected (gc.h, gc.cpp)
- [ ] Float support - Value has float_val but VM doesn't handle (vm.cpp)
- [ ] Stack frames - not implemented for function calls

### 8. Symbol Table & Memory
- [ ] Flat hash table - still uses std::unordered_map
- [ ] Arena allocator - only used for AST, not IR/symbols
- [ ] Stdlib binding - memory functions not fully connected to VM

### 9. Diagnostics & Safety
- [ ] Diagnostic engine - structured diagnostics not implemented
- [ ] Error propagation - uses nullptr instead of Result<T,E>
- [ ] Borrow checker - incomplete (ownership.cpp)
- [ ] Design-by-contract - pre/post/invariant not implemented
- [ ] Concurrent GC - channel primitive missing

### 10. Build System & Tooling
- [ ] Self-host scripts - reference old names (velc, .vel, src/velc/)
  - `build_selfhost.sh`
  - `compare_selfhost.sh`
  - `test_full_selfhost.sh`
  - `test_selfhost.sh`
- [ ] Driver - uses 'velc' binary name (driver.cpp:266)
- [ ] CMake - no install target
- [ ] CMake - no test target
- [ ] Documentation - needs updates

## Status Tracking

### Phase 1: Critical Fixes (IN PROGRESS)
- [ ] Update self-host scripts and driver to use veldanc/.veldan
  - Started: 2026-04-25
  - Status: In Progress

### Phase 2: Lexer & Parser
- [ ] Fix peek_token()
- [ ] Fix scan_string()
- [ ] Implement parse_type_expr()
- [ ] Store generic/return types

### Phase 3: Type System & IR
- [ ] Integrate TypeChecker
- [ ] Implement AST → IR
- [ ] SSA construction

### Phase 4: Codegen & VM
- [ ] Fix string handling
- [ ] Handle all AST nodes
- [ ] Native backend integration
- [ ] Computed goto VM dispatch

## Notes
- Total tasks: ~80+ items
- Critical blockers: 11 items (must fix for pipeline)
- After C1-C11: compiler can compile simple programs
