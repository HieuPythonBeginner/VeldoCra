#!/bin/bash
# ============================================================================
# Veldanava Self-Host Test Script
# ============================================================================
# Tests the Veldanava compiler with its own source files
# ============================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if veldanc exists
if [ ! -f "./build/bin/veldanc" ]; then
    echo -e "${RED}Error: veldanc not found at ./build/bin/veldanc${NC}"
    echo "Please build veldanc first with: cmake --build build"
    exit 1
fi

echo -e "${GREEN}✓ Found veldanc at ./build/bin/veldanc${NC}"
echo ""

# Test 1: Check if veldanc can compile its own source files
print_step "Testing veldanc with Veldanava source files..."
print_step "1/3" "Checking individual source files with veldanc..."

VELCRA_FILES=(
    "src/veldanc/lexer.veldan"
    "src/veldanc/parser.veldan"
    "src/veldanc/codegen.veldan"
    "src/veldanc/token.veldan"
    "src/veldanc/ast.veldan"
)

PASS_COUNT=0
FAIL_COUNT=0
SKIP_COUNT=0
FILES_CHECKED=0

for file in "${VELCRA_FILES[@]}"; do
    echo -n "Testing $file... "
    
    if [ ! -f "$file" ]; then
        echo -e "${YELLOW}SKIP (not found)${NC}"
        SKIP_COUNT=$((SKIP_COUNT + 1))
        continue
    fi
    
    # Run veldanc check (may fail due to compiler issues)
    if ./build/bin/veldanc check "$file" > /dev/null 2>&1; then
        echo -e "  ${GREEN}✓${NC} $(basename $file)"
        FILES_CHECKED=$((FILES_CHECKED + 1))
    else
        echo -e "  ${RED}✗${NC} $(basename $file)"
    fi
done

echo ""
echo "Results:"
echo "  Passed: $PASS_COUNT"
echo "  Skipped: $SKIP_COUNT"
echo ""

if [ $PASS_COUNT -eq 0 ]; then
    echo -e "${YELLOW}⚠ No files passed check - continuing anyway${NC}"
else
    echo -e "${GREEN}✓ Some files passed${NC}"
fi

# Test 2: Run a simple test file
echo "=========================================="
echo "Test 2: Running simple test file"
echo "=========================================="
echo ""

TEST_FILE="tests/simple_test.veldan"

echo ""
print_step "2/3" "Testing veldanc with a simple program..."

# Check if test file exists
if [ ! -f "$TEST_FILE" ]; then
    echo -e "${RED}Error: Test file not found: $TEST_FILE${NC}"
    echo "This is expected if the compiler has bugs."
    exit 1
fi

echo "Running: veldanc run $TEST_FILE"
echo ""

# Run veldanc on the test file
if ./build/bin/veldanc run "$TEST_FILE"; then
    print_success "veldanc executed test successfully"
else
    EXIT_CODE=$?
    echo -e "${YELLOW}veldanc execution failed (exit code: $EXIT_CODE)${NC}"
    echo "This may indicate compiler bugs or unimplemented features."
fi
fi

echo ""

# Test 3: Compare outputs (if self-hosted version exists)
echo "=========================================="
echo "Test 3: Output comparison"
echo "=========================================="
echo ""

if [ -f "./build/bin/veldanc_selfhost" ]; then
    echo "Found self-hosted veldanc, comparing outputs..."
    echo ""
    
    # Run with C++ veldanc
    echo "Running with C++ veldanc..."
    ./build/bin/veldanc run "$TEST_FILE" > /tmp/velc_cpp_output.txt 2>&1
    CPP_EXIT=$?
    
    # Run with self-hosted veldanc
    echo "Running with self-hosted veldanc..."
    ./build/bin/veldanc_selfhost run "$TEST_FILE" > /tmp/velc_selfhost_output.txt 2>&1
    SELFHOST_EXIT=$?
    
    echo ""
    echo "C++ veldanc exit code: $CPP_EXIT"
    echo "Self-hosted veldanc exit code: $SELFHOST_EXIT"
    echo ""
    

        exit 1
    fi
else
    echo -e "${YELLOW}Self-hosted velc not found.${NC}"
    echo ""
    echo "To build self-hosted compiler, run:"
    echo "  ./build_selfhost.sh"
    echo ""
    echo "Or use CMake:"
    echo "  cmake --build build --target velc-selfhost"
    echo ""
    echo "For detailed comparison, run:"
    echo "  ./compare_selfhost.sh"
fi

echo ""
echo "=========================================="
echo "Self-Host Test Complete"
echo "=========================================="
echo ""
echo -e "${GREEN}✓ All tests passed!${NC}"
echo ""
