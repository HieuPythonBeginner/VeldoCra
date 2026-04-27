# Changes Summary: Rename velc → veldanc, .vel → .veldan

## Files Modified

### 1. build_selfhost.sh
- Changed binary name: `velc_selfhost` → `veldanc_selfhost`
- Changed binary references: `velc` → `veldanc`
- Changed source files: `src/velc/*.vel` → `src/veldanc/*.veldan`
- Changed test file: `tests/simple_test.vel` → `tests/simple_test.veldan`
- Updated combined compiler references to use `combined.veldan`
- All comments and echo messages updated

### 2. compare_selfhost.sh
- Changed `CPP_VELC="./build/bin/velc"` → `CPP_VELC="./build/bin/veldanc"`
- Changed `SELFHOST_VELC="./build/bin/velc_selfhost"` → `SELFHOST_VELC="./build/bin/veldanc_selfhost"`
- Changed test file extensions from `.vel` to `.veldan`
- Updated all error messages and success messages

### 3. test_selfhost.sh
- Changed binary check from `velc` to `veldanc`
- Changed source files: `src/velc/*.vel` → `src/veldanc/*.veldan`
- Changed test file: `tests/simple_test.vel` → `tests/simple_test.veldan`
- Changed self-host binary from `velc_selfhost` to `veldanc_selfhost`
- All echo messages and comments updated

### 4. test_full_selfhost.sh
- Changed title from "VeldoCra" → "Veldanava"

### 5. ISSUES.md (new)
- Comprehensive issue tracking document
- Lists all critical blockers (11 items)
- Phase-based task organization
- Status tracking

### 6. update_status.md (new)
- Summary of Phase 1 completion
- Next steps outlined

### 7. CHANGES_SUMMARY.md (this file)
- Complete list of all changes

## Unchanged Files (Already Correct)

- `CMakeLists.txt` - Already uses `veldanc` target name
- `src/driver/driver.cpp` - Already accepts both `.vel` and `.veldan` extensions
- `read.md` - Still contains "VeldoCra" references (intentional - this is historical documentation of the original task)

## Backward Compatibility Notes

The driver already has backward compatibility for file extensions (lines 359-371 in driver.cpp):
```cpp
bool has_vel = lower_path.size() >= 4 && lower_path.substr(lower_path.size() - 4) == ".vel";
bool has_velden = lower_path.size() >= 7 && lower_path.substr(lower_path.size() - 7) == ".veldan";
```

Both `.vel` and `.veldan` extensions are accepted by the compiler.

## Remaining Work

After this rename, the following critical issues remain:

1. Lexer: `peek_token()` returns next_token instead of peeking
2. Lexer: `scan_string()` loses first character
3. Parser: Type expressions not parsed
4. Parser: OOP definitions incomplete
5. TypeChecker: Not integrated in pipeline
6. IR: No AST→IR visitor
7. Codegen: String handling, unhandled nodes
8. Native: Assembler uses wrong IR
9. VM: Switch dispatch, HLT handling
10. Build: No install/test targets

See ISSUES.md for complete details.
