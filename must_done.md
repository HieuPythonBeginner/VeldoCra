# Veldanava — Must-Done Tasks (Còn Thiếu & Cần Fix)

**Lưu ý**: Tên cũ (VeldoCra, Veldora, .veldan) đã được thay thế bằng tên mới:
- **Veldanava** (tên ngôn ngữ hiện tại)
- **veldanc** (compiler binary)
- **.veldan** (file extension)

File này liệt kê **TẤT CẢ** công việc còn thiếu, sắp xếp theo thứ tự ưu tiên thực hiện.

---

## ✅ Phase 1 Complete: Remove ALL .vel References

**Date**: 2026-04-25  
**Changes**: All `.vel` file references replaced with `.veldan`

- Scripts: `build_selfhost.sh`, `compare_selfhost.sh`, `test_selfhost.sh`, `test_full_selfhost.sh`
- Binary: `velc` → `veldanc`, `velc_selfhost` → `veldanc_selfhost`
- Driver: Accepts ONLY `.veldan` (`.vel` support removed)
- Config: `package.json`, `icons/veldora-icons.json`, `install-icons/*`
- Result: ZERO `.vel` files remain in project

---

## 📌 Giai Đoạn 1 — Fix Blockers & Pipeline Cơ Bản (NGAY LẬP TỨC)

### 1. Lexer — Fix bugs nghiêm trọng

- [ ] **`Lexer::handle_indentation()`** — hiện tại rỗng (src/frontend/lexer/lexer.cpp:119-120)
  - Implement theo Python-style: đẩy/pop indent stack, emit INDENT/DEDENT tokens.
  - Cần theo dõi `current_indent` và `indent_stack`.

- [ ] **`Lexer::skip_multi_line_comment()`** — chỉ `return true` (lexer.cpp:289-291)
  - Phải scan cho đến khi gặp `*/`, xử lý nested comment nếu cần.

- [x] **`Lexer::peek_token()`** — FIXED: save/restore state để peek không consume
  - Sửa để chỉ `peek` mà không consume token.

- [ ] **`Lexer::report_error()`** — chỉ tăng `error_count_` (lexer.cpp:132-134)
  - Mở rộng: lưu chi tiết lỗi (line, column, message) vào vector để driver báo cáo.

- [ ] **`Lexer::scan_string()`** — string bị mất ký tự đầu (test "Hello" → "ello")
  - Kiểm tra lại logic: sau khi gặp opening quote, `advance()` rồi lấy `start`; đến closing quote thì length = position - start. Có thể lỗi ở `make_token()` hoặc caching string.

- [ ] **Inline ASM hot paths** — yêu cầu spec: phân loại ký tự dùng SIMD inline assembly
  - Thêm assembly vào `is_digit()`, `is_alpha()`, `is_whitespace()` trong lexer.h.

### 2. Parser — Hoàn thiện type expressions & OOP stubs

- [x] **Parse type expressions** — `TODO` ở parser.cpp:567, 612
  - Viết `parse_type_expr()`: hỗ trợ `int`, `str`, `bool`, `float`, `vec<type>`, `Registry<k,v>`...
  - Gắn type vào LetStmt, ParamDecl, FnStmt.
  - Status: ✅ IMPLEMENTED

- [ ] **Generic parameters** — lưu vào FnStmtNode (parser.cpp:714)
  -Parser đã có `generic_params` nhưng không gán vào node → sửa.

- [ ] **Return type** — lưu vào FnStmtNode (parser.cpp:750)
  - Hiện tại ignore return type → thêm field `return_type` vào `FnStmtNode` và gán.

- [ ] **Default parameters** — parser.cpp:681-685 đã stub `default_value`
  - Hoàn thiện: parse default value expression (nếu có) và gán vào ParamDecl.

- [ ] **OOP definitions** — `parse_oop_definition()` chỉ tạo empty block (parser.cpp:1603-1628)
  - Implement đầy đủ:
    - `race` (class) → tạo `ClassStmtNode` với fields, methods, inheritance.
    - `avatar` (struct) → `StructStmtNode`.
    - `grant` (impl) → `ImplStmtNode` (có thể skip nếu chưa cần).
    - `blessing` (trait) → `TraitStmtNode`.
  - Phân biệt rõ `ego` (this) và `core` (self).

- [ ] **Import/Module system** — `parse_import_statement()` chưa define (parser.h:162)
  - Thêm `parse_import_statement()` xử lý `import`, `from`, `as`.

- [ ] **Memory statements** — `parse_memory_statement()` khai báo chưa define (parser.h:175)
  - Parse các từ khóa memory: `forge`, `pur`, `clm`, `rsz`, `ee`, `ee_inf_layers`, `touch`, `anomaly`, etc.
  - Biến đổi thành expression statements hoặc specialized nodes.

- [ ] **Index expression** — `TODO: Create index expression` (parser.cpp:1273)
  - Tạo `IndexExprNode` với `object` và `index` con trỏ.

- [ ] **Origin/asm statement** — tạo block nhưng không đánh dấu ASM mode (parser.cpp:1640-1661)
  - Thêm flag `is_asm_block` vào BlockStmtNode hoặc dùng node riêng `AsmStmt`.

- [ ] **Pattern matching node kinds** — AST pattern nodes set sai `kind` (ast.cpp:532,544,557,570,601,630,664,679,724)
  - Sửa: pattern nodes (WildcardPattern, LiteralPattern, VariablePattern, BindingPattern, TuplePattern) phải có `kind` tương ứng, không phải `NodeKind::MatchStmt`.

- [ ] **AST pattern node definitions** — nếu chưa có, thêm các struct pattern node vào ast.h.

### 3. AST — Sửa lỗi & hoàn thiện

- [ ] **F-string support** — `create_fstring()` tồn tại nhưng parser không dùng (ast.h:511)
  - Thêm `FStringExprNode` và parser code.

- [ ] **MemberExpr handling** — codegen chưa support, cần node đầy đủ.

### 4. Type Checking — Integrate & implement

- [ ] **Gọi TypeChecker trong Driver** sau ownership (driver.cpp:401-403 hiện chỉ ownership)
  - Thêm bước: `type_checker.check(ast_root);`

- [ ] **Implement `TypeChecker::check_expression()`** (type.cpp:627-631) — hiện return Unknown
  - Xử lý các expression types: literal, variable, binary, unary, call, etc.

- [ ] **Implement `TypeChecker::infer_type()`** (type.cpp:650-654)

- [ ] **Unify functions** — `unify_generic()`, `unify_trait()`, `unify_instance()` (type.cpp:784-823)
  - Hiện tại return placeholder → cần logic thực.

- [ ] **`check_trait_bounds()`** — luôn false (type.cpp:474-479, 736-749)
  - Implement kiểm tra implements trait.

- [ ] **`substitute()`** — chỉ xử lý GenericType (type.cpp:751-780)
  - Mở rộng cho StructType, FunctionType.

- [ ] **Type environment** — đảm bảo `TypeEnvironment` được dùng đúng scope.

### 5. IR Generation — Connect AST → IR

- [ ] **AST → IR visitor** — không tồn tại
  - Tạo class `IRGenerator` với methods visit cho từng node kind.
  - Sinh IR: expressions → SSA values, statements → basic blocks, functions → IRFunction.

- [ ] **`get_type_from_ast()`** — luôn trả `i32` (ir.cpp:552-556)
  - Map AST type nodes sang `middle::ir::Type*`.

- [ ] **SSA construction** — đặt PHI nodes cho phi Mathiền
  - Parser có thể đã tạo implicit phi; cần visitor đặt explicit.

- [ ] **Control Flow Graph** — link basic blocks sau khi sinh.

- [ ] **Optimization passes** (DCE, constant folding, inlining, escape analysis) — **chưa có**
  - Có thể làm sau khi IR generation ổn.

- [ ] **Driver: thay đổi pipeline** — bỏ `vm_to_ir()` giả, thay bằng IR thực từ AST.

### 6. Codegen (VM) — Fix strings & unhandled nodes

- [ ] **String literals**:
  - Codegen (`codegen_fixed.cpp`) dùng `PRINT` cho string (line 256) → VM `PRINT` không hiển thị string.
  - **Cách fix**: Với string literal, thay `PRINT` bằng `PRINTS` (dùng string index) HOẶC set type string trong Value.
  - Kiểm tra lại `LiteralExprNode` storage: `value.string_val` phải là `const char*` trỏ đến arena.

- [ ] **Fix string truncation** (first char missing) — có thể do lexer hoặc AST
  - Xác minn `scan_string()` trả về token với text đủ dài.
  - Đảm bảo `create_literal_expr()` copy toàn bộ string vào arena.

- [ ] **PRINT instruction** — VM `PRINT` (vm.cpp:457-481) switch on `val.type`
  - Hiện tại `unknown type` default → cần đảm bảo string có `type = ValueType::String`.
  - Codegen phải set type đúng; hoặc sửa VM để `PRINT` accept int rồi convert.

- [ ] **Unknown opcode after HLT** — codegen thêm HLT rồi VM vẫn lỗi (full.log)
  - Kiểm tra `Program::add()` và `Program.size()`; có thể codegen thêm instruction sau HLT (vô tình).
  - VM `execute()` nên dừng khi `ip >= program.size()` hoặc gặp HLT.

- [ ] **Loop "limit reached"** — VM có safeguard (có thể là `loop_counter_`)
  - Codegen sinh loop thiếu back-edge hoặc condition không thay đổi → VM tự thoát.
  - Fix while/for codegen để đúng.

- [ ] **Unhandled AST node kinds** trong codegen switch (codegen_fixed.cpp:439):
  - `MemberExpr`, `ArrayExpr`, `DictExpr`, `LambdaExpr`, `ClassStmt`, `StructStmt`, `EnumStmt`, `ImportStmt`, `MatchStmt` (đã có nhưng cần hoàn thiện), `ReturnStmt` (có), `IfStmt` (có), `WhileStmt` (có), `ForStmt` (có), `LetStmt` (có), `FnStmt` (chưa sinh body), v.v.
  - Thêm từng case một.

- [ ] **Float arithmetic** — literals float được xử lý (line 141-144) nhưng arithmetic ops (ADD/SUB/MUL/DIV) chỉ integer
  - Mở rộng: thêm opcode float hoặc dùng same op với float bits.

- [ ] **Function calls**:
  - Builtin `scribe` đã được xử lý近似 nhưng chưa đúng (line 252-258).
  - User-defined functions: sinh CALL và fix address (có), nhưng chưa có prologue/epilogue, chưa push/pop registers.
  - Cần stack frame management.

- [ ] **Return from function** — sinh RET, nhưng chưa có function entry/exit.

### 7. Native Codegen (AOT) — Reconnect IR

- [ ] **AssemblerBackend phải dùng `middle::ir`** — assembler.cpp hiện tự định nghĩa IR
  - Sửa: thay các struct `Function`, `BasicBlock`, `Instruction` bằng `middle::ir::IRFunction`, `IRBasicBlock`, `IRInstruction`.
  - `Assembler::assemble()` nhận `middle::ir::IRModule`, duyệt functions, basic blocks, instructions.

- [ ] **Register allocation** — hiện code chỉ dùng reg counter đơn giản, có overflow
  - Implement linear scan hoặc đơn giản: spill to stack khi hết reg.

- [ ] **Instruction encoding** — hiện `assembler.cpp` có thể đã có, nhưng cần xác minh nó đọc từ IR đúng.

- [ ] **ELF generator** — kiểm tra `elf.cpp` và `elf.h`:
  - Có sections: `.text`, `.data`, `.bss`, symtab?
  - Entry point đúng?
  - Nếu thiếu, implement.

- [ ] **LLVMBackend** — stub trả empty vector (native_codegen.cpp:114-138)
  - Hoàn thiện: iterate IR module, tạo LLVM IR, compile thành machine code.

- [ ] **CraneliftBackend** — chưa implement (native_codegen.cpp:145-149)
  - Nếu muốn hỗ trợ, thêm.

- [ ] **Driver execute_build()** hiện dùng `vm_to_ir()` giả → sửa để gọi `IRGenerator` rồi `NativeCodeGen`.

### 8. VM — Tối ưu & hoàn thiện

- [ ] **Replace switch dispatch với computed goto** (vm.cpp:201)
  - Dùng GCC extension: `static void* dispatch_table[] = &&label_0, &&label_1, ...; goto *dispatch_table[inst.op];`
  - Hoặc dùng inline assembly jump table.

- [ ] **Add inline ASM hot paths**:
  - VM instruction loop — computed goto là đủ.
  - Memory ops (LOAD/STORE) có thể dùng inline asm nếu cần.

- [ ] **`execute_from_ast()`** — stub (vm.cpp:786-790), nhưng đã có codegen nên có thể bỏ.

- [ ] **GC integration** — `gc.h` tồn tại nhưng không dùng
  - Implement reference counting trong `Value` (thêm `refcount`).
  - Cycle detector (Marksweep hoặc CPA) — sau khi reference counting đổ.

- [ ] **Fix string handling** trong `PRINT`/`PRINTS` — đã nói ở codegen.

- [ ] **Floating-point support** — `Value` có `float_val` nhưng VM arithmetic chưa xử lý.

- [ ] **Stack frame / BP** — chưa dùng; cần cho function call/return đúng.

- [ ] **Error after HLT** — VM vẫn fetch instruction sau HLT (vm.cpp:231-233)
  - Sửa: khi gặp HLT, set `running_ = false` và break loop.

### 9. Standard Library — FFI & type safety

- [ ] **Expose stdlib functions to bytecode**:
  - Tạo bảng `BuiltinFunction` với ID (e.g., 100+).
  - Codegen sinh `CALL` với builtin ID.
  - VM `CALL` xử lý builtin trước user functions.

- [ ] **Type-safe collections**:
  - Thay `void*` trong `Sequence`, `Registry`, `Collective` bằng template-style (có thể dùng `any` type hoặc generic).
  - Add bounds checking.

- [ ] **Implement stdlib theo docs/stdlib.md**:
  - `scribe` (print), `listen` (input), `inscribe`/`perceive` (file I/O).
  - Math: `abs_val`, `root`, `exp_val`, `floor_val`, `ceiling_val`, `sin_val`, ...
  - String: `lex_span`, `lex_void`, `lex_glyph_at`, `lex_elevate`, `lex_depress`, `lex_sanitize`, `lex_swap`, `lex_find`, `lex_anchors_with`, `lex_trails_with`, `lex_parse_int`, `lex_parse_float`, `lex_parse_logic`, `lex_shatter`, `lex_weave`.
  - Collections: `Sequence`, `Registry`, `Collective` với methods đầy đủ.

- [ ] **Memory functions** — `forge`, `pur`, `clm`, `rsz`, `ee`, `touch`, `manifest`, `ee_inf_layers` cần được tích hợp.
  - Hiện tại là wrapper C; cần binding vào VM.

### 10. Symbol Table & Middle Layers

- [ ] **Flat hash table with open addressing** — thay `std::unordered_map` trong parser/codegen
  - Implement `FlatHashTable<K,V>` với Robin hood hoặc linear probing.
  - Dùng cho symbol table, variable maps, etc.

- [ ] **Arena allocator toàn cục** — hiện chỉ dùng cho AST
  - Mở rộng: dùng arena cho IR, symbol table strings, etc.
  - Tránh `new`/`delete` ở middle/backend.

- [ ] **Data-Oriented Design** — cấu trúc cache-friendly:
  - IR instructions contiguous.
  - Symbol table entries contiguous.

### 11. Safety & Verification Layers — Hoàn thiện

- [ ] **Rust layer** — borrow checker đầy đủ:
  - Lifetime tracking (với `Lifetime` class).
  - Mutable vs immutable borrow conflicts.
  - Use-after-move (có rồi nhưng cần mở rộng).

- [ ] **SPARK layer** — design by contract:
  - Syntax: `precondition`, `postcondition`, `invariant`.
  - Static overflow checking (compile-time).
  - Null safety (maybe với Option type).

- [ ] **Go layer** — concurrent GC:
  - Implement low-pause GC (tri-color).
  - Channel primitive (đang thiếu).

- [ ] **Python layer** — reference counting + cycle detector:
  - Add refcount to heap objects.
  - Cycle detector (periodic mark-sweep).

- [ ] **Zero-cost principle** — remove debug prints from release build (`#ifdef NDEBUG`).

### 12. Diagnostics

- [ ] **Diagnostic Engine**:
  - Structured: `Diagnostic` với severity, message, location (file/line/col), snippet, fix-it.
  - Collect tất cả diagnostics, in đẹp (colors, underlines).
  - JSON output cho IDE.

- [ ] **Improve error propagation** — dùng `Result<T,E>` xuyên suốt, không trả `nullptr` thụ động.

### 13. Testing & CI

- [ ] **CMake test target** — thêm `add_test()` cho mỗi test file.
- [ ] **Automated test runner** — chạy tất cả `.veldan` trong `tests/`, so sánh output.
- [ ] **Unit tests** — cho lexer, parser, AST, codegen, VM (dùng framework như Catch2).
- [ ] **Performance benchmarks** — đo độ lệch so với C.
- [ ] **CI pipeline** — GitHub Actions:
  - Build on Ubuntu, macOS, Windows.
  - Run tests.
  - Lint/format check.

### 14. Documentation

- [ ] **IR_REFERENCE.md** — mô tả đầy đủ IR nodes, opcodes, SSA form, examples.
- [ ] **Language Specification** — grammar (BNF/EBNF), semantics, type system, ownership rules.
- [ ] **Stdlib API Reference** — theo `stdlib.md` nhưng cập nhật implementation.
- [ ] **Self-Host Guide** — cập nhật cho tên mới (`veldanc`, `.veldan`).
- [ ] **Developer Guide** — kiến trúc,Contributing, code style.
- [ ] **Update read.md** — bỏ design prompt, thay bằng project overview.

### 15. Build System & Scripts

- [ ] **Update self-host scripts**:
  - `build_selfhost.sh` → đổi `velc` → `veldanc`, `src/velc/` → `src/veldanc/`, `.vel` → `.veldan`.
  - `compare_selfhost.sh` → tương tự.
  - `test_full_selfhost.sh`, `test_selfhost.sh` → update.

- [ ] **Driver SelfHostCompare** — sửa binary tên sai (driver.cpp:266 đã ghi `velc`).

- [ ] **Add install target** — `make install` sao chép `veldanc` vào `/usr/local/bin`.

- [ ] **Package config** — tạo `veldanc.pc` cho pkg-config.

### 16. Performance Requirements (Spec)

- [ ] **Inline ASM trong lexer** — `is_digit`, `is_alpha`, `is_whitespace` dùng SIMD (x86-64).
- [ ] **Arena allocator fast path** — `allocate()` dùng inline asm bump pointer.
- [ ] **VM dispatch** — computed goto (đã nói).
- [ ] **Atomic primitives** — cho concurrency (nếu có), dùng `lock` prefix hoặc C++11 atomics với asm.

### 17. Tooling

- [ ] **LSP server** — hover, completion, go-to-definition.
- [ ] **Syntax highlighting** — đã có `.tmLanguage.json`, nhưng cần test VS Code.
- [ ] **Debugger** — GDB/LLDB integration, biết đọc bytecode.

---

## 📋 Bảng Tổng Hợp Phân Loại

### CRITICAL (Block pipeline)

| ID | Vấn đề | File | Ưu tiên |
|----|--------|------|---------|
| C1 | Lexer indentation empty | lexer.cpp:119-120 | Highest |
| C2 | Lexer comment stub | lexer.cpp:289-291 | High |
| C3 | Lexer peek_token wrong | lexer.cpp:84-86 | High |
| C4 | Lexer string truncation | lexer.cpp:157-? | High |
| C5 | Parser type expressions TODO | parser.cpp:567,612 | Highest |
| C6 | Parser OOP stubs | parser.cpp:1603-1628 | High |
| C7 | Parser memory statements missing | parser.h:175 | High |
| C8 | Codegen string/print bug | codegen_fixed.cpp:141-149,256 | Highest |
| C9 | Codegen loop limit reached | codegen_fixed.cpp:304-320 | High |
| C10 | VM unknown opcode after HLT | vm.cpp:231-233 | High |
| C11 | Driver pipeline bypass IR | driver.cpp:102-105,27-46 | Highest |

### HIGH (Giảm chất lượng, ảnh hưởng nhiều)

| ID | Vấn đề |
|----|--------|
| H1 | Type checker stub |
| H2 | IR generation missing |
| H3 | Native backend IR mismatch |
| H4 | Ownership borrow checker incomplete |
| H5 | Symbol table dùng std::unordered_map |
| H6 | Self-host scripts outdated |
| H7 | Diagnostics engine missing |
| H8 | Stdlib không integrate với VM |

### MEDIUM (Tối ưu, hoàn thiện)

| ID | Vấn đề |
|----|--------|
| M1 | Inline ASM hot paths |
| M2 | SIMD trong lexer |
| M3 | Computed goto VM dispatch |
| M4 | GC integration |
| M5 | Optimization passes |
| M6 | Import/module system |
| M7 | Float arithmetic |
| M8 | OOP codegen (classes) |
| M9 | Pattern matching hoàn chỉnh |

### LOW (Refactor, polish)

| ID | Vấn đề |
|----|--------|
| L1 | ASTPrinter |
| L2 | Source location accuracy |
| L3 | F-string support |
| L4 | Memory management (anomaly, voice) |
| L5 | JavaScript transpiler (disabled) |
| L6 | CI/CD setup |
| L7 | Documentation (IR ref, spec) |

---

## 🗂️ Checklist Từng File (Micro-level)

### `src/frontend/lexer/lexer.cpp`
- [ ] Line 84-86: `peek_token()` → fix to return token without advancing.
- [ ] Line 119-120: `handle_indentation()` → implement indent/dedent stack.
- [ ] Line 132-134: `report_error()` → store error details.
- [ ] Line 157-173: `scan_string()` → verify string length & storage.
- [ ] Line 282-291: `skip_single_line_comment()` & `skip_multi_line_comment()` → multi-line properly.
- [ ] Add SIMD inline asm for `is_digit`, `is_alpha`, `is_whitespace`.

### `src/frontend/parser/parser.cpp`
- [ ] Line 567: `parse_type_expr()` — implement and call.
- [ ] Line 612: type in parameter — use parsed type.
- [ ] Line 681-685: default parameter — parse default value expression.
- [ ] Line 714: generic params — store in FnStmtNode.
- [ ] Line 750: return type — store in FnStmtNode.
- [ ] Line 1273: index expression — create IndexExprNode.
- [ ] Line 1603-1628: `parse_oop_definition()` — full Class/Struct/Trait nodes.
- [ ] Line 1640-1661: `parse_origin_statement()` — mark ASM block.
- [ ] Add `parse_import_statement()`.
- [ ] Add `parse_memory_statement()`.

### `src/frontend/ast/ast.cpp`
- [ ] Line 532,544,557,570,601,630,664,679,724: pattern nodes — fix `kind`.
- [ ] Line 835-839: `ASTPrinter::to_string()` — implement traversal.
- [ ] Ensure all nodes set `start_offset`, `end_offset` from lexer tokens.

### `src/middle/types/type.cpp`
- [ ] Line 627-631: `check_expression()` — full implementation.
- [ ] Line 650-654: `infer_type()` — implementation.
- [ ] Line 474-479, 736-749: `check_trait_bounds()` — real logic.
- [ ] Line 751-780: `substitute()` — handle all type kinds.
- [ ] Line 784-823: unify_* — complete.

### `src/middle/ownership/ownership.cpp`
- [ ] Line 837: remove "MANIFESTATION LOG" or make debug-only.
- [ ] Line 1085-1115: `generate_safety_hooks()` — generate real IR safety checks.
- [ ] Implement full borrow checker (lifetimes, mutable/immutable).

### `src/middle/ir/ir.cpp`
- [ ] Line 552-556: `get_type_from_ast()` — map types correctly.
- [ ] Add IRBuilder methods for all expression/statement kinds.
- [ ] Implement SSA phi placement.

### `src/backend/codegen/codegen_fixed.cpp`
- [ ] String literal branch: use `PRINTS` or set string type in value.
- [ ] Ensure `LiteralExprNode` stores full string (no truncation).
- [ ] Add cases for unhandled node kinds.
- [ ] Fix function call prologue/epilogue.
- [ ] Handle float arithmetic.

### `src/backend/native_codegen/assembler.cpp`
- [ ] Change to use `middle::ir` structures.
- [ ] Implement instruction encoding per target (x86-64).
- [ ] Add register allocator.

### `src/backend/vm/vm.cpp`
- [ ] Line 201: replace switch with computed goto.
- [ ] Line 457-479: `PRINT` — ensure string type handled.
- [ ] Line 231-233: after HLT, break loop.
- [ ] Add safeguard to stop after loop limit reached (or fix loop generation).

### `src/driver/driver.cpp`
- [ ] Line 102-105: `generate_code()` — already calls real codegen, but ensure pipeline includes type checking.
- [ ] Line 27-46: `vm_to_ir()` — remove dummy, generate from IRGenerator.
- [ ] Line 401-403: insert type checking after ownership.
- [ ] Line 266 (in SelfHostCompare): use `veldanc` not `velc`.
- [ ] Remove commented code or enable appropriately.

### `CMakeLists.txt`
- [ ] Ensure `codegen_fixed.cpp` is included (is already? line 33 shows it).
- [ ] Add test target.
- [ ] Add install target.

---

## 🔄 Giai Đoạn Thực Hiện Đề Xuất

**Tuần 1–2**: Giai đoạn 1 (C1–C10, H1)
- Fix lexer (indentation, comment, peek, string).
- Parser type expressions cơ bản.
- Codegen string + loop bug.
- Type checker integration minimal.

**Tuần 3–4**: Giai đoạn 2 (H2, H3, H8)
- IR generation.
- Driver pipeline mới.
- Native backend cơ bản (AssemblerBackend dùng IR).

**Tuần 5–6**: Giai đoạn 3 (M1–M4)
- VM computed goto.
- Inline asm hot paths.
- Borrow checker đầy đủ.
- GC integration.

**Tuần 7–10**: Giai đoạn 4 (H4–H8, M5–M8)
- OOP full.
- Stdlib FFI.
- Import/module.
- Optimization passes.

**Tuần 11–12**: Giai đoạn 5 & 6
- Native codegen hoàn thiện.
- Self-host scripts update.
- Documentation.
- CI.

---

## 📌 Kết Luận

- **Tổng số tasks**: khoảng 80+ items (chi tiết từng dòng code).
- **Critical blockers**: 11 items (C1–C11) — cần fix trước để pipeline chạy.
- **Sau khi fix C1–C11**: compiler có thể compile simple programs có string, loop, if, và chạy trên VM.
- **Sau đó**: nối IR, type checking, native backend.

**Hành động ngay**: Bắt đầu với `Lexer::handle_indentation()` và `skip_multi_line_comment()`, sau đó đến `peek_token()` và string scan. Test với `tests/minimal.veldan` và `tests/working_test.veldan`.

---

*File này sẽ được update dần khi hoàn thành từng task.*
