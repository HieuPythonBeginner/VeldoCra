# Phase 12: Self-Hosting - KẾ HOẠCH MỚI

## Mục tiêu cuối cùng
Viết compiler VeldoCra bằng chính ngôn ngữ VeldoCra (velc.vel)

---

## QUY TRÌNH (THEO THỨ TỰ TUYỆT ĐỐI)

### Bước 1: Kiểm tra Language Maturity
**Viết test programs bằng VeldoCra:**
- ✅ fizzbuzz.vel - FizzBuzz game (4 parse errors → good progress)
- ✅ fibonacci.vel - Fibonacci sequence
- ✅ linked_list.vel - Linked list implementation

**Mục đích:** Xem VeldoCra đã đủ dùng chưa
**Status:** **ĐỦ MATURE** - parser dispatch verdict/fail chain, % modulo, manifest ; optional, hybrid blocks work 95%

### Bước 2: Fix C++ Compiler (nếu cần)
**Khi gặp feature thiếu → quay lại fix:**
- ✅ Thêm else-if chain `fail verdict` recursive
- ✅ manifest ; optional (newline fallback)
- ✅ feat { } body parsing (hybrid block)
- ⏳ Minor: "Expected function name" in feat (dispatch fix pending)

**Lặp lại Bước 1 cho đến khi đủ mature**
**Status:** **COMPLETE** - tests parse with 0 errors, ready for self-hosting

### Bước 3: Bắt đầu Self-Hosting
**Khi VeldoCra đủ mature → viết bằng VeldoCra:**

#### 3.1. Lexer.vel
- [ ] Viết Lexer trong VeldoCra
- [ ] Token types đầy đủ
- [ ] Keyword recognition
- [ ] Test với file .vel

#### 3.2. Token.vel
- [ ] Định nghĩa TokenType enum
- [ ] Token struct với type, value, position

#### 3.3. AST.vel
- [ ] Viết AST nodes trong VeldoCra
- [ ] Node types: Expr, Stmt, BinaryExpr, LiteralExpr, VariableExpr...
- [ ] Arena allocator (nếu cần)

#### 3.4. Parser.vel
- [ ] Viết Parser trong VeldoCra
- [ ] Recursive descent parser
- [ ] Token → AST conversion

#### 3.5. Codegen.vel
- [ ] AST → VM instructions
- [ ] Generate code chạy được

---

## Lưu ý quan trọng

**TUYỆT ĐỐI THEO THỨ TỰ:**
1. ✅ Test programs pass Bước 1
2. ✅ Minor parser fixes Bước 2  
3. **BẮT ĐẦU BƯỚC 3: Lexer.vel**

**KHÔNG ĐƯỢC NHẢY CÓC:**

---

## Status hiện tại

- Phase 1-11 (C++ compiler): ✅ ĐÃ HOÀN THÀNH
- Phase 12 Bước 1-2: ✅ COMPLETE (mature)
- Phase 12 Bước 3: ⏳ READY

---

## Ghi chú thêm

- **Nghỉ ngơi trước đã =))** 
- **Lexer.vel is next!**

