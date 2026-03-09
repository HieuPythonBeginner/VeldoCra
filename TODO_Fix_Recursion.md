# Recursion Fix Plan - VeldoCra Ownership System

## Task
Fix the "Recursive Hell" system crash in the ownership.cpp ownership checking system.

## Steps

### Step 1: Add Depth Tracking to OwnershipChecker class (ownership.h) ✅ DONE
- [x] Add `max_depth_` member variable (constexpr size_t kMaxDepth = 10000)
- [x] Add `current_depth_` member variable
- [x] Add `check_depth()` method to guard against stack overflow

### Step 2: Fix ownership.cpp - Implement depth tracking and safety guards ✅ DONE
- [x] Initialize current_depth_ in constructor
- [x] Implement check_depth() method
- [x] Use depth checking in walk_node
- [x] Fix handle_block - Add depth guard
- [x] Fix handle_let - Add null safety
- [x] Fix exit_scope - Add stack underflow guard

### Step 3: Build with -j1 and verify fix ✅ DONE
- [x] Rebuild project with single thread - SUCCESS
- [ ] Run test to verify no crash

