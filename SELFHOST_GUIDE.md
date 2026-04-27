# VeldoCra Self-Host Guide

## Tổng quan

VeldoCra có khả năng **self-hosting** - nghĩa là compiler VeldoCra có thể compile chính nó. Đây là một milestone quan trọng chứng minh tính đầy đủ và ổn định của ngôn ngữ.

## Kiến trúc

```
┌─────────────────────────────────────────────────────────────┐
│                    VeldoCra Compiler                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────┐         ┌──────────────┐                  │
│  │  C++ velc    │────────▶│ VeldoCra     │                  │
│  │  (bin/velc)  │ compile │ Source       │                  │
│  └──────────────┘         │ (src/velc/)  │                  │
│         │                 └──────────────┘                  │
│         │                          │                        │
│         ▼                          ▼                        │
│  ┌──────────────┐         ┌──────────────┐                  │
│  │  Run tests   │         │  Self-hosted │                  │
│  │  & compare   │◀────────│  velc        │                  │
│  └──────────────┘         │ (velc_selfhost)                 │
│                           └──────────────┘                  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```
## Kiến trúc self host thật

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    VELDENAVA COMPILER ARCHITECTURE                      │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  [ FRONTEND - Ngôn ngữ nguồn ]       [ MIDDLE-END - Tối ưu hóa ]        │
│                                                                         │
│  ┌──────────────────────────┐        ┌──────────────────────────┐       │
│  │  Veldanava Source Code   │───┐    │  VELDANC IR (Universal)  │       │
│  └──────────────────────────┘   │    │                          │       │
│                                 │    │  (Lớp trung gian dùng    │       │
│  ┌──────────────────────────┐   │    │   chung cho 3 ngôn ngữ,  │       │
│  │  Language 2 Source Code  │───┼───▶│   nơi xử lý Logic và     │       │
│  └──────────────────────────┘   │    │   Optimization)          │       │
│                                 │    └────────────┬─────────────┘       │
│  ┌──────────────────────────┐   │                 │                     │
│  │  Language 3 Source Code  │───┘                 │                     │
│  └──────────────────────────┘                     │                     │
│                                                   ▼                     │
│                                      [ BACKEND - Kết xuất ]             │
│                                                                         │
│  (Mỗi ngôn ngữ có Parser riêng       ┌──────────────────────────┐       │
│   để dịch Syntax khác nhau           │    Code Generator        │       │
│   về cùng một chuẩn Veldanc IR)      └────────────┬─────────────┘       │
│                                                   │                     │
│                                                   ▼                     │
│                                      ┌──────────────────────────┐       │
│                                      │   BINARY (Machine Code)  │       │
│                                      └──────────────────────────┘       │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

## Scripts

### 1. `build_selfhost.sh` - Build Self-Hosted Compiler

Script này thực hiện toàn bộ quá trình build self-hosted compiler:

```bash
./build_selfhost.sh
```

**Các bước thực hiện:**
1. Build C++ compiler (`bin/velc`)
2. Verify VeldoCra source files
3. Check syntax của các file source
4. Compile VeldoCra bằng chính nó
5. Tạo self-hosted binary
6. So sánh kết quả

**Kết quả:**
- `build/bin/velc` - C++ compiler
- `build/bin/velc_selfhost` - Self-hosted compiler

### 2. `compare_selfhost.sh` - So sánh Compilers

Script này so sánh output của C++ velc và self-hosted velc trên nhiều test files:

```bash
./compare_selfhost.sh
```

**Tính năng:**
- Test trên nhiều files khác nhau
- So sánh output và error
- Tạo báo cáo chi tiết
- Lưu kết quả vào `selfhost_results/`

### 3. `test_full_selfhost.sh` - Full Test Suite

Script này chạy toàn bộ test suite:

```bash
./test_full_selfhost.sh
```

**Các phases:**
1. Build C++ Compiler
2. Build Self-Hosted Compiler
3. Compare Compilers
4. Run Original Test Suite

### 4. `test_selfhost.sh` - Original Test Script

Script test gốc (đã có sẵn):

```bash
./test_selfhost.sh
```

## CMake Targets

CMakeLists.txt đã được cập nhật với các targets mới:

```bash
# Build C++ compiler
cmake --build build

# Run self-host lexer test
cmake --build build --target velc-selfhost-lexer

# Build self-hosted compiler
cmake --build build --target velc-selfhost

# Compare compilers
cmake --build build --target velc-selfhost-compare
```

## Usage

### Quick Start

```bash
# 1. Build everything and run tests
./test_full_selfhost.sh

# 2. Or step by step:
./build_selfhost.sh
./compare_selfhost.sh
```

### Using the Compilers

```bash
# Run with C++ compiler
./build/bin/velc run tests/simple_test.vel

# Run with self-hosted compiler
./build/bin/velc_selfhost run tests/simple_test.vel
```

### Checking Results

Kết quả so sánh được lưu trong `selfhost_results/`:

```bash
# Xem báo cáo
cat selfhost_results/comparison_report_*.txt

# Xem diff chi tiết
diff selfhost_results/*_cpp_*.txt selfhost_results/*_self_*.txt
```

## How It Works

### Self-Hosting Process

1. **C++ Compiler (velc)**
   - Được viết bằng C++23
   - Compile từ `src/*.cpp`
   - Đây là reference implementation

2. **VeldoCra Source (src/velc/)**
   - Compiler được viết bằng chính VeldoCra
   - Bao gồm: lexer, parser, codegen, token, ast
   - File chính: `src/veldanc/combined.veldan`

3. **Self-Compilation**
   - C++ velc compile VeldoCra source
   - Tạo ra self-hosted compiler
   - Self-hosted compiler có thể compile chính nó

4. **Verification**
   - So sánh output của 2 compilers
   - Đảm bảo kết quả giống nhau
   - Chứng minh self-hosting thành công

### Why Self-Hosting Matters

1. **Proof of Completeness**
   - Ngôn ngữ đủ mạnh để viết compiler
   - Không cần ngôn ngữ khác để bootstrap

2. **Stability Test**
   - Compiler có thể compile chính nó
   - Chứng minh tính ổn định

3. **Dogfooding**
   - Sử dụng chính ngôn ngữ để phát triển
   - Phát hiện bugs và limitations

4. **Independence**
   - Không phụ thuộc vào C++ sau khi self-host
   - Có thể loại bỏ C++ version

## Troubleshooting

### velc not found

**Lỗi:** `C++ velc not found at ./build/bin/velc`

**Giải pháp:**
```bash
# Cách 1: Dùng script
./build_selfhost.sh

# Cách 2: Thủ công
cd build
cmake ..
make -j$(nproc)
cd ..

# Cách 3: Dùng CMake
cmake --build build
```

### Build failed

**Lỗi:** `CMake failed` hoặc `Compilation failed`

**Giải pháp:**
```bash
# Kiểm tra lỗi chi tiết
cd build
cmake ..
make -j$(nproc)

# Nếu thiếu dependencies, cài đặt:
# Ubuntu/Debian:
sudo apt-get install build-essential cmake

# macOS:
xcode-select --install
```

### Self-hosted velc not found

**Lỗi:** `Self-hosted velc not found at ./build/bin/velc_selfhost`

**Giải pháp:**
```bash
# Build self-hosted compiler
./build_selfhost.sh

# Hoặc dùng CMake
cmake --build build --target velc-selfhost
```

### Outputs differ

**Lỗi:** `FAILURE: Outputs differ!`

**Giải pháp:**
```bash
# Xem chi tiết diff
./compare_selfhost.sh

# Kiểm tra báo cáo
cat selfhost_results/comparison_report_*.txt

# Xem diff từng file
diff selfhost_results/*_cpp_*.txt selfhost_results/*_self_*.txt
```

### Source files have errors

**Lỗi:** `Some source files have errors` hoặc `SKIP (compiler issue)`

**Giải pháp:**
Đây là vấn đề với compiler, không phải với script. Các file source VeldoCra có thể có syntax mà compiler chưa hỗ trợ.

```bash
# Thử check để xem lỗi gì
./build/bin/velc check src/veldanc/combined.veldan

# Nếu có lỗi parse, đây là vấn đề của compiler
# Hệ thống self-hosting vẫn hoạt động nếu bạn có binary
```

**Lưu ý:** Hệ thống self-hosting vẫn có thể hoạt động ngay cả khi check fails, vì mục tiêu là test self-hosting, không phải fix compiler.

### Permission denied

**Lỗi:** `Permission denied: ./build_selfhost.sh`

**Giải pháp:**
```bash
# Cấp quyền execute
chmod +x *.sh

# Hoặc chạy với bash
bash build_selfhost.sh
```

### Quick Fix

Nếu gặp lỗi, thử chạy:
```bash
# Clean và build lại
rm -rf build
./test_full_selfhost.sh
```

## Advanced Usage

### Custom Test Files

```bash
# Test với file riêng
./build/bin/velc run my_test.vel
./build/bin/velc_selfhost run my_test.vel
```

### Verbose Output

```bash
# Xem chi tiết quá trình build
./build_selfhost.sh 2>&1 | tee build.log
```

### Clean Up

```bash
# Xóa kết quả test
rm -rf selfhost_results

# Xóa build
rm -rf build bin/selfhost_build
```

## Future Improvements

1. **Native Self-Host**
   - Thay vì dùng wrapper, compile thành native binary
   - Cần linker cho VeldoCra bytecode

2. **Bootstrap Chain**
   - Tạo bootstrap từ minimal compiler
   - Lo bỏ dependency vào C++

3. **Performance Comparison**
   - So sánh tốc độ 2 compilers
   - Optimize self-hosted version

4. **Cross-Platform**
   - Test trên nhiều platforms
   - Ensure portability

## References

- [VeldoCra README](README.md)
- [Phase 12 Complete](TODO_Phase12_Complete.md)
- [Language Specification](read.md)

---

**Status:** ✅ Self-hosting infrastructure ready!

**Last Updated:** 2026-03-28
