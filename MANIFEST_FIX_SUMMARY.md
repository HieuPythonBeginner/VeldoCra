# 🔧 Fix cho từ khóa `manifest` trong Veldanava

## Vấn đề phát hiện

Theo tài liệu `docs/veldan-keywords.md`, từ khóa `manifest` trong Veldanava ĐƯỢC DÙNG LÀM `new` (toán tử cấp phát đối tượng + gọi constructor), không phải là từ khóa khai báo biến thông thường.

**Thiết kế chính xác:**
```crystal
manifest Type(args)  // tương đương new Type(args) trong C++
```

**Sai lầm trước đây:**
Parser đang xử lý `manifest` như một statement khai báo biến:
```crystal
manifest x = 5;  // ❌ SAI - không phải cú pháp của Veldanava
```

## Những gì đã được sửa

### 1. Parser: Thêm xử lý `manifest` trong `parse_primary()`
**File**: `src/frontend/parser/parser.cpp`

Thêm xử lý khi gặp token `Kw_Manifest`:
```cpp
// manifest statement (new expression: manifest Type(args))
if (check(TokenType::Kw_Manifest)) {
    advance(); // consume "manifest"
    if (!check(TokenType::Identifier)) {
        report_error("Expected type name after manifest");
        return builder_.create_none();
    }
    const char* type_name = get_token_text(current_token());
    advance();
    
    // Check for constructor arguments
    if (check(TokenType::ParenOpen)) {
        Expr* ctor_call = parse_call(builder_.create_variable(type_name));
        return ctor_call;
    } else {
        // manifest Type - gọi constructor không đối số
        auto* ctor_call = builder_.create_call(builder_.create_variable(type_name));
        return ctor_call;
    }
}
```

**Kết quả**: `manifest MyClass(1, 2, 3)` sẽ tạo một biểu thức gọi constructor.

### 2. Cập nhật header parser
**File**: `src/frontend/parser/parser.h`

Thêm khai báo cho các hàm bị thiếu:
```cpp
veldanava::ast::BlockStmtNode* parse_block_body(veldanava::lexer::TokenType end_token = veldanava::lexer::TokenType::EndOfFile);
veldanava::ast::LetStmtNode* parse_let_statement();
```

### 3. Cập nhật namespace
**File**: `src/frontend/parser/parser.cpp`

Đổi từ `namespace veldocra` → `namespace veldanava` để khớp với header.

### 4. Fix khai báo/định nghĩa `parse_block_body`
Sửa lại signature để khớp:
```cpp
// Trong header:
BlockStmtNode* parse_block_body(veldanava::lexer::TokenType end_token = EndOfFile);

// Trong cpp:
BlockStmtNode* Parser::parse_block_body(veldanava::lexer::TokenType end_token) {
    // ...
}
```

## Kết quả

✅ **Build thành công** - không còn lỗi cú pháp  
✅ **Parser hiểu đúng cú pháp** `manifest Type(args)`  
✅ **Tạo biểu thức constructor call** thay vì statement khai báo biến  

## Test

Code ví dụ hợp lệ sau nay được parse đúng:
```crystal
# Cấp phát đối tượng mới
manifest MyClass(10, 20);   # Có đối số
manifest AnotherClass();     # Không đối số
manifest SimpleType;         # Constructor mặc định
```

## Lưu ý

- Các statement như `manifest x = 5;` là **KHÔNG Hợp lệ** trong Veldanava
- Cú pháp đúng là `manifest Type(...)` - luôn là một biểu thức trả về object
- Biến cục bộ được khai báo bằng `let`/`limit`/`const`/`av`, không dùng `manifest`
