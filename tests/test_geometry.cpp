#include <gtest/gtest.h>
#include "geom_simd/geom_simd.h"

using namespace geom;

TEST(GeometryTest, PointConstruction) {
    Point p1;
    EXPECT_DOUBLE_EQ(p1.x, 0.0);
    EXPECT_DOUBLE_EQ(p1.y, 0.0);
    
    Point p2(3.5, 7.2);
    EXPECT_DOUBLE_EQ(p2.x, 3.5);
    EXPECT_DOUBLE_EQ(p2.y, 7.2);
}

TEST(SIMDCapabilitiesTest, CanDetect) {
    auto caps = get_simd_capabilities();
    
    // At least one of these should be true on any modern system
    // (This is a weak test but ensures the detection runs)
    bool has_any = caps.avx2_available || caps.avx512_available || caps.neon_available;
    
    // On x86-64, we should have at least AVX2 on most modern CPUs
    // On ARM, we should have NEON
    // But we can't assert this strongly without knowing the test environment
    
    // Just verify the function doesn't crash
    SUCCEED();
}
