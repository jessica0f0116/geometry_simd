# Fetch Google Benchmark
include(FetchContent)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

# Benchmark executable
add_executable(bench_simplify
    bench_simplify.cpp
    test_data.cpp
)

target_link_libraries(bench_simplify
    PRIVATE
        geom_simd
        benchmark::benchmark
)

target_include_directories(bench_simplify
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Set optimization flags for benchmarks
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bench_simplify PRIVATE -O3 -march=native)
elseif(MSVC)
    target_compile_options(bench_simplify PRIVATE /O2 /arch:AVX2)
endif()
