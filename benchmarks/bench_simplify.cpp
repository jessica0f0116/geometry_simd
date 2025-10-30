#include <benchmark/benchmark.h>
#include "geom_simd/geom_simd.h"
#include "test_data.h"

using namespace geom;

// Benchmark fixture for different line sizes
class SimplifyFixture : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {
        size_t num_points = state.range(0);
        test_line = benchmark_data::generate_random_line(num_points);
        sine_wave = benchmark_data::generate_sine_wave(num_points);
        noisy_line = benchmark_data::generate_noisy_line(num_points);
        coastline = benchmark_data::generate_coastline(num_points);
    }
    
    Polyline test_line;
    Polyline sine_wave;
    Polyline noisy_line;
    Polyline coastline;
};

// Scalar implementation benchmarks
BENCHMARK_DEFINE_F(SimplifyFixture, Scalar_Random)(benchmark::State& state) {
    for (auto _ : state) {
        auto result = simplify(test_line, 1.0, SimplifyAlgorithm::SCALAR);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * test_line.size());
}
BENCHMARK_REGISTER_F(SimplifyFixture, Scalar_Random)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(SimplifyFixture, Scalar_SineWave)(benchmark::State& state) {
    for (auto _ : state) {
        auto result = simplify(sine_wave, 1.0, SimplifyAlgorithm::SCALAR);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * sine_wave.size());
}
BENCHMARK_REGISTER_F(SimplifyFixture, Scalar_SineWave)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(SimplifyFixture, Scalar_Noisy)(benchmark::State& state) {
    for (auto _ : state) {
        auto result = simplify(noisy_line, 1.0, SimplifyAlgorithm::SCALAR);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * noisy_line.size());
}
BENCHMARK_REGISTER_F(SimplifyFixture, Scalar_Noisy)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(SimplifyFixture, Auto_Noisy)(benchmark::State& state) {
    for (auto _ : state) {
        auto result = simplify(noisy_line, 1.0, SimplifyAlgorithm::AUTO);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * noisy_line.size());
}
BENCHMARK_REGISTER_F(SimplifyFixture, Auto_Noisy)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(SimplifyFixture, Scalar_Coastline)(benchmark::State& state) {
    for (auto _ : state) {
        auto result = simplify(coastline, 1.0, SimplifyAlgorithm::SCALAR);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * coastline.size());
}
BENCHMARK_REGISTER_F(SimplifyFixture, Scalar_Coastline)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(SimplifyFixture, Auto_Coastline)(benchmark::State& state) {
    for (auto _ : state) {
        auto result = simplify(coastline, 1.0, SimplifyAlgorithm::AUTO);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * coastline.size());
}
BENCHMARK_REGISTER_F(SimplifyFixture, Auto_Coastline)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Unit(benchmark::kMicrosecond);

// Auto implementation (will use best available SIMD)
BENCHMARK_DEFINE_F(SimplifyFixture, Auto_Random)(benchmark::State& state) {
    for (auto _ : state) {
        auto result = simplify(test_line, 1.0, SimplifyAlgorithm::AUTO);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * test_line.size());
}
BENCHMARK_REGISTER_F(SimplifyFixture, Auto_Random)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(SimplifyFixture, Auto_SineWave)(benchmark::State& state) {
    for (auto _ : state) {
        auto result = simplify(sine_wave, 1.0, SimplifyAlgorithm::AUTO);
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations() * sine_wave.size());
}
BENCHMARK_REGISTER_F(SimplifyFixture, Auto_SineWave)
    ->RangeMultiplier(4)
    ->Range(64, 16384)
    ->Unit(benchmark::kMicrosecond);

// Tolerance variation benchmarks
static void BM_SimplifyTolerance(benchmark::State& state) {
    auto line = benchmark_data::generate_random_line(1000);
    double tolerance = std::pow(10.0, -state.range(0));  // 10^-range
    
    for (auto _ : state) {
        auto result = simplify(line, tolerance);
        benchmark::DoNotOptimize(result);
    }
    state.SetLabel("tol=" + std::to_string(tolerance));
}
BENCHMARK(BM_SimplifyTolerance)
    ->DenseRange(0, 3)  // Tolerance from 1.0 to 0.001
    ->Unit(benchmark::kMicrosecond);

// Comparison benchmark showing speedup
static void BM_CompareImplementations(benchmark::State& state) {
    auto line = benchmark_data::generate_random_line(1000);
    auto algo = static_cast<SimplifyAlgorithm>(state.range(0));
    
    auto caps = get_simd_capabilities();
    
    // Skip if not available
    if (algo == SimplifyAlgorithm::AVX2 && !caps.avx2_available) {
        state.SkipWithError("AVX2 not available");
        return;
    }
    if (algo == SimplifyAlgorithm::AVX512 && !caps.avx512_available) {
        state.SkipWithError("AVX512 not available");
        return;
    }
    if (algo == SimplifyAlgorithm::NEON && !caps.neon_available) {
        state.SkipWithError("NEON not available");
        return;
    }
    
    for (auto _ : state) {
        auto result = simplify(line, 1.0, algo);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations() * line.size());
}
BENCHMARK(BM_CompareImplementations)
    ->Arg(static_cast<int>(SimplifyAlgorithm::SCALAR))
    ->Arg(static_cast<int>(SimplifyAlgorithm::AUTO))
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();