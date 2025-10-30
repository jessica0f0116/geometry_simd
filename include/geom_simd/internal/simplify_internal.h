#pragma once

#include "geom_simd/geom_simd.h"

namespace geom {
namespace internal {

/**
 * Scalar baseline implementation of Douglas-Peucker simplification.
 * This is the reference implementation for correctness testing.
 */
Polyline simplify_scalar(const Polyline& input, double tolerance);

#ifdef HAVE_AVX2
/**
 * AVX2 SIMD implementation of Douglas-Peucker simplification.
 * Uses 256-bit vectors to process 4 doubles at once.
 */
Polyline simplify_avx2(const Polyline& input, double tolerance);
#endif

#ifdef HAVE_AVX512
/**
 * AVX-512 SIMD implementation of Douglas-Peucker simplification.
 * Uses 512-bit vectors to process 8 doubles at once.
 */
Polyline simplify_avx512(const Polyline& input, double tolerance);
#endif

#ifdef HAVE_NEON
/**
 * ARM NEON SIMD implementation of Douglas-Peucker simplification.
 * Uses 128-bit vectors to process 2 doubles at once.
 */
Polyline simplify_neon(const Polyline& input, double tolerance);
#endif

/**
 * Calculate perpendicular distance from a point to a line segment.
 * 
 * @param px, py Point coordinates
 * @param x1, y1 Line segment start
 * @param x2, y2 Line segment end
 * @return Perpendicular distance
 */
inline double perpendicular_distance(double px, double py,
                                     double x1, double y1,
                                     double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    
    // If the segment is effectively a point, return distance to that point
    double mag_sq = dx * dx + dy * dy;
    if (mag_sq < 1e-10) {
        double dpx = px - x1;
        double dpy = py - y1;
        return dpx * dpx + dpy * dpy;
    }
    
    // Calculate perpendicular distance using cross product
    // |cross product| / |segment length|
    double cross = (px - x1) * dy - (py - y1) * dx;
    return (cross * cross) / mag_sq;
}

// Conversion helpers for now
inline PolylineSoA to_soa(const Polyline& aos) {
    PolylineSoA soa;
    soa.x.reserve(aos.size());
    soa.y.reserve(aos.size());
    for (const auto& p : aos) {
        soa.x.push_back(p.x);
        soa.y.push_back(p.y);
    }
    return soa;
}

inline Polyline to_aos(const PolylineSoA& soa) {
    Polyline aos;
    aos.reserve(soa.size());
    for (size_t i = 0; i < soa.size(); ++i) {
        aos.push_back({soa.x[i], soa.y[i]});
    }
    return aos;
}

} // namespace internal
} // namespace geom
