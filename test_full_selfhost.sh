#!/bin/bash
# ============================================================================
# Veldanava Full Self-Host Test Suite
# ============================================================================
# This script runs the complete self-hosting test suite
# ============================================================================

set -e  # Exit on error

echo "=========================================="
echo "VeldoCra Full Self-Host Test Suite"
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
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# ============================================================================
# Helper Functions
# ============================================================================

print_step() {
    echo -e "${BLUE}[$1]${NC} $2"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_header() {
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo ""
}

run_step() {
    local step_name="$1"
    local script="$2"
    
    print_header "$step_name"
    
    if [ -f "$script" ]; then
        chmod +x "$script"
        if ./"$script"; then
            print_success "$step_name completed"
            return 0
        else
            print_error "$step_name failed"
            return 1
        fi
    else
        print_error "Script not found: $script"
        return 1
    fi
}

# ============================================================================
# Test Phases
# ============================================================================

PHASES=(
    "Build C++ Compiler|build_step"
    "Build Self-Hosted Compiler|build_selfhost.sh"
    "Compare Compilers|compare_selfhost.sh"
    "Run Original Test Suite|test_selfhost.sh"
)

# Track results
PHASE_RESULTS=()
TOTAL_PHASES=${#PHASES[@]}
PASSED_PHASES=0
FAILED_PHASES=0

# ============================================================================
# Execute Phases
# ============================================================================

for phase in "${PHASES[@]}"; do
    IFS='|' read -r phase_name phase_script <<< "$phase"
    
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Phase: $phase_name${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    if [ "$phase_script" = "build_step" ]; then
        # Special case: build C++ compiler
        print_step "Build" "Building C++ compiler..."
        
        # Check if already built
        if [ -f "build/bin/velc" ]; then
            print_success "C++ compiler already exists"
            PHASE_RESULTS+=("$phase_name:PASS")
            PASSED_PHASES=$((PASSED_PHASES + 1))
        else
            # Try to build
            if [ ! -d "build" ]; then
                mkdir -p build
            fi
            
            cd build
            if cmake .. > /dev/null 2>&1 && cmake --build . -j$(nproc) > /dev/null 2>&1; then
                cd ..
                if [ -f "build/bin/velc" ]; then
                    print_success "C++ compiler built"
                    PHASE_RESULTS+=("$phase_name:PASS")
                    PASSED_PHASES=$((PASSED_PHASES + 1))
                else
                    print_error "C++ compiler build failed - binary not found"
                    PHASE_RESULTS+=("$phase_name:FAIL")
                    FAILED_PHASES=$((FAILED_PHASES + 1))
                fi
            else
                cd ..
                print_error "C++ compiler build failed - cmake/make error"
                print_warning "Try manually: cd build && cmake .. && make -j$(nproc)"
                PHASE_RESULTS+=("$phase_name:FAIL")
                FAILED_PHASES=$((FAILED_PHASES + 1))
            fi
        fi
    else
        # Run script
        if run_step "$phase_name" "$phase_script"; then
            PHASE_RESULTS+=("$phase_name:PASS")
            PASSED_PHASES=$((PASSED_PHASES + 1))
        else
            PHASE_RESULTS+=("$phase_name:FAIL")
            FAILED_PHASES=$((FAILED_PHASES + 1))
        fi
    fi
done

# ============================================================================
# Final Summary
# ============================================================================

print_header "Final Summary"

echo "Test Suite Results:"
echo ""
echo "  Total Phases:  $TOTAL_PHASES"
echo "  Passed:        $PASSED_PHASES"
echo "  Failed:        $FAILED_PHASES"
echo ""

echo "Phase Details:"
for result in "${PHASE_RESULTS[@]}"; do
    IFS=':' read -r phase status <<< "$result"
    if [ "$status" = "PASS" ]; then
        echo -e "  ${GREEN}✓${NC} $phase"
    else
        echo -e "  ${RED}✗${NC} $phase"
    fi
done

echo ""

if [ $FAILED_PHASES -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                                                            ║${NC}"
    echo -e "${GREEN}║          🎉 SELF-HOSTING VERIFIED! 🎉                      ║${NC}"
    echo -e "${GREEN}║                                                            ║${NC}"
    echo -e "${GREEN}║  VeldoCra can successfully compile itself!                 ║${NC}"
    echo -e "${GREEN}║  The self-hosted compiler produces identical results       ║${NC}"
    echo -e "${GREEN}║  to the C++ version.                                       ║${NC}"
    echo -e "${GREEN}║                                                            ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    exit 0
else
    echo -e "${RED}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║                                                            ║${NC}"
    echo -e "${RED}║          ❌ SELF-HOSTING FAILED ❌                         ║${NC}"
    echo -e "${RED}║                                                            ║${NC}"
    echo -e "${RED}║  Some phases failed. Please check the logs above.          ║${NC}"
    echo -e "${RED}║                                                            ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    exit 1
fi
