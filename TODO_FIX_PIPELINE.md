# Veldanava — Fix Pipeline C++ (Hướng A)

## Progress
- [ ] C8 — Codegen string/print bug
- [ ] C2 — Lexer skip_multi_line_comment
- [ ] C10 — VM dừng sau HLT
- [ ] Integrate Type Checker minimal vào Driver
- [ ] Tạo AST → IR visitor cơ bản

## C8: Codegen string/print bug
**File:** `src/backend/codegen/codegen_fixed.cpp`, `src/backend/vm/vm.cpp`, `src/frontend/lexer/lexer.cpp`
**Vấn đề:** String literal bị mất ký tự đầu hoặc VM PRINT không hiển thị string
**Hành động:**
1. Kiểm tra `Lexer::scan_string()` xem token.length có đúng không
2. Kiểm tra `ASTBuilder::create_literal(std::string_view)` xem arena copy có đủ không
3. Kiểm tra VM `PRINT` instruction xử lý `ValueType::String`
4. Đảm bảo codegen emit `PRINTS` cho string literal

## C2: Lexer skip_multi_line_comment  
**File:** `src/frontend/lexer/lexer.cpp`
**Vấn đề:** Chỉ scan đến EOF
**Hành động:** Implement scan đến `*/`, hỗ trợ nested comment

## C10: VM dừng sau HLT
**File:** `src/backend/vm/vm.cpp`
**Vấn đề:** VM fetch instruction sau HLT
**Hành động:** Set `running_ = false` và break loop ngay khi gặp HLT

## Type Checker Integration
**File:** `src/driver/driver.cpp`, `src/middle/types/type.cpp`
**Hành động:**
1. Tạo `TypeChecker` instance trong driver
2. Gọi `type_checker.check(ast_root)` sau ownership check
3. Implement minimal `check_expression()` cho literal, variable, binary

## AST → IR Visitor
**File:** `src/middle/ir/ir.cpp`, `src/middle/ir/ir.h`, `src/driver/driver.cpp`
**Hành động:**
1. Tạo class `IRGenerator` với methods visit cho từng NodeKind
2. Map AST type nodes sang `middle::ir::Type*`
3. Sinh IR: expressions → SSA values, statements → basic blocks
4. Thay `vm_to_ir()` giả bằng `IRGenerator::generate(ast_root)`

