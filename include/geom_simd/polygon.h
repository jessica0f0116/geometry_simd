#pragma once

#include "geom_simd/geom_simd.h"
#include <vector>

namespace geom {

/**
 * A polygon represented as a closed sequence of vertices.
 * 
 * Conventions:
 * - First and last vertex should be the same (closed loop)
 * - Vertices ordered counter-clockwise for outer rings
 * - Vertices ordered clockwise for holes
 */
struct Polygon {
    PolylineSoA vertices;
    
    /**
     * Number of vertices (including closing vertex if present)
     */
    size_t size() const { return vertices.size(); }
    
    /**
     * Check if polygon is closed (first == last vertex)
     */
    bool is_closed() const;
    
    /**
     * Calculate signed area (positive = counter-clockwise, negative = clockwise)
     * Uses the shoelace formula: 0.5 * sum((x[i] * y[i+1] - x[i+1] * y[i]))
     */
    double signed_area() const;
    
    /**
     * Calculate absolute area
     */
    double area() const;
    
    /**
     * Check if polygon vertices are ordered counter-clockwise
     */
    bool is_ccw() const { return signed_area() > 0; }
    
    /**
     * Test if a point is inside the polygon using ray casting
     * 
     * @param x X-coordinate of test point
     * @param y Y-coordinate of test point
     * @return true if point is inside polygon
     */
    bool contains(double x, double y) const;
    
    /**
     * Close the polygon by adding first vertex at the end (if not already closed)
     */
    void close();
    
    /**
     * Reverse vertex order (flip orientation)
     */
    void reverse();
};

/**
 * Polygon with holes (outer boundary + zero or more holes)
 */
struct PolygonWithHoles {
    Polygon outer;
    std::vector<Polygon> holes;
};

/**
 * Result of polygon clipping - may produce multiple polygons
 */
using ClipResult = std::vector<Polygon>;

} // namespace geom
