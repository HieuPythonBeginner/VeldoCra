# TODO: Fix Ownership Checker Recursion Hell

## Task: Implement depth_limit in OwnershipChecker::check_node and add debug logging

### Steps:
- [ ] 1. Replace all PLACEHOLDER_TO_REPLACE with actual depth_limit logic
- [ ] 2. Add debug logging to all handlers (std::cout)
- [ ] 3. Ensure graceful failure if depth limit is hit
- [ ] 4. Keep 5-Layer Defense logic intact
- [ ] 5. Rebuild and test

## Changes to ownership.cpp:
1. Implement check_depth() method properly
2. Add debug logging at start of walk_node and each handler
3. Replace all 16 PLACEHOLDER markers with proper depth checking

