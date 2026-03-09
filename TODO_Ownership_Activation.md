# TODO: Ownership Activation Plan

## Task: Re-enable and Fix Ownership Checker

### Step 1: Fix ownership.cpp - Add handlers for ALL NodeKind types
- [ ] Add handling for: ConstStmt, BreakStmt, ContinueStmt, ClassStmt, StructStmt, EnumStmt, ImportStmt, MatchStmt
- [ ] Add handling for: IndexExpr, MemberExpr, ArrayExpr, DictExpr, LambdaExpr
- [ ] Add handling for: ParamDecl, FieldDecl, VariantDecl, Decl
- [ ] Ensure all handlers have null checks

### Step 2: Re-enable ownership check in driver.cpp
- [ ] Replace bypass code in run_ownership_check() with actual OwnershipChecker invocation

### Step 3: Test and verify
- [ ] Rebuild the compiler
- [ ] Run tests/simple_test.vel
- [ ] Verify ownership checker is walking the manifest nodes

### Step 4: Verify no segfaults
- [ ] Run multiple test files
- [ ] Check for clean output

