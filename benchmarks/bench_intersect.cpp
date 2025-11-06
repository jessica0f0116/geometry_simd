#include <benchmark/benchmark.h>
#include "geom_simd/clip.h"
#include <random>

using namespace geom;
using namespace geom::intersect;

// Generate random polygon with N vertices
PolylineSoA generate_random_polygon(size_t n_vertices, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-100.0, 100.0);
    
    PolylineSoA vertices;
    vertices.x.reserve(n_vertices);
    vertices.y.reserve(n_vertices);
    
    for (size_t i = 0; i < n_vertices; ++i) {
        vertices.x.push_back(dist(rng));
        vertices.y.push_back(dist(rng));
    }
    
    return vertices;
}

// Benchmark scalar edge intersection
static void BM_EdgeIntersect_Scalar(benchmark::State& state) {
    size_t n_edges = state.range(0);
    auto poly_b = generate_random_polygon(n_edges + 1);
    
    // Test edge
    double ax1 = 0, ay1 = 0, ax2 = 50, ay2 = 50;
    
    for (auto _ : state) {
        size_t intersection_count = 0;
        
        // Test against all edges in polygon B
        for (size_t i = 0; i < n_edges; ++i) {
            auto result = edge_intersect_scalar(
                ax1, ay1, ax2, ay2,
                poly_b.x[i], poly_b.y[i],
                poly_b.x[i+1], poly_b.y[i+1]
            );
            
            if (result.intersects) {
                intersection_count++;
            }
        }
        
        benchmark::DoNotOptimize(intersection_count);
    }
    
    state.SetItemsProcessed(state.iterations() * n_edges);
}
BENCHMARK(BM_EdgeIntersect_Scalar)
    ->Arg(64)
    ->Arg(256)
    ->Arg(1024)
    ->Arg(4096)
    ->Unit(benchmark::kMicrosecond);

#ifdef HAVE_AVX512

// Benchmark AVX-512 edge intersection
static void BM_EdgeIntersect_AVX512(benchmark::State& state) {
    size_t n_edges = state.range(0);
    auto poly_b = generate_random_polygon(n_edges + 1);
    
    // Test edge
    double ax1 = 0, ay1 = 0, ax2 = 50, ay2 = 50;
    
    for (auto _ : state) {
        size_t intersection_count = 0;
        EdgeIntersection results[8];
        
        // Process 8 edges at a time
        size_t i = 0;
        for (; i + 7 < n_edges; i += 8) {
            edge_intersect_avx512(ax1, ay1, ax2, ay2, poly_b, i, results);
            
            for (int j = 0; j < 8; ++j) {
                if (results[j].intersects) {
                    intersection_count++;
                }
            }
        }
        
        // Handle remainder with scalar
        for (; i < n_edges; ++i) {
            auto result = edge_intersect_scalar(
                ax1, ay1, ax2, ay2,
                poly_b.x[i], poly_b.y[i],
                poly_b.x[i+1], poly_b.y[i+1]
            );
            
            if (result.intersects) {
                intersection_count++;
            }
        }
        
        benchmark::DoNotOptimize(intersection_count);
    }
    
    state.SetItemsProcessed(state.iterations() * n_edges);
}
BENCHMARK(BM_EdgeIntersect_AVX512)
    ->Arg(64)
    ->Arg(256)
    ->Arg(1024)
    ->Arg(4096)
    ->Unit(benchmark::kMicrosecond);

#endif // HAVE_AVX512

// Benchmark full N×M intersection finding (realistic use case)
static void BM_AllIntersections_Scalar(benchmark::State& state) {
    size_t n = state.range(0);
    auto poly_a = generate_random_polygon(n, 42);
    auto poly_b = generate_random_polygon(n, 123);
    
    for (auto _ : state) {
        size_t total_intersections = 0;
        
        // Test all edge pairs
        for (size_t i = 0; i + 1 < poly_a.size(); ++i) {
            for (size_t j = 0; j + 1 < poly_b.size(); ++j) {
                auto result = edge_intersect_scalar(
                    poly_a.x[i], poly_a.y[i],
                    poly_a.x[i+1], poly_a.y[i+1],
                    poly_b.x[j], poly_b.y[j],
                    poly_b.x[j+1], poly_b.y[j+1]
                );
                
                if (result.intersects) {
                    total_intersections++;
                }
            }
        }
        
        benchmark::DoNotOptimize(total_intersections);
    }
    
    // N edges from A × M edges from B = N×M tests
    state.SetItemsProcessed(state.iterations() * n * n);
}
BENCHMARK(BM_AllIntersections_Scalar)
    ->Arg(16)
    ->Arg(32)
    ->Arg(64)
    ->Arg(128)
    ->Unit(benchmark::kMicrosecond);

#ifdef HAVE_AVX512

static void BM_AllIntersections_AVX512(benchmark::State& state) {
    size_t n = state.range(0);
    auto poly_a = generate_random_polygon(n, 42);
    auto poly_b = generate_random_polygon(n, 123);
    
    for (auto _ : state) {
        size_t total_intersections = 0;
        EdgeIntersection results[8];
        
        // Test all edge pairs
        for (size_t i = 0; i + 1 < poly_a.size(); ++i) {
            // Process 8 edges from B at a time
            size_t j = 0;
            for (; j + 8 < poly_b.size(); j += 8) {
                edge_intersect_avx512(
                    poly_a.x[i], poly_a.y[i],
                    poly_a.x[i+1], poly_a.y[i+1],
                    poly_b, j, results
                );
                
                for (int k = 0; k < 8; ++k) {
                    if (results[k].intersects) {
                        total_intersections++;
                    }
                }
            }
            
            // Handle remainder
            for (; j + 1 < poly_b.size(); ++j) {
                auto result = edge_intersect_scalar(
                    poly_a.x[i], poly_a.y[i],
                    poly_a.x[i+1], poly_a.y[i+1],
                    poly_b.x[j], poly_b.y[j],
                    poly_b.x[j+1], poly_b.y[j+1]
                );
                
                if (result.intersects) {
                    total_intersections++;
                }
            }
        }
        
        benchmark::DoNotOptimize(total_intersections);
    }
    
    state.SetItemsProcessed(state.iterations() * n * n);
}
BENCHMARK(BM_AllIntersections_AVX512)
    ->Arg(16)
    ->Arg(32)
    ->Arg(64)
    ->Arg(128)
    ->Unit(benchmark::kMicrosecond);

#endif // HAVE_AVX512

BENCHMARK_MAIN();
