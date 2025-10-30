#pragma once

#include <cstddef>
#include <vector>

namespace geom {

/// A 2D point with double precision coordinates
struct Point {
    double x;
    double y;

    Point() : x(0.0), y(0.0) {}
    Point(double x_, double y_) : x(x_), y(y_) {}
};

/// A polyline represented as a sequence of points
using Polyline = std::vector<Point>;

/// Simplification algorithm selection
enum class SimplifyAlgorithm {
    AUTO,      // Automatically select best available implementation
    SCALAR,    // Baseline scalar implementation
    AVX2,      // AVX2 SIMD implementation
    AVX512,    // AVX-512 SIMD implementation
    NEON       // ARM NEON SIMD implementation
};

/**
 * Simplify a polyline using the Douglas-Peucker algorithm.
 *
 * @param input Input polyline to simplify
 * @param tolerance Maximum distance a point can be from the simplified line
 * @param algorithm Which implementation to use (default: AUTO)
 * @return Simplified polyline
 *
 * The tolerance is in the same units as the input coordinates.
 * The algorithm recursively removes points that contribute less than
 * the tolerance threshold to the shape of the line.
 *
 * Note: The first and last points are always preserved.
 */
Polyline simplify(const Polyline& input, 
                  double tolerance,
                  SimplifyAlgorithm algorithm = SimplifyAlgorithm::AUTO);

/**
 * Check which SIMD implementations are available at runtime.
 */
struct SIMDCapabilities {
    bool avx2_available;
    bool avx512_available;
    bool neon_available;
};

SIMDCapabilities get_simd_capabilities();

} // namespace geom
