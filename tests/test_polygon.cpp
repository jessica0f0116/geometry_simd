#include <gtest/gtest.h>
#include "geom_simd/polygon.h"
#include <cmath>

using namespace geom;

class PolygonTest : public ::testing::Test {
protected:
    // Simple square (counter-clockwise)
    Polygon create_square() {
        Polygon poly;
        poly.vertices = PolylineSoA({
            {0, 0},
            {10, 0},
            {10, 10},
            {0, 10},
            {0, 0}  // Closed
        });
        return poly;
    }
    
    // Triangle (counter-clockwise)
    Polygon create_triangle() {
        Polygon poly;
        poly.vertices = PolylineSoA({
            {0, 0},
            {10, 0},
            {5, 10},
            {0, 0}  // Closed
        });
        return poly;
    }
    
    // Clockwise square
    Polygon create_square_cw() {
        Polygon poly;
        poly.vertices = PolylineSoA({
            {0, 0},
            {0, 10},
            {10, 10},
            {10, 0},
            {0, 0}  // Closed
        });
        return poly;
    }
};

TEST_F(PolygonTest, IsClosed) {
    auto square = create_square();
    EXPECT_TRUE(square.is_closed());
    
    Polygon open_poly;
    open_poly.vertices = PolylineSoA({
        {0, 0},
        {10, 0},
        {10, 10}
    });
    EXPECT_FALSE(open_poly.is_closed());
}

TEST_F(PolygonTest, SignedArea) {
    auto square = create_square();
    double area = square.signed_area();
    EXPECT_NEAR(area, 100.0, 1e-6);  // 10x10 = 100, CCW = positive
    
    auto square_cw = create_square_cw();
    area = square_cw.signed_area();
    EXPECT_NEAR(area, -100.0, 1e-6);  // CW = negative
    
    auto triangle = create_triangle();
    area = triangle.signed_area();
    EXPECT_NEAR(area, 50.0, 1e-6);  // base=10, height=10, area=50
}

TEST_F(PolygonTest, Area) {
    auto square = create_square();
    EXPECT_NEAR(square.area(), 100.0, 1e-6);
    
    auto square_cw = create_square_cw();
    EXPECT_NEAR(square_cw.area(), 100.0, 1e-6);  // Absolute value
    
    auto triangle = create_triangle();
    EXPECT_NEAR(triangle.area(), 50.0, 1e-6);
}

TEST_F(PolygonTest, IsCCW) {
    auto square = create_square();
    EXPECT_TRUE(square.is_ccw());
    
    auto square_cw = create_square_cw();
    EXPECT_FALSE(square_cw.is_ccw());
}

TEST_F(PolygonTest, ContainsPoint) {
    auto square = create_square();
    
    // Points inside
    EXPECT_TRUE(square.contains(5, 5));
    EXPECT_TRUE(square.contains(1, 1));
    EXPECT_TRUE(square.contains(9, 9));
    
    // Points outside
    EXPECT_FALSE(square.contains(-1, 5));
    EXPECT_FALSE(square.contains(11, 5));
    EXPECT_FALSE(square.contains(5, -1));
    EXPECT_FALSE(square.contains(5, 11));
    
    // Edge/corner cases (on boundary - implementation dependent)
    // Most implementations consider boundary as outside
}

TEST_F(PolygonTest, Close) {
    Polygon open_poly;
    open_poly.vertices = PolylineSoA({
        {0, 0},
        {10, 0},
        {10, 10},
        {0, 10}
    });
    
    EXPECT_FALSE(open_poly.is_closed());
    EXPECT_EQ(open_poly.size(), 4);
    
    open_poly.close();
    
    EXPECT_TRUE(open_poly.is_closed());
    EXPECT_EQ(open_poly.size(), 5);
    EXPECT_DOUBLE_EQ(open_poly.vertices.x[4], 0.0);
    EXPECT_DOUBLE_EQ(open_poly.vertices.y[4], 0.0);
    
    // Closing again should not add another vertex
    open_poly.close();
    EXPECT_EQ(open_poly.size(), 5);
}

TEST_F(PolygonTest, Reverse) {
    auto square = create_square();
    double area_before = square.signed_area();
    
    square.reverse();
    
    double area_after = square.signed_area();
    EXPECT_NEAR(area_after, -area_before, 1e-6);  // Sign flipped
    
    // First vertex should now be last
    EXPECT_DOUBLE_EQ(square.vertices.x[0], 0.0);
    EXPECT_DOUBLE_EQ(square.vertices.y[0], 0.0);
}

TEST_F(PolygonTest, EmptyPolygon) {
    Polygon empty;
    EXPECT_EQ(empty.size(), 0);
    EXPECT_DOUBLE_EQ(empty.area(), 0.0);
    EXPECT_FALSE(empty.contains(0, 0));
}

TEST_F(PolygonTest, TriangleContains) {
    auto tri = create_triangle();
    
    // Centroid should be inside
    EXPECT_TRUE(tri.contains(5, 3));
    
    // Points clearly outside
    EXPECT_FALSE(tri.contains(0, 11));
    EXPECT_FALSE(tri.contains(11, 0));
    EXPECT_FALSE(tri.contains(5, -1));
}
