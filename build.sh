#!/bin/bash
set -e

# Simple build script for geom-simd

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building geom-simd...${NC}"

# Parse arguments
BUILD_TYPE="Release"
CLEAN=false
RUN_TESTS=false
RUN_BENCHMARKS=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --bench)
            RUN_BENCHMARKS=true
            shift
            ;;
        --help)
            echo "Usage: ./build.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug         Build in Debug mode (default: Release)"
            echo "  --clean         Clean build directory first"
            echo "  --test          Run tests after building"
            echo "  --bench         Run benchmarks after building"
            echo "  --help          Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

# Configure
echo -e "${GREEN}Configuring CMake (${BUILD_TYPE})...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_TESTS=ON \
    -DBUILD_BENCHMARKS=ON

# Build
echo -e "${GREEN}Building...${NC}"
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
cmake --build . -j${NPROC}

echo -e "${GREEN}Build complete!${NC}"

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    echo -e "${GREEN}Running tests...${NC}"
    ctest --output-on-failure
fi

# Run benchmarks if requested
if [ "$RUN_BENCHMARKS" = true ]; then
    echo -e "${GREEN}Running benchmarks...${NC}"
    ./bin/bench_simplify --benchmark_filter="Scalar.*Random/1024$" --benchmark_min_time=0.5s
fi

echo -e "${GREEN}Done!${NC}"
echo ""
echo "To run tests:      cd build && ctest"
echo "To run benchmarks: cd build && ./bin/bench_simplify"
echo "To clean:          rm -rf build"
