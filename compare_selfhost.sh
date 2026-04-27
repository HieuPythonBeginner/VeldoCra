#!/bin/bash
# ============================================================================
# Veldanava Self-Host Comparison Script
# ============================================================================
# This script compares the output of C++ veldanc and self-hosted veldanc
# ============================================================================

set -e  # Exit on error

echo "=========================================="
echo "Veldanava Self-Host Comparison"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
CPP_VELC="./build/bin/veldanc"
SELFHOST_VELC="./build/bin/veldanc_selfhost"
RESULTS_DIR="/tmp"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

mkdir -p "$RESULTS_DIR"

# Test files
TEST_FILES=(
    "tests/simple_test.veldan"
    "tests/minimal_test.veldan"
    "tests/fizzbuzz.veldan"
    "tests/fibonacci.veldan"
    "tests/arithmetic_test.veldan"
    "tests/comment_test.veldan"
    "tests/simple_if.veldan"
    "tests/string_index_test.veldan"
)

if [ -f "tests/self_host_test.veldan" ]; then
    TEST_FILES+=("tests/self_host_test.veldan")
fi

echo "Test files to compare:"
for file in "${TEST_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file"
    else
        echo "  ✗ $file (not found)"
    fi
done

echo ""

# ============================================================================
# Comparison Functions
# ============================================================================

compare_test_file() {
    local test_file="$1"
    # Handle both .vel and .veldan extensions
    if [[ "$test_file" == *.veldan ]]; then
    local test_name=$(basename "$test_file" .veldan)
    fi
    local cpp_output="$RESULTS_DIR/${test_name}_cpp_${TIMESTAMP}.txt"
    local cpp_error="$RESULTS_DIR/${test_name}_cpp_err_${TIMESTAMP}.txt"
    local self_output="$RESULTS_DIR/${test_name}_self_${TIMESTAMP}.txt"
    local self_error="$RESULTS_DIR/${test_name}_self_err_${TIMESTAMP}.txt"
    
    echo -n "Testing $test_name... "
    
    # Run with C++ veldanc
    if ! $CPP_VELC run "$test_file" > "$cpp_output" 2> "$cpp_error"; then
        echo -e "${YELLOW}SKIP (C++ veldanc failed)${NC}"
        return 2
    fi
    
    # Run with self-hosted veldanc
    if [ ! -f "$SELFHOST_VELC" ]; then
        echo -e "${YELLOW}SKIP (self-hosted veldanc not found)${NC}"
        return 1
    fi
    
    if ! $SELFHOST_VELC run "$test_file" > "$self_output" 2> "$self_error"; then
        echo -e "${YELLOW}SKIP (self-hosted veldanc failed)${NC}"
        return 3
    fi
    
    # Compare outputs
    if diff -q "$cpp_output" "$self_output" > /dev/null 2>&1; then
        echo -e "${GREEN}MATCH${NC}"
        return 0
    else
        echo -e "${RED}DIFFER${NC}"
        return 4
    fi
}

show_diff() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .veldan)
    fi
    local cpp_output="$RESULTS_DIR/${test_name}_cpp_${TIMESTAMP}.txt"
    local self_output="$RESULTS_DIR/${test_name}_self_${TIMESTAMP}.txt"
    local cpp_error="$RESULTS_DIR/${test_name}_cpp_err_${TIMESTAMP}.txt"
    local self_error="$RESULTS_DIR/${test_name}_self_err_${TIMESTAMP}.txt"
    
    echo ""
    echo "--- $test_name ---"
    echo ""
    
    # Show output diff
    if ! diff -q "$cpp_output" "$self_output" > /dev/null 2>&1; then
        echo "Output differences:"
        diff -u "$cpp_output" "$self_output" || true
        echo ""
    fi
    
    # Show error diff
    if ! diff -q "$cpp_error" "$self_error" > /dev/null 2>&1; then
        echo "Error differences:"
        diff -u "$cpp_error" "$self_error" || true
        echo ""
    fi
}

# ============================================================================
# Run Comparisons
# ============================================================================

echo "=========================================="
echo "Running Comparisons"
echo "=========================================="
echo ""

MATCH_COUNT=0
DIFF_COUNT=0
SKIP_COUNT=0
FAIL_COUNT=0

for test_file in "${TEST_FILES[@]}"; do
    if [ ! -f "$test_file" ]; then
        continue
    fi
    
    compare_test_file "$test_file"
    result=$?
    
    case $result in
        0) MATCH_COUNT=$((MATCH_COUNT + 1)) ;;
        4) DIFF_COUNT=$((DIFF_COUNT + 1)) ;;
        1|2|3) SKIP_COUNT=$((SKIP_COUNT + 1)) ;;
        *) FAIL_COUNT=$((FAIL_COUNT + 1)) ;;
    esac
done

echo ""
echo "=========================================="
echo "Summary"
echo "=========================================="
echo ""
echo "  Matches:    $MATCH_COUNT"
echo "  Diffs:      $DIFF_COUNT"
echo "  Skipped:    $SKIP_COUNT"
echo "  Failed:     $FAIL_COUNT"
echo ""

if [ $DIFF_COUNT -gt 0 ]; then
    echo "Showing first diff..."
    for test_file in "${TEST_FILES[@]}"; do
        if [ ! -f "$test_file" ]; then
            continue
        fi
        test_name=$(basename "$test_file" .veldan)
        fi
        cpp_output="$RESULTS_DIR/${test_name}_cpp_${TIMESTAMP}.txt"
        self_output="$RESULTS_DIR/${test_name}_self_${TIMESTAMP}.txt"
        
        if ! diff -q "$cpp_output" "$self_output" > /dev/null 2>&1; then
            show_diff "$test_file"
            break
        fi
    done
fi

if [ $FAIL_COUNT -gt 0 ]; then
    exit 1
elif [ $DIFF_COUNT -gt 0 ]; then
    exit 2
else
    exit 0
fi
