# Phase 12: Semicolon Law & Segfault Elimination

## Tasks:
- [ ] 1. Fix Parser to handle Kw_Let token properly (fix category lookup)
- [ ] 2. Make semicolon mandatory in parser
- [ ] 3. Add null guards in Ownership Checker to eliminate segfaults
- [ ] 4. Configure CMakeLists for static build
- [ ] 5. Test with `velc check tests/simple_test.vel`

## Implementation Steps:
1. Fix parser.cpp - ensure Kw_Let is recognized as Declaration category
2. Add semicolon requirement in parse_let_statement(), parse_expression_statement(), etc.
3. Add comprehensive null guards in ownership.cpp walk_node()
4. Update CMakeLists.txt for static build
5. Rebuild and test

