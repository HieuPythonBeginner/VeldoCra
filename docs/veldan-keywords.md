# Veldanava Language Keywords Mapping

This document defines the keyword mappings for the Veldanava programming language, organized by semantic categories.

## 1. HỆ KHAI BÁO (THE GENESIS)
Khởi tạo thực thể và định nghĩa sự tồn tại.

| Standard Keyword | Veldanava Keyword | Description |
|------------------|-------------------|-------------|
| var | av | Variable declaration |
| let | limit | Block-scoped variable |
| auto | senect | Type inference |
| const | immo | Immutable variable |
| static | omni | Static/storage duration |
| mut | evolve | Mutable variable |
| type | species | Type declaration |

## 2. LUỒNG MA PHÁP (CONTROL FLOW)
Điều hướng dòng chảy của chân lý.

| Standard Keyword | Veldanava Keyword | Description |
|------------------|-------------------|-------------|
| if | verdict | Conditional statement |
| else | fail | Else branch |
| else if | faildict | Else-if branch |
| switch | path | Switch statement |
| match | analysis | Pattern matching |
| for | cycle | For loop |
| while | sustain | While loop |
| loop | samsara | Infinite loop |
| break | end | Break from loop |
| continue | skip | Continue to next iteration |

## 3. KỸ NĂNG & THỦ TỤC (SKILLS & ACTIONS)
Định nghĩa ma pháp và hành động.

| Standard Keyword | Veldanava Keyword | Description |
|------------------|-------------------|-------------|
| def (default public) | ability | Function definition |
| fn (default lambda) | feat | Function definition (lambda-style) |
| func (default private) | hax | Function definition (private) |
| return | scale | Return from function |
| yield | stasis | Generator yield |
| async | non_linear | Asynchronous function |
| await | wait | Await asynchronous operation |

### Built-in Intrinsics (optimized at compile-time)
| Intrinsic | Description | Optimization |
|-----------|-------------|--------------|
| `scribe("literal")` | Print string literal | **Intrinsic** → `OP_PRINTS` (zero overhead) |
| `scribe(expr)` | Print expression (int/float/bool) | **Builtin** → CALL ID 107 (fast runtime) |

## 4. HÌNH THÁI THỰC THỂ (OOP/STRUCTURE)
Định hình cấu trúc và quan hệ giữa các chủng tộc.

| Standard Keyword | Veldanava Keyword | Description |
|------------------|-------------------|-------------|
| class | race | Class definition |
| struct | avatar | Structure definition |
| impl | grant | Implementation block |
| trait | blessing | Trait/interface definition |
| interface | contract | Interface definition |
| public | glory | Public access modifier |
| private | secrt | Private access modifier |
| self | core | Self reference |
| this | ego | This reference |

## 5. THAO TÁC LINH HỒN (MEMORY/LOW-LEVEL)
Can thiệp trực tiếp vào cốt lõi vật chất.

### Dual-Context Classification

| Standard Keyword | Veldanava Keyword | Phân loại | Mô tả kỹ thuật & Ngữ cảnh sử dụng |
| :--- | :--- | :--- | :--- |
| **ptr** | `voice` | Type Modifier | Xác định kiểu dữ liệu con trỏ. Ví dụ: `int voice p;` |
| **ref** | `Naming` | Type Wrapper | Tham chiếu đến vùng nhớ. Thường dùng trong khai báo tham số hàm. |
| **deref** | `touch` | **Operator/Func** | Mặc định là toán tử giải băm. Trong bối cảnh đặc biệt (như interop với C), có thể dùng như hàm wrapper. |
| **alloc** | `forge` | Function Call | Hàm cấp phát bộ nhớ thô từ Standard Library. |
| **calloc** | `pur` | Function Call | Hàm cấp phát vùng nhớ sạch (Zeroed). |
| **malloc** | `clm` | Function Call | Hàm cấp phát tương thích với C runtime. |
| **realloc** | `rsz` | Function Call | Hàm thay đổi kích thước vùng nhớ động. |
| **free** | `ee` | Function Call | Hàm giải phóng bộ nhớ cơ bản. |
| **new** | `manifest` | **Operator** | Cấp phát + Gọi Constructor. Trả về một Instance hoàn chỉnh. |
| **delete** | `ee_inf_layers` | **Statement/Op** | Xóa đối tượng. Có thể đứng độc lập (Statement) hoặc dùng trong biểu thức hủy (Operator). |
| **unsafe** | `forbidden` | **Block/Modifier** | Có thể dùng làm khối lệnh `{...}` hoặc bổ túc từ cho hàm `forbidden func`. |
| **volatile** | `anomaly` | Type Qualifier | Chỉ thị cho Compiler không tối ưu hóa vùng nhớ này. |

### Call Style Notes

**Memory operations:**
- **Function call style** (with parentheses): `ee(ptr)`, `forge(size)`, `pur(count, size)`, `clm(size)`, `rsz(ptr, new_size)`
- **Statement/operator style** (no parentheses): `ee_inf_layers k` (delete object), `ee_inf_layers[] arr` (delete array)
- **Operator style**: `manifest Type(args)` equivalent to `new Type(args)`

**Intrinsic I/O:**
- `scribe("literal")` → `OP_PRINTS` (intrinsic, zero overhead)
- `scribe(expr)` → CALL builtin ID 107

### Dual-Context Explanations

1. **`touch` (Operator / Function)**: Operator form `touch ptr` (gọn), function form `touch(ptr)` (compatibility).
2. **`ee_inf_layers` (Statement / Operator)**: Statement `ee_inf_layers obj;` or operator `ee_inf_layers[] arr`.
3. **`forbidden` (Block / Modifier)**: Block `forbidden { ... }` or modifier `forbidden voice p;`.
4. **`manifest` (Operator)**: `manifest Type(args)` — cấp phát + khởi tạo + constructor.

---

## 🧠 Phân tích sâu về kiến trúc (System Architecture)

### 1. Sự khác biệt giữa `touch` và hàm Dereference
Trong các ngôn ngữ như Cython, `operator.dereference` là một lời gọi hàm giả lập. Tuy nhiên, trong **Veldanava**, `touch` được định nghĩa là một **Toán tử đơn nhất (Unary Operator)**. 
- **Cấu trúc:** `val = touch voice_ptr;`
- **Assembly:** Chuyển trực tiếp thành lệnh `MOV` (X86) hoặc `LDR` (ARM), không thông qua stack frame của hàm, giúp tối ưu tốc độ tối đa.

### 2. Quản lý vòng đời với `manifest` và `ee_inf_layers`
Đây không đơn thuần là cấp phát bộ nhớ:
- **`manifest`**: Thực hiện "phơi bày" thực thể ra vùng nhớ, thiết lập bảng phương thức ảo (vtable) và gán các giá trị mặc định.
- **`ee_inf_layers`**: Cái tên này cực hay vì nó ám chỉ việc phá hủy các lớp thực thể từ ngoài vào trong, đảm bảo mọi tài nguyên con bên trong object cũng được giải phóng sạch sẽ.

### 3. Tầm quan trọng của `anomaly` (Volatile)
Khi bác code hệ thống hoặc can thiệp sâu vào thanh ghi CPU:
- Nếu không có `anomaly`, Compiler có thể "tưởng" rằng giá trị biến không đổi và giữ nó trong Thanh ghi (Register) để tăng tốc.
- `anomaly` ép CPU phải thực hiện một thao tác đọc/ghi trực tiếp từ RAM mỗi khi biến được gọi, cực kỳ quan trọng trong lập trình nhúng hoặc đa luồng (Multi-threading).

## 6. CHẾ ĐỘ "TRUE FORM" (SYSTEM/ASM)
Giao tiếp bằng ngôn ngữ thô của vũ trụ.

| Standard Keyword | Veldanava Keyword | Description |
|------------------|-------------------|-------------|
| asm | origin | Inline assembly (2 ways: block `{}` or line declaration auto-unlock keywords) |
| mov | flow | Move data |
| push | sink | Push to stack |
| pop | rise | Pop from stack |
| call | summon | Function call |
| ret | resoul | Return from function |
| jmp | leap | Jump |
| int | halt | Interrupt |
| syscall | petition | System call |
| in | inhale | Input |
| out | exhale | Output |

## 7. XỬ LÝ SỰ CỐ & KẾT THÚC (ERROR/END)
Giải quyết xung đột và tan biến.

| Standard Keyword | Veldanava Keyword | Description |
|------------------|-------------------|-------------|
| try | clash | Try block |
| catch | counter | Catch exception |
| except | counter | Except clause |
| panic | lowdiff | Panic/abort |
| raise | debuff | Raise exception |
| drop | dispel | Drop resource |
| exit | end_of_canon | Exit program |

---

*Note: This mapping is part of the Veldanava self-hosted compiler implementation.*
