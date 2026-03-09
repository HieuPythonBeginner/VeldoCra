# Phase 7 Implementation Plan

## Information Gathered

### Files Examined:
1. `PHASE7_AI_TASK_PROMPT.md` - Task requirements
2. `src/frontend/lexer/token.h` - Token types (all Phase 7 keywords already defined)
3. `src/frontend/lexer/keyword_table.h` - Keywords registered with categories
4. `src/frontend/parser/parser.cpp` - Parser implementation (needs updates)
5. `src/frontend/parser/parser.h` - Parser header
6. `src/frontend/ast/ast.h` - AST nodes (needs MemberExprNode)

### Current Status:
- ✅ Token types defined for all Phase 7 keywords
- ✅ Keywords registered in keyword table with categories
- ❌ Parser doesn't handle Phase 7 keywords properly
- ❌ Missing MemberExprNode for ego.core member access

## Implementation Plan

### Phase 1: Add MemberExprNode to AST (ast.h)
- Add `MemberExprNode` struct for `ego.field` and `core.field` access
- Add `create_member()` method to ASTBuilder

### Phase 2: Update Parser (parser.cpp)

#### 2.1 Handle ego and core keywords
- When `ego` or `core` is encountered, parse as special "this/self" reference
- If followed by `.`, create MemberExprNode

#### 2.2 Handle Memory keywords
- `forge(<size>)`, `pur(<size>)`, `clm(<size>)`, `rsz(<ptr>, <size>)` - function calls
- `ee <variable>`, `ee_inf_layers <variable>` - unary operations (no parentheses)

#### 2.3 Hybrid colon and braces
- Flow control (`verdict`, `fail`, `cycle`, `sustain`): support both `{}` and `:` with `;`
- OOP (`race`, `avatar`, `grant`, `blessing`): support both `{}` and `:` with `;`
- Error handling (`clash`, `counter`): support both `{}` and `:` with `;`

#### 2.4 origin keyword
- `origin` alone: activates all ASM keywords in file
- `origin { }`: limits ASM keywords to within block

### Phase 3: Testing
- Create test files to verify each feature

## Files to Edit:
1. `src/frontend/ast/ast.h` - Add MemberExprNode
2. `src/frontend/ast/ast.cpp` - Add MemberExprNode implementation
3. `src/frontend/parser/parser.cpp` - Implement Phase 7 keyword handling
4. `src/frontend/parser/parser.h` - Add helper method declarations if needed

