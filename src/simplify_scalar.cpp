#include "geom_simd/internal/simplify_internal.h"
#include <algorithm>
#include <cmath>

namespace geom {
namespace internal {

namespace {

/**
 * Recursive Douglas-Peucker implementation.
 * 
 * @param points Input points
 * @param start Start index (inclusive)
 * @param end End index (inclusive)
 * @param tolerance Squared tolerance threshold
 * @param keep Bitmask of which points to keep
 */
void douglas_peucker_recursive(const PolylineSoA& points,
                               size_t start,
                               size_t end,
                               double tolerance_sq,
                               std::vector<bool>& keep) {
    if (end <= start + 1) {
        return;
    }
    
    auto p_start = points[start];
    auto p_end = points[end];
    
    // Find the point with maximum distance
    double max_dist_sq = 0.0;
    size_t max_idx = start;
    
    for (size_t i = start + 1; i < end; ++i) {
        double dist_sq = perpendicular_distance(
            points[i].x, points[i].y,
            p_start.x, p_start.y,
            p_end.x, p_end.y
        );
        
        if (dist_sq > max_dist_sq) {
            max_dist_sq = dist_sq;
            max_idx = i;
        }
    }
    
    // If max distance exceeds tolerance, keep the point and recurse
    if (max_dist_sq > tolerance_sq) {
        keep[max_idx] = true;
        douglas_peucker_recursive(points, start, max_idx, tolerance_sq, keep);
        douglas_peucker_recursive(points, max_idx, end, tolerance_sq, keep);
    }
}

} // anonymous namespace

PolylineSoA simplify_scalar(const PolylineSoA& input, double tolerance) {
    if (input.size() <= 2) {
        return input;
    }
    
    // Square the tolerance to avoid sqrt in distance calculations
    double tolerance_sq = tolerance * tolerance;
    
    // Mark which points to keep
    std::vector<bool> keep(input.size(), false);
    keep[0] = true;  // Always keep first point
    keep[input.size() - 1] = true;  // Always keep last point
    
    // Run the recursive algorithm
    douglas_peucker_recursive(input, 0, input.size() - 1, tolerance_sq, keep);
    
    // count how many points we're actually keeping
    size_t kept_count = std::count(keep.begin(), keep.end(), true);
    
    // Build the result
    PolylineSoA result;
    result.reserve(kept_count);  // Upper bound
    
    for (size_t i = 0; i < input.size(); ++i) {
        if (keep[i]) {
            result.push_back(input[i].x, input[i].y);
        }
    }
    
    return result;
}

} // namespace internal
} // namespace geom
