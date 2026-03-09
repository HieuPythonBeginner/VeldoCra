# AI Task Prompt: Implement Phase 7 Keywords trong VeldoCra Parser

## Context
Đây là project VeldoCra - một ngôn ngữ lập trình với keywords theo chủ đề. Phase 7 đã định nghĩa các keywords nhưng **CHƯA được implement đầy đủ trong parser**.

## Tình trạng hiện tại
- ✅ Đã implement: `manifest` (variable declaration)
- ❌ Chưa implement: `ego`, `core`, `ee`, `ee_inf_layers`, `vec`, `echo`, `forge`, `pur`, `clm`, `rsz`, và các keywords OOP khác
- Parser hiện tại xử lý keywords này như **Identifier** thông thường

## CÚ PHÁP CHÍNH XÁC (QUAN TRỌNG!)

### 1. OOP Keywords - Dùng `.`:
```
ego.<biến>         → this.field (vd: ego.name)
ego.<method>()     → this.method() (vd: ego.getName())
core.<biến>        → self.field (vd: core.value)
```

### 2. Memory Keywords - CÓ ngoặc đơn (đặc thù riêng!):
```
forge(<size>)       → alloc: forge(100)
pur(<size>)         → calloc: pur(100)
clm(<size>)         → malloc: clm(100)
rsz(<ptr>, <size>)  → realloc: rsz(ptr, 200)
ee <biến>           → free <biến> (KHÔNG có ngoặc!)
ee_inf_layers <biến> → delete <biến> (KHÔNG có ngoặc!)
vec <tên>           → tạo pointer
echo <tên>          → tạo reference  
touch <biến>        → dereference
```

### 3. Genesis (Khai báo) - Có type annotation:
```
av <kiểu> <tên> = <giá trị>           → var <type> <name> = <value>
limit <kiểu> <tên> = <giá trị>        → let <type> <name> = <value>
immo <kiểu> <tên> = <giá trị>         → const <type> <name> = <value>
species <kiểu>                         → khai báo type (giúp print ra biết kiểu)
senect <tên> = <giá trị>              → auto (tự động xác định kiểu)
evolve <tên> = <giá trị>              → mut (biến mutable, có thể thay đổi)
```

### 4. Flow Control - HỖ TRỢ CẢ `:` VÀ `{}`:
**Quy tắc: Nếu dùng `:` phải kết thúc bằng `;`**

Dùng `{}`:
```veldo
verdict x > 0 {
    print("positive")
} fail {
    print("negative")
}
```

Dùng `:` (PHẢI có `;` cuối):
```veldo
verdict x > 0:
    print("positive")
fail:
    print("negative")
;

cycle i in range(5):
    print(i)
;
```

### 5. OOP Definitions - HỖ TRỢ CẢ `:` VÀ `{}`:
```veldo
// Dùng {}
race Person {
    ego.name = "John"
}

// Dùng :
race Person:
    ego.name = "John"
    ego.age = 30
;

// Tương tự cho:
avatar <tên>    → struct
grant <tên>     → impl  
blessing <tên>  → trait
```

### 6. origin - Keyword đặc biệt (2 cách dùng):

**Cách 1: Dùng riêng - tự động active tất cả keywords bên trong:**
```
origin  // Gõ mỗi keyword này -> tự động active toàn bộ ASM keywords trong file
flow x, y     // Tự hiểu là ASM instruction
sink 10       // Tự hiểu là ASM instruction
rise          // Tự hiểu là ASM instruction
```

**Cách 2: Dùng với block - giới hạn scope:**
```
origin { 
    flow x, y     // Chỉ active khi trong block origin
    sink 10 
}
origin {
    flow a, b
    rise
}
```

### 7. Error Handling - HỖ TRỢ CẢ `:` VÀ `{}`:
```veldo
// Dùng {}
clash {
    // try
} counter e {
    // catch
}

// Dùng :
clash:
    // try
counter e:
    // catch
;
```

## Nhiệm vụ cần làm

### 1. Cập nhật parser xử lý:
- `ego` và `core` - member access với `.`
- `ee` và `ee_inf_layers` - KHÔNG có ngoặc đơn
- `forge`, `pur`, `clm`, `rsz` - CÓ ngoặc đơn (đặc thù)

### 2. Hybrid `:` và `{}`:
- Flow: `verdict`, `fail`, `path`, `analysis`, `cycle`, `sustain`, `samsara`
- OOP: `race`, `avatar`, `grant`, `blessing`, `contract`
- Error: `clash`, `counter`, `lowdiff`
- **Quy tắc: Dùng `:` phải kết thúc bằng `;`**

### 3. origin keyword:
- Cách 1: `origin` đứng riêng -> active toàn bộ ASM keywords trong file
- Cách 2: `origin { }` -> chỉ active trong block đó

### 4. Type annotations:
- `av <kiểu> <tên> = <giá trị>`
- `limit <kiểu> <tên> = <giá trị>`
- `immo <kiểu> <tên> = <giá trị>`
- `species <kiểu>`

## Files cần sửa:
- `src/frontend/parser/parser.cpp`
- `src/frontend/parser/parser.h`
- `src/frontend/ast/ast.h` (nếu cần thêm AST nodes)

## Test sau khi implement:
```veldo
// Test OOP
ego.name = "Test"
core.value = 10

// Test Memory - CÓ ngoặc!
forge(100)
pur(50)
clm(25)
rsz(ptr, 200)

// Test Memory - KHÔNG có ngoặc!
ee ptr
ee_inf_layers obj

// Test Hybrid Flow  
verdict x > 0:
    print("positive")
fail:
    print("negative")
;

cycle i in range(5):
    print(i)
;

// Test OOP Hybrid
race Person:
    ego.name = "John"
;

// Test origin
origin
flow x, y
sink 10
rise
```

