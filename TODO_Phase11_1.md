# Phase 11.1: The Hybrid Awakening - TODO

## Objective
Implement optional semicolon support in VeldoCra parser - statements can end with EITHER `;` OR newline.

## Tasks

### 1. Parser Evolution (parser.cpp)
- [x] Modify `parse_simple_statement()` to optionally consume `;` after statement
- [x] Update `parse_let_statement()` to optionally consume `;`  
- [x] Update `parse_expression_statement()` to optionally consume `;`
- [x] Update `parse_return_statement()` to optionally consume `;`
- [x] Update `parse_const_statement()` to optionally consume `;`
- [x] Update `parse_function_definition()` to optionally consume `;`
- [x] Newlines are already handled by existing skip logic

### 2. Test Script Creation
- [x] Create test file WITHOUT semicolons
- [x] Create test file WITH mixed styles (semicolons and newlines)

### 3. Build and Test
- [ ] Build the project: `cd build && make`
- [ ] Test with file without semicolons
- [ ] Test with file with mixed styles

### 4. Verification
- [ ] Verify successful 'velc check' on script with NO semicolons
- [ ] Verify AST structure is correct

