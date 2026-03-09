# TODO: Ownership Checker Activation Plan

## Status: IN PROGRESS

### Step 1: Re-enable Ownership Checker in driver.cpp
- [x] Replace bypass code in run_ownership_check() with actual OwnershipChecker::check()
- [x] Remove "WARNING: Bypassed" message

### Step 2: Add missing node handlers in ownership.cpp
- [x] Add MatchStmt handler
- [x] Add BreakStmt handler
- [x] Add ContinueStmt handler
- [x] Add ConstStmt handler
- [x] Add ClassStmt handler
- [x] Add StructStmt handler
- [x] Add EnumStmt handler
- [x] Add ImportStmt handler
- [x] Add IndexExpr handler
- [x] Add MemberExpr handler
- [x] Add ArrayExpr handler
- [x] Add DictExpr handler
- [x] Add LambdaExpr handler
- [x] Add ParamDecl handler
- [x] Add FieldDecl handler
- [x] Add VariantDecl handler

### Step 3: Test the fix
- [ ] Rebuild the compiler
- [ ] Run: velc check tests/simple_test.vel
- [ ] Verify logs show manifest x and manifest y being processed

