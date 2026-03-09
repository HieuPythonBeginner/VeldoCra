# Phase 7 - VeldoCra Syntax Summary

## Tổng kết cú pháp các keywords VeldoCra

---

### 1. Genesis (Khai báo - Declarations)

| Keyword | Cú pháp | Tương đương |
|---------|---------|-------------|
| `av` | `av <kiểu> <tên>` | `var <type> <name>` |
| `limit` | `limit <tên> = <giá trị>` | `let <name> = <value>` |
| `senect` | `senect <tên> = <giá trị>` | `auto <name> = <value>` |
| `immo` | `immo <kiểu> <tên> = <giá trị>` | `const <type> <name> = <value>` |
| `omni` | `omni <kiểu> <tên>` | `static <type> <name>` |
| `evolve` | `evolve <tên> = <giá trị>` | `mut <name> = <value>` |
| `species` | `species <tên>` | `type <name>` - khai báo kiểu mới |

**Ghi chú:** `species` dùng để khai báo type, giúp compiler print ra biết biến đó thuộc kiểu gì.

---

### 2. Causality Control (Luồng điều khiển - Flow Control)

**Hỗ trợ cả `:` và `{}`. Nếu dùng `:` phải kết thúc bằng `;`**

| Keyword | Cú pháp `{}` | Cú pháp `:` |
|---------|--------------|-------------|
| `verdict` | `verdict <điều kiện> { ... } fail { ... }` | `verdict <điều kiện>: ... fail: ... ;` |
| `fail` | (dùng trong verdict) | (dùng trong verdict) |
| `path` | `path <biến> { case <giá trị>: ... }` | `path <biến>: case <giá trị>: ... ;` |
| `analysis` | `analysis <biến> { => <giá trị>: ... }` | `analysis <biến>: => <giá trị>: ... ;` |
| `cycle` | `cycle <tên> in <mảng> { ... }` | `cycle <tên> in <mảng>: ... ;` |
| `sustain` | `sustain <điều kiện> { ... }` | `sustain <điều kiện>: ... ;` |
| `samsara` | `samsara { ... }` | `samsara: ... ;` |
| `end` | `end` | `end` |
| `skip` | `skip` | `skip` |

**Ví dụ `:` syntax:**
```veldo
cycle i in range(5):
    print(i)
;
```

---

### 3. Abilities & Actions (Hàm - Functions)

| Keyword | Cú pháp | Tương đương |
|---------|---------|-------------|
| `ability` | `ability <tên>(<params>) { ... }` | `def <name>(<params>) { ... }` |
| `feat` | `feat <tên>(<params>) -> <kiểu> { ... }` | `fn <name>(<params>) -> <type> { ... }` |
| `hax` | `hax <tên>(<args>)` | `func <name>(<args>)` |
| `scale` | `scale <giá trị>` | `return <value>` |
| `stasis` | `stasis <giá trị>` | `yield <value>` |
| `non_linear` | `non_linear <hàm>(<args>)` | `async <func>(<args>)` |
| `wait` | `await <biểu thức>` | `await <expression>` |

---

### 4. Entity Morphology (OOP)

**Hỗ trợ cả `:` và `{}`. Tùy thích sử dụng.**

| Keyword | Cú pháp `{}` | Cú pháp `:` |
|---------|--------------|-------------|
| `race` | `race <tên> { ... }` | `race <tên>: ... ;` |
| `avatar` | `avatar <tên> { ... }` | `avatar <tên>: ... ;` |
| `grant` | `grant <tên> { ... }` | `grant <tên>: ... ;` |
| `blessing` | `blessing <tên> { ... }` | `blessing <tên>: ... ;` |
| `contract` | `contract <tên> { ... }` | `contract <tên>: ... ;` |
| `core` | `core.<biến>` | `self.<field>` |
| `ego` | `ego.<biến>` hoặc `ego.<phương thức>()` | `this.<field>` hoặc `this.<method>()` |

**Ví dụ:**
```veldo
race Person:
    ego.name = "John"
    ego.age = 30
;
```

---

### 5. Soul Manipulation (Bộ nhớ - Memory)

| Keyword | Cú pháp | Tương đương |
|---------|---------|-------------|
| `vec` | `vec <tên>` | `ptr <name>` - tạo pointer |
| `echo` | `echo <tên>` | `ref <name>` - tạo reference |
| `touch` | `touch <biến>` | `*<variable>` - dereference |
| `forge` | `forge(<size>)` | `alloc(<size>)` - cấp phát bộ nhớ |
| `pur` | `pur(<size>)` | `calloc(<size>)` - cấp phát và zero |
| `clm` | `clm(<size>)` | `malloc(<size>)` - cấp phát |
| `rsz` | `rsz(<ptr>, <size>)` | `realloc(<ptr>, <size>)` - cấp phát lại |
| `ee` | `ee(<ptr>)` | `free(<ptr>)` - giải phóng bộ nhớ |
| `manifest` | `manifest <tên> = <giá trị>` | `new <name> = <value>` |
| `ee_inf_layers` | `ee_inf_layers(<ptr>)` | `delete(<ptr>)` - xóa object |
| `forbidden` | `forbidden { ... }` | `unsafe { ... }` |
| `anomaly` | `anomaly <tên>` | `volatile <name>` |
| `cap` | `cap(<biến>)` | `sizeof(<variable>)` |

**Ghi chú:** `ee` và `ee_inf_layers` dùng syntax `(<biến>)` - có dấu ngoặc đơn!

---

### 6. True Form (Hệ thống/ASM)

**Là keyword đặc biệt để active các keyword bên trong nó. Khuyến khích khai báo đầu dòng nếu sử dụng chung file.**

| Keyword | Cú pháp | Tương đương |
|---------|---------|-------------|
| `origin` | `origin { <instructions> }` | `asm { <instructions> }` |
| `flow` | `flow <src>, <dest>` | `mov <src>, <dest>` |
| `sink` | `sink <giá trị>` | `push <value>` |
| `rise` | `rise` | `pop` |
| `summon` | `summon <hàm>(<args>)` | `call <function>(<args>)` |
| `resoul` | `resoul` | `ret` |
| `leap` | `leap <label>` | `jmp <label>` |
| `halt` | `halt <số>` | `int <number>` |
| `petition` | `petition(<syscall_num>, ...)` | `syscall(<num>, ...)` |
| `inhale` | `inhale(<port>)` | `in <port>` |
| `exhale` | `exhale(<port>, <value>)` | `out <port>, <value>` |

---

### 7. Doomsday Protocols (Xử lý lỗi - Error Handling)

**Hỗ trợ cả `:` và `{}`. Tùy thích sử dụng.**

| Keyword | Cú pháp `{}` | Cú pháp `:` |
|---------|--------------|-------------|
| `clash` | `clash { ... } counter <e> { ... }` | `clash: ... counter <e>: ... ;` |
| `counter` | (dùng trong clash) | (dùng trong clash) |
| `lowdiff` | `lowdiff { ... }` | `lowdiff: ... ;` |
| `debuff` | `debuff <lỗi>` | `raise <error>` |
| `dispel` | `dispel <biến>` | `drop <variable>` |
| `end_of_canon` | `end_of_cannon <code>` | `exit(<code>)` |

---

### 8. Module & Import System

| Keyword | Cú pháp | Tương đương |
|---------|---------|-------------|
| `absorb` | `absorb "<file>"` | `#include "<file>"` |
| `federation` | `federation <tên> { ... }` | `module <name> { ... }` |
| `legion` | `legion <tên> { ... }` | `package <name> { ... }` |
| `proclaim` | `proclaim <tên>` | `export <name>` |
| `alias` | `alias <tên> as <bí danh>` | `use <name> as <alias>` |
| `domain` | `domain <tên> { ... }` | `namespace <name> { ... }` |

---

## Tổng kết syntax đặc biệt

### Keywords dùng `()` (function call syntax):
- `forge(<size>)` - alloc
- `pur(<size>)` - calloc
- `clm(<size>)` - malloc
- `rsz(<ptr>, <size>)` - realloc
- `ee(<ptr>)` - free
- `ee_inf_layers(<ptr>)` - delete

### Keywords dùng `.` (member access):
- `ego.<biến>` - this.field
- `ego.<phương thức>()` - this.method()
- `core.<biến>` - self.field

### Keywords hỗ trợ cả `:` và `{}`:
- Flow Control: `verdict`, `fail`, `path`, `analysis`, `cycle`, `sustain`, `samsara`
- OOP: `race`, `avatar`, `grant`, `blessing`, `contract`
- Error Handling: `clash`, `counter`, `lowdiff`

**Quy tắc:** Nếu dùng `:` phải kết thúc bằng `;`

