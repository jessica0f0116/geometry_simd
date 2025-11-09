#include "geom_simd/clip.h"
#include <cmath>

namespace geom {
namespace intersect {

EdgeIntersection edge_intersect_scalar(
    const Point& a1, const Point& a2,
    const Point& b1, const Point& b2
) {
    /*
     * 
     * We want to find if two line segments intersect.
     * Represent each segment parametrically:
     *   P = A1 + t*(A2 - A1)  where t ∈ [0, 1]
     *   Q = B1 + u*(B2 - B1)  where u ∈ [0, 1]
     * 
     * At intersection: P = Q
     *   A1 + t*(A2 - A1) = B1 + u*(B2 - B1)
     * 
     * Solving for t and u using cross products:
     *   t = (B1 - A1) × (B2 - B1) / (A2 - A1) × (B2 - B1)
     *   u = (B1 - A1) × (A2 - A1) / (A2 - A1) × (B2 - B1)
     * 
     * Where × is the 2D cross product: (x1,y1) × (x2,y2) = x1*y2 - y1*x2
     * 
     * Segments intersect if both t and u are in [0, 1]
     */
    
    // Direction vectors
    double dx_a = a2.x - a1.x;  // A2 - A1
    double dy_a = a2.y - a1.y;
    double dx_b = b2.x - b1.x;  // B2 - B1
    double dy_b = b2.y - b1.y;
    
    // Cross product (A2 - A1) × (B2 - B1)
    double denominator = dx_a * dy_b - dy_a * dx_b;
    
    // If denominator is zero, lines are parallel (or collinear)
    if (std::abs(denominator) < 1e-10) {
        return EdgeIntersection();  // No intersection
    }
    
    // Vector from A1 to B1
    double dx_ab = b1.x - a1.x;  // B1 - A1
    double dy_ab = b1.y - a1.y;
    
    // Calculate t and u
    // t = (B1 - A1) × (B2 - B1) / denominator
    double numerator_t = dx_ab * dy_b - dy_ab * dx_b;
    double t = numerator_t / denominator;
    
    // u = (B1 - A1) × (A2 - A1) / denominator
    double numerator_u = dx_ab * dy_a - dy_ab * dx_a;
    double u = numerator_u / denominator;
    
    // Check if intersection point is within both segments
    if (t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0) {
        // Calculate intersection point using parameter t on edge A
        double ix = a1.x + t * dx_a;
        double iy = a1.y + t * dy_a;
        
        return EdgeIntersection(true, t, u, ix, iy);
    }
    
    return EdgeIntersection();  // No intersection within segments
}

} // namespace intersect
} // namespace geom
