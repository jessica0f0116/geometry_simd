#pragma once

#include "geom_simd/polygon.h"
#include "geom_simd/geom_simd.h"

namespace geom {

/**
 * Boolean operations for polygon clipping
 */
enum class ClipOperation {
    INTERSECTION,  // A ∩ B - where both polygons overlap
    UNION,         // A ∪ B - combined area of both polygons
    DIFFERENCE,    // A - B - area in A but not in B
    XOR            // A ⊕ B - area in either A or B but not both
};

/**
 * Clip two polygons using a boolean operation
 * 
 * @param subject The subject polygon (A)
 * @param clip The clip polygon (B)
 * @param op The boolean operation to perform
 * @param algorithm Which SIMD implementation to use
 * @return Vector of resulting polygons (may be empty or contain multiple polygons)
 * 
 * Note: This is a placeholder for future implementation.
 * Currently only edge intersection helpers are implemented.
 */
ClipResult clip_polygons(
    const Polygon& subject,
    const Polygon& clip,
    ClipOperation op,
    SimplifyAlgorithm algorithm = SimplifyAlgorithm::AUTO
);

namespace intersect {

/**
 * Result of an edge-edge intersection test
 */
struct EdgeIntersection {
    bool intersects;     // Do the edges intersect?
    double t;            // Parameter along first edge [0,1]
    double u;            // Parameter along second edge [0,1]
    double x, y;         // Intersection point (if intersects=true)
    
    EdgeIntersection() 
        : intersects(false), t(0), u(0), x(0), y(0) {}
    
    EdgeIntersection(bool intersects_, double t_, double u_, double x_, double y_)
        : intersects(intersects_), t(t_), u(u_), x(x_), y(y_) {}
};

/**
 * Test if two line segments intersect (scalar implementation)
 * 
 * Given edges:
 *   Edge A: from (ax1, ay1) to (ax2, ay2)
 *   Edge B: from (bx1, by1) to (bx2, by2)
 * 
 * Returns intersection info including the point if they intersect.
 * 
 * Algorithm: Solves the line-line intersection equations
 *   P = A1 + t*(A2 - A1) = B1 + u*(B2 - B1)
 * 
 * @return EdgeIntersection with intersects=true if segments intersect in [0,1]
 */
EdgeIntersection edge_intersect_scalar(
    double ax1, double ay1, double ax2, double ay2,
    double bx1, double by1, double bx2, double by2
);

#ifdef HAVE_AVX512
/**
 * Test one edge against 8 edges simultaneously (AVX-512 implementation)
 * 
 * Tests if edge A intersects with any of 8 edges from polygon B.
 * 
 * @param ax1, ay1, ax2, ay2 The single edge to test
 * @param b_vertices Vertices of polygon B (should have at least start_idx+9 vertices)
 * @param start_idx Starting index in b_vertices (will test edges [start_idx, start_idx+8))
 * @param results Output array of 8 EdgeIntersection results
 * 
 * Note: Each edge is formed by (b_vertices[i], b_vertices[i+1])
 *       So we test against edges:
 *         [start_idx -> start_idx+1]
 *         [start_idx+1 -> start_idx+2]
 *         ...
 *         [start_idx+7 -> start_idx+8]
 */
void edge_intersect_avx512(
    double ax1, double ay1, double ax2, double ay2,
    const PolylineSoA& b_vertices,
    size_t start_idx,
    EdgeIntersection results[8]
);
#endif

#ifdef HAVE_AVX2
/**
 * AVX2 version - tests one edge against 4 edges simultaneously
 */
void edge_intersect_avx2(
    double ax1, double ay1, double ax2, double ay2,
    const PolylineSoA& b_vertices,
    size_t start_idx,
    EdgeIntersection results[4]
);
#endif

#ifdef HAVE_NEON
/**
 * ARM NEON version - tests one edge against 2 edges simultaneously
 */
void edge_intersect_neon(
    double ax1, double ay1, double ax2, double ay2,
    const PolylineSoA& b_vertices,
    size_t start_idx,
    EdgeIntersection results[2]
);
#endif

/**
 * Find all intersections between two polygons
 * 
 * @param a First polygon
 * @param b Second polygon
 * @param algorithm Which SIMD implementation to use
 * @return Vector of all intersection points with edge indices
 * 
 * This will use the fastest available SIMD implementation.
 */
std::vector<EdgeIntersection> find_all_intersections(
    const Polygon& a,
    const Polygon& b,
    SimplifyAlgorithm algorithm = SimplifyAlgorithm::AUTO
);

} // namespace intersect
} // namespace geom
