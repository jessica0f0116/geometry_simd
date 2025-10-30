cmake_minimum_required(VERSION 3.15)
project(geom-simd VERSION 0.1.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build benchmark suite" ON)
option(ENABLE_AVX2 "Enable AVX2 optimizations" ON)
option(ENABLE_AVX512 "Enable AVX-512 optimizations" ON)
option(ENABLE_NEON "Enable ARM NEON optimizations" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Detect CPU features
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)

# Function to check SIMD support
function(check_simd_support flag_name compile_flag test_code output_var)
    set(CMAKE_REQUIRED_FLAGS "${compile_flag}")
    check_cxx_source_compiles("${test_code}" ${output_var})
    set(${output_var} ${${output_var}} PARENT_SCOPE)
endfunction()

# Check AVX2
if(ENABLE_AVX2)
    check_simd_support(
        "AVX2"
        "-mavx2"
        "#include <immintrin.h>
        int main() {
            __m256d v = _mm256_setzero_pd();
            return 0;
        }"
        HAVE_AVX2
    )
    if(HAVE_AVX2)
        message(STATUS "AVX2 support detected")
        add_compile_definitions(HAVE_AVX2)
    endif()
endif()

# Check AVX-512
if(ENABLE_AVX512)
    check_simd_support(
        "AVX512"
        "-mavx512f"
        "#include <immintrin.h>
        int main() {
            __m512d v = _mm512_setzero_pd();
            return 0;
        }"
        HAVE_AVX512
    )
    if(HAVE_AVX512)
        message(STATUS "AVX-512 support detected")
        add_compile_definitions(HAVE_AVX512)
    endif()
endif()

# Check ARM NEON
if(ENABLE_NEON AND (CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64"))
    check_simd_support(
        "NEON"
        ""
        "#include <arm_neon.h>
        int main() {
            float64x2_t v = vdupq_n_f64(0.0);
            return 0;
        }"
        HAVE_NEON
    )
    if(HAVE_NEON)
        message(STATUS "ARM NEON support detected")
        add_compile_definitions(HAVE_NEON)
    endif()
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror=return-type
    )
endif()

# Main library
add_subdirectory(src)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Benchmarks
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Export compile commands for clangd/LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
