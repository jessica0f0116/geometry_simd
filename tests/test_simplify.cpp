#include <gtest/gtest.h>
#include "geom_simd/geom_simd.h"
#include <cmath>

using namespace geom;

// Helper to check if two points are approximately equal
bool points_equal(double ax, double ay, double bx, double by, double epsilon = 1e-9) {
    return std::abs(ax - bx) < epsilon && std::abs(ay - by) < epsilon;
}

// Helper to check if two polylines are equal
bool polylines_equal(const PolylineSoA& a, const PolylineSoA& b, double epsilon = 1e-9) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (!points_equal(a[i].x, a[i].y, b[i].x, b[i].y, epsilon)) return false;
    }
    return true;
}

class SimplifyTest : public ::testing::Test {
protected:
    // Test case from Douglas-Peucker paper
    PolylineSoA create_test_line() {
        return {
            {0, 0},
            {1, 0.1},
            {2, -0.1},
            {3, 5},
            {4, 6},
            {5, 7},
            {6, 8.1},
            {7, 9},
            {8, 9},
            {9, 9}
        };
    }
    
    // Simple square
    PolylineSoA create_square() {
        return {
            {0, 0},
            {0, 10},
            {10, 10},
            {10, 0},
            {0, 0}
        };
    }
    
    // Zigzag pattern
    PolylineSoA create_zigzag() {
        PolylineSoA line;
        for (int i = 0; i < 20; ++i) {
            line.push_back(static_cast<double>(i), (i % 2) * 0.5);
        }
        return line;
    }
};

TEST_F(SimplifyTest, EmptyLine) {
    PolylineSoA empty;
    auto result = simplify(empty, 1.0);
    EXPECT_TRUE(result.empty());
}

TEST_F(SimplifyTest, SinglePoint) {
    PolylineSoA single = {{1.0, 2.0}};
    auto result = simplify(single, 1.0);
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(points_equal(result[0].x, result[0].y, single[0].x, single[0].y));
}

TEST_F(SimplifyTest, TwoPoints) {
    PolylineSoA two = {{0, 0}, {10, 10}};
    auto result = simplify(two, 1.0);
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(polylines_equal(result, two));
}

TEST_F(SimplifyTest, PreservesEndpoints) {
    auto line = create_test_line();
    auto result = simplify(line, 1.0);
    
    EXPECT_GE(result.size(), 2);
    EXPECT_TRUE(points_equal(result.x.front(), result.y.front(), line.x.front(), line.y.front()));
    EXPECT_TRUE(points_equal(result.x.back(), result.y.back(), line.x.back(), line.y.back()));
}

TEST_F(SimplifyTest, LargeToleranceRemovesPoints) {
    auto line = create_test_line();
    auto result = simplify(line, 10.0);  // Large tolerance
    
    // Should remove many intermediate points
    EXPECT_LT(result.size(), line.size());
    EXPECT_GE(result.size(), 2);  // At least start and end
}

TEST_F(SimplifyTest, ZeroToleranceInvalid) {
    auto line = create_test_line();
    EXPECT_THROW(simplify(line, 0.0), std::invalid_argument);
}

TEST_F(SimplifyTest, NegativeToleranceInvalid) {
    auto line = create_test_line();
    EXPECT_THROW(simplify(line, -1.0), std::invalid_argument);
}

TEST_F(SimplifyTest, SquareWithSmallTolerance) {
    auto square = create_square();
    auto result = simplify(square, 0.1);
    
    // Should preserve the square structure
    EXPECT_EQ(result.size(), square.size());
}

TEST_F(SimplifyTest, ZigzagWithLargeTolerance) {
    auto zigzag = create_zigzag();
    auto result = simplify(zigzag, 1.0);
    
    // Large tolerance should remove most of the zigzag
    EXPECT_LT(result.size(), zigzag.size() / 2);
}

TEST_F(SimplifyTest, StraightLineUnchanged) {
    PolylineSoA straight = {{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}};
    auto result = simplify(straight, 0.01);
    
    // Should reduce to just endpoints (all intermediate points are collinear)
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(points_equal(result[0].x, result[0].y, straight.x.front(), straight.y.front()));
    EXPECT_TRUE(points_equal(result[1].y, result[1].y, straight.x.back(), straight.y.back()));
}

// Test all implementations produce consistent results
class SimplifyConsistencyTest : public SimplifyTest,
                                 public ::testing::WithParamInterface<SimplifyAlgorithm> {};

TEST_P(SimplifyConsistencyTest, ConsistentResults) {
    auto algorithm = GetParam();
    auto caps = get_simd_capabilities();
    
    // Skip if implementation not available
    if (algorithm == SimplifyAlgorithm::AVX2 && !caps.avx2_available) {
        GTEST_SKIP();
    }
    if (algorithm == SimplifyAlgorithm::AVX512 && !caps.avx512_available) {
        GTEST_SKIP();
    }
    if (algorithm == SimplifyAlgorithm::NEON && !caps.neon_available) {
        GTEST_SKIP();
    }
    
    auto line = create_test_line();
    double tolerance = 1.0;
    
    auto result_scalar = simplify(line, tolerance, SimplifyAlgorithm::SCALAR);
    auto result_tested = simplify(line, tolerance, algorithm);
    
    // Results should be identical (or at least very close due to floating point)
    EXPECT_TRUE(polylines_equal(result_scalar, result_tested, 1e-6));
}

INSTANTIATE_TEST_SUITE_P(
    AllAlgorithms,
    SimplifyConsistencyTest,
    ::testing::Values(
        SimplifyAlgorithm::SCALAR,
        SimplifyAlgorithm::AUTO
        // AVX2, AVX512, NEON will be added when implemented
    )
);

// Parameterized test for different tolerances
class SimplifyToleranceTest : public SimplifyTest,
                               public ::testing::WithParamInterface<double> {};

TEST_P(SimplifyToleranceTest, MonotonicReduction) {
    auto line = create_test_line();
    double tolerance = GetParam();
    
    auto result = simplify(line, tolerance);
    
    // Result should never be larger than input
    EXPECT_LE(result.size(), line.size());
    
    // Larger tolerance should produce fewer or equal points
    if (tolerance > 0.1) {
        auto result_small = simplify(line, 0.1);
        EXPECT_LE(result.size(), result_small.size());
    }
}

INSTANTIATE_TEST_SUITE_P(
    VariousTolerances,
    SimplifyToleranceTest,
    ::testing::Values(0.01, 0.1, 1.0, 5.0, 10.0)
);
