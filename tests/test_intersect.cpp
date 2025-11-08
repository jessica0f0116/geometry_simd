#include <gtest/gtest.h>
#include "geom_simd/clip.h"
#include <cmath>

using namespace geom;
using namespace geom::intersect;

// Helper to check if two EdgeIntersection results are approximately equal
bool edge_intersections_equal(const EdgeIntersection& a, const EdgeIntersection& b, 
                               double epsilon = 1e-6) {
    if (a.intersects != b.intersects) return false;
    if (!a.intersects) return true;  // Both false, equal
    
    return std::abs(a.t - b.t) < epsilon &&
           std::abs(a.u - b.u) < epsilon &&
           std::abs(a.x - b.x) < epsilon &&
           std::abs(a.y - b.y) < epsilon;
}

TEST(EdgeIntersectTest, SimpleIntersection) {
    // Two edges that clearly intersect at (5, 5)
    // Edge A: (0,0) to (10,10)
    // Edge B: (0,10) to (10,0)
    auto result = edge_intersect_scalar(0, 0, 10, 10, 
                                        0, 10, 10, 0);
    
    EXPECT_TRUE(result.intersects);
    EXPECT_NEAR(result.x, 5.0, 1e-6);
    EXPECT_NEAR(result.y, 5.0, 1e-6);
    EXPECT_NEAR(result.t, 0.5, 1e-6);
    EXPECT_NEAR(result.u, 0.5, 1e-6);
}

TEST(EdgeIntersectTest, NoIntersection) {
    // Parallel horizontal edges
    auto result = edge_intersect_scalar(0, 0, 10, 0,
                                        0, 5, 10, 5);
    EXPECT_FALSE(result.intersects);
    
    // Edges that would intersect if extended, but don't overlap
    result = edge_intersect_scalar(0, 0, 5, 5,
                                   6, 0, 10, 10);
    EXPECT_FALSE(result.intersects);
}

TEST(EdgeIntersectTest, TouchingAtEndpoint) {
    // Edges meet at (5, 5)
    auto result = edge_intersect_scalar(0, 0, 5, 5,
                                        5, 5, 10, 0);
    
    EXPECT_TRUE(result.intersects);
    EXPECT_NEAR(result.x, 5.0, 1e-6);
    EXPECT_NEAR(result.y, 5.0, 1e-6);
    EXPECT_NEAR(result.t, 1.0, 1e-6);  // At end of edge A
    EXPECT_NEAR(result.u, 0.0, 1e-6);  // At start of edge B
}

TEST(EdgeIntersectTest, ParallelEdges) {
    // Parallel but not collinear
    auto result = edge_intersect_scalar(0, 0, 10, 0,
                                        0, 1, 10, 1);
    EXPECT_FALSE(result.intersects);
}

TEST(EdgeIntersectTest, CollinearEdges) {
    // Overlapping collinear edges - typically returns false in most implementations
    auto result = edge_intersect_scalar(0, 0, 10, 0,
                                        5, 0, 15, 0);
    // Behavior is implementation-defined for collinear
    // Our implementation returns false (parallel check)
    EXPECT_FALSE(result.intersects);
}

TEST(EdgeIntersectTest, VerticalEdges) {
    // Vertical edge intersecting horizontal
    auto result = edge_intersect_scalar(5, 0, 5, 10,   // Vertical
                                        0, 5, 10, 5);   // Horizontal
    
    EXPECT_TRUE(result.intersects);
    EXPECT_NEAR(result.x, 5.0, 1e-6);
    EXPECT_NEAR(result.y, 5.0, 1e-6);
}

TEST(EdgeIntersectTest, TIntersection) {
    // T-shaped intersection: edge B ends at edge A
    auto result = edge_intersect_scalar(0, 0, 10, 0,   // Horizontal
                                        5, -5, 5, 0);   // Vertical ending at edge A
    
    EXPECT_TRUE(result.intersects);
    EXPECT_NEAR(result.x, 5.0, 1e-6);
    EXPECT_NEAR(result.y, 0.0, 1e-6);
}

#ifdef HAVE_AVX512

TEST(EdgeIntersectAVX512Test, ConsistencyWithScalar) {
    // Create 8 edges to test
    PolylineSoA b_vertices;
    b_vertices.x = {0, 0, 1, 2, 3, 4, 5, 6, 7};
    b_vertices.y = {10, 8, 8, 8, 8, 8, 8, 8, 8};
    
    // Test edge: (0,0) to (10,10)
    double ax1 = 0, ay1 = 0, ax2 = 10, ay2 = 10;
    
    // Get SIMD results (tests 8 edges)
    EdgeIntersection simd_results[8];
    edge_intersect_avx512(ax1, ay1, ax2, ay2, b_vertices, 0, simd_results);
    
    // Compare with scalar results
    for (int i = 0; i < 8; ++i) {
        auto scalar_result = edge_intersect_scalar(
            ax1, ay1, ax2, ay2,
            b_vertices.x[i], b_vertices.y[i],
            b_vertices.x[i+1], b_vertices.y[i+1]
        );
        
        EXPECT_TRUE(edge_intersections_equal(scalar_result, simd_results[i]))
            << "Mismatch at edge " << i;
    }
}

TEST(EdgeIntersectAVX512Test, MultipleIntersections) {
    // Create polygon edges that intersect with test edge
    PolylineSoA b_vertices;
    b_vertices.x = {0, 10, 0, 10, 15, 20, 25, 30, 35};
    b_vertices.y = {10, 0, 5, 15, 0, 0, 0, 0, 0};
    
    // Test edge: vertical line at x=5
    double ax1 = 5, ay1 = 0, ax2 = 5, ay2 = 20;
    
    EdgeIntersection results[8];
    edge_intersect_avx512(ax1, ay1, ax2, ay2, b_vertices, 0, results);
    
    // First edge (0,10)-(10,0) should intersect
    EXPECT_TRUE(results[0].intersects);
    EXPECT_NEAR(results[0].x, 5.0, 1e-6);
    
    // Second edge (10,0)-(0,5) should intersect
    EXPECT_TRUE(results[1].intersects);
}

TEST(EdgeIntersectAVX512Test, NoIntersections) {
    // All edges far from test edge
    PolylineSoA b_vertices;
    b_vertices.x = {20, 21, 22, 23, 24, 25, 26, 27, 28};
    b_vertices.y = {20, 21, 22, 23, 24, 25, 26, 27, 28};
    
    // Test edge at origin
    double ax1 = 0, ay1 = 0, ax2 = 10, ay2 = 0;
    
    EdgeIntersection results[8];
    edge_intersect_avx512(ax1, ay1, ax2, ay2, b_vertices, 0, results);
    
    // None should intersect
    for (int i = 0; i < 8; ++i) {
        EXPECT_FALSE(results[i].intersects) << "Edge " << i << " shouldn't intersect";
    }
}

TEST(EdgeIntersectAVX512Test, MixedIntersections) {
    // Some edges intersect, some don't
    PolylineSoA b_vertices;
    b_vertices.x = {0, 10, 20, 30, 10, 0, 0, 10, 40};
    b_vertices.y = {0, 10, 0, 10, 0, 10, 5, 5, 0};
    
    // Test edge: (5, 0) to (5, 10) - vertical line
    double ax1 = 5, ay1 = 0, ax2 = 5, ay2 = 10;
    
    EdgeIntersection results[8];
    edge_intersect_avx512(ax1, ay1, ax2, ay2, b_vertices, 0, results);
    
    EXPECT_TRUE(results[0].intersects);   // {(0,0)(10,10)} does cross
    EXPECT_FALSE(results[1].intersects);  // {(10,10)(20,0)} doesn't cross
    EXPECT_FALSE(results[2].intersects);   // {(20,0)(30,10)} doesn't cross
    EXPECT_FALSE(results[3].intersects);   // {(30,10)(10,0)} doesn't cross
    EXPECT_TRUE(results[4].intersects);  // {(10,0)(0,10)} does cross
    EXPECT_FALSE(results[5].intersects);  // {(0,10)(0,5)} doesn't cross
    EXPECT_TRUE(results[6].intersects);  // {(0,5)(10,5)} horizontal, does cross
    EXPECT_FALSE(results[7].intersects);  // {(10,5)(40,0)} far away, doesn't cross
}

#endif // HAVE_AVX512
