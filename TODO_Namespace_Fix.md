# TODO: Fix Namespace Issue in ast.cpp

## Problem
Lines 177-180 in ast.cpp contain corrupted code inside the `report_error()` function:
```
  /** * @brief Intern a string in the arena (allocate null-terminated copy) */ const char* ASTBuilder::intern_string(std::string_view str) const { return alloc_string(str); }
```
These lines break the namespace structure and cause compilation errors.

## Solution
Remove the 4 corrupted lines from inside the `report_error()` function.

## Steps
- [x] 1. Identify the corrupted lines in report_error()
- [ ] 2. Remove the corrupted lines (177-180)
- [ ] 3. Verify compilation succeeds

