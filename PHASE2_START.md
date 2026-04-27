# Phase 2: Critical Compiler Fixes - STARTED

## Date: 2026-04-25

## Changes Made in This Phase

### 1. ✅ Phase 1 Complete - Naming Convention
- All `.vel` references removed, only `.veldan` remains
- All scripts updated: `velc` → `veldanc`, `velc_selfhost` → `veldanc_selfhost`
- Driver updated to accept ONLY `.veldan`
- Documentation updated

### 2. 🔧 Lexer Fix - peek_token()
**File**: `src/frontend/lexer/lexer.cpp`
**Issue**: `peek_token()` was returning `next_token()` instead of peeking  
**Fix**: Implemented proper peek by saving/restoring lexer state
```cpp
Token Lexer::peek_token() {
    if (is_at_end()) {
        return make_token(TokenType::EndOfFile, 0);
    }
    
    size_t saved_pos = position_;
    size_t saved_line = line_;
    size_t saved_column = column_;
    
    Token token = next_token();
    
    position_ = saved_pos;
    line_ = saved_line;
    column_ = saved_column;
    
    return token;
}
```

### 3. 🔧 Parser Fix - parse_type_expr() + Type Support
**Files**: `src/frontend/parser/parser.cpp`, `src/frontend/parser/parser.h`, `src/frontend/ast/ast.cpp`, `src/frontend/ast/ast.h`

**Issues Fixed**:
- `parse_type_expr()` was TODO, not implemented
- `LetStmtNode` had `type_expr` field but it wasn't being used
- Generic parameters parsed but not stored
- Return type parsed but not stored

**Changes**:

a) Added `parse_type_expr()` function to parse type annotations:
```cpp
Expr* Parser::parse_type_expr() {
    // Handles: int, str, bool, float, vec<T>, Registry<K,V>
}
```

b) Updated `LetStmtNode::create_let()` to accept type_expr parameter:
```cpp
LetStmtNode* create_let(const char* name, Expr* init, bool mutable_flag, Expr* type_expr = nullptr);
```

c) Updated `parse_manifest_statement()` to parse and store type expressions:
```cpp
if (check(TokenType::Colon)) {
    advance();
    type_expr = parse_type_expr();  // NEW
}
...
LetStmtNode* result = builder_.create_let(name, initializer, is_mutable, type_expr);  // UPDATED
```

d) Updated AST header to include default parameter for backward compatibility

### 4. 🔧 Parser Fix - Generic Parameters & Return Type
**File**: `src/frontend/parser/parser.cpp`

**Status**: Partially complete - parsing works but storage in FnStmtNode needs verification

- Generic parameters parsed in `parse_function_definition()` (line 707-723)
- Return type parsed (line 743-750)
- Need to verify they're properly stored in FnStmtNode fields

## Critical Blockers Remaining (C1-C11)

| ID | Issue | Status |
|----|-------|--------|
| C1 | Lexer `handle_indentation()` empty | ❌ Not started |
| C2 | Lexer `skip_multi_line_comment()` stub | ❌ Not started |
| C3 | Lexer `peek_token()` broken | ✅ FIXED |
| C4 | Lexer `scan_string()` loses first char | ❌ Not started |
| C5 | Parser `parse_type_expr()` TODO | ✅ FIXED |
| C6 | Parser OOP stubs | ❌ Not started |
| C7 | Parser memory statements missing | ❌ Not started |
| C8 | Codegen string/print bug | ❌ Not started |
| C9 | Codegen loop limit | ❌ Not started |
| C10 | VM unknown opcode after HLT | ❌ Not started |
| C11 | Driver pipeline bypasses IR | ❌ Not started |

## Testing

Builds successfully:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4  # No errors
```

## Next Steps (Phase 2)

1. Fix Lexer `handle_indentation()` - implement full Python-style indent/dedent
2. Fix Lexer `skip_multi_line_comment()` - proper scanning for `*/`
3. Fix Lexer `scan_string()` - investigate first-char loss
4. Fix Lexer inline ASM - add SIMD character classification
5. Fix Parser OOP definitions - implement Class/Struct/Trait/Impl nodes
6. Fix Parser memory statements - implement forge/pur/clm/rsz/etc
7. Fix Codegen string handling - use PRINTS for strings, set Value.type
8. Fix Codegen unhandled nodes - add all missing AST node kinds
9. Fix VM dispatch - replace switch with computed goto
10. Fix Driver pipeline - integrate TypeChecker, IR generation

## Estimated Time

- Lexer fixes (C1-C4): 2-3 hours
- Parser OOP/Memory (C6-C7): 3-4 hours  
- Codegen fixes (C8-C9): 2-3 hours
- VM/IR/Driver fixes (C10-C11): 3-4 hours

**Total**: ~10-14 hours for all critical blockers

## Notes

- String truncation issue in `scan_string()` needs investigation
- Need to verify Generic/Return types are stored in FnStmtNode
- Type system integration into driver pipeline still needed
- IR generation (AST→IR) not started
