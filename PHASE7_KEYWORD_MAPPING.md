# Phase 7 Keyword Mapping

## Traditional Keywords → Phase 7 Keywords

This document maps traditional programming keywords to VeldoCra Phase 7 keywords.

---

### Declaration (Khai báo biến)

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `let` | `limit` | Khai báo biến immutable |
| `const` | `immo` | Khai báo hằng số |
| `mut` | `evolve` | Khai báo biến mutable |
| `var` | `av` | Khai báo biến tổng quát |
| `static` | `omni` | Biến tĩnh |
| `auto` | `senect` | Tự động suy luận kiểu |
| `type` | `species` | Định nghĩa kiểu mới |

---

### Flow Control (Luồng điều khiển)

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `if` | `verdict` | Câu điều kiện |
| `else` | `fail` | Nhánh else trong verdict |
| `elif` | (nested verdict) | Dùng verdict lồng nhau |
| `for` | `cycle` | Vòng lặp |
| `while` | `sustain` | Vòng lặp có điều kiện |
| `switch` | `path` | Rẽ nhánh nhiều case |
| `match` | `analysis` | Pattern matching |
| `break` | `end` | Thoát vòng lặp |
| `continue` | `skip` | Bỏ qua lần lặp hiện tại |

---

### Function (Hàm)

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `def` | `ability` | Định nghĩa hàm |
| `fn` | `feat` | Định nghĩa hàm có kiểu trả về |
| `func` | `hax` | Gọi hàm |
| `return` | `scale` | Trả về giá trị |
| `yield` | `stasis` | Tạm dừng và trả về |
| `async` | `non_linear` | Hàm bất đồng bộ |
| `await` | `wait` | Đợi bất đồng bộ |

---

### OOP (Lập trình hướng đối tượng)

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `class` | `race` | Định nghĩa lớp |
| `struct` | `avatar` | Định nghĩa cấu trúc |
| `interface` | `contract` | Định nghĩa giao diện |
| `trait` | `blessing` | Định nghĩa trait |
| `self` | `core` | Tham chiếu instance |
| `this` | `ego` | Tham chiếu instance |

---

### Memory (Quản lý bộ nhớ)

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `ptr` | `vec` | Tạo con trỏ |
| `ref` | `echo` | Tạo tham chiếu |
| `*` (dereference) | `touch` | Bỏ tham chiếu |
| `alloc` | `forge` | Cấp phát bộ nhớ |
| `calloc` | `pur` | Cấp phát và zero |
| `malloc` | `clm` | Cấp phát |
| `realloc` | `rsz` | Cấp phát lại |
| `free` | `ee` | Giải phóng bộ nhớ |
| `new` | `manifest` | Tạo object |
| `delete` | `ee_inf_layers` | Xóa object |
| `unsafe` | `forbidden` | Khối unsafe |
| `volatile` | `anomaly` | Biến volatile |
| `sizeof` | `cap` | Kích thước kiểu |

---

### System/Assembly

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `asm` | `origin` | Khối assembly |
| `mov` | `flow` | Di chuyển dữ liệu |
| `push` | `sink` | Đẩy vào stack |
| `pop` | `rise` | Lấy ra khỏi stack |
| `call` | `summon` | Gọi hàm |
| `ret` | `resoul` | Trả về |
| `jmp` | `leap` | Nhảy |
| `int` | `halt` | Ngắt |
| `syscall` | `petition` | System call |
| `in` | `inhale` | Đọc cổng |
| `out` | `exhale` | Ghi cổng |

---

### Error Handling (Xử lý lỗi)

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `try` | `clash` | Khối bắt lỗi |
| `catch` | `counter` | Xử lý lỗi |
| `throw` | `debuff` | Ném lỗi |
| `drop` | `dispel` | Giải phóng tài nguyên |
| `exit` | `end_of_canon` | Thoát chương trình |
| `assert` | `lowdiff` | Kiểm tra điều kiện |

---

### Module (Quản lý module)

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `#include` | `absorb` | Import file |
| `module` | `federation` | Định nghĩa module |
| `package` | `legion` | Định nghĩa package |
| `export` | `proclaim` | Xuất symbol |
| `use` | `alias` | Import với alias |
| `namespace` | `domain` | Định nghĩa namespace |

---

### Literals & Operators

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `true` | `true` | Giữ nguyên |
| `false` | `false` | Giữ nguyên |
| `null`/`nil` | `None` | Giá trị null |
| `and` | `and` | Giữ nguyên |
| `or` | `or` | Giữ nguyên |
| `not` | `not` | Giữ nguyên |

---

### Comments

| Traditional | Phase 7 | Ghi chú |
|-------------|---------|---------|
| `//` | `cmt` | Comment một dòng |
| `/* */` | `mul_cmt` | Comment nhiều dòng |

---

## Keywords đã được xóa (giờ là identifiers)

Các traditional keywords sau đã bị xóa khỏi keyword_table và giờ có thể dùng làm tên biến:
- `let`, `const`, `mut`
- `if`, `else`, `elif`, `for`, `while`
- `def`, `return`
- `class`, `struct`
- `and`, `or`, `not`
- `cmt`, `mul_cmt`

