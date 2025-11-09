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

struct PolylineSoA {
    std::vector<double> x;
    std::vector<double> y;

    // default constructor
    PolylineSoA() = default;

    // point list initializer for tests
    PolylineSoA(std::initializer_list<std::pair<double, double>> points) {
        x.reserve(points.size());
        y.reserve(points.size());
        for (const auto& [px, py] : points) {
            x.push_back(px);
            y.push_back(py);
        }
    }

    size_t size() const { return x.size(); }
    bool empty() const { return x.empty(); }
        
    void reserve(size_t n) {
        x.reserve(n);
        y.reserve(n);
    }
        
    // **maybe add encapsulation to reduce public api surface?
    // **would also make it easier to validate x and y are same size
    void push_back(double px, double py) {
        x.push_back(px);
        y.push_back(py);
    }
        
    void clear() {
        x.clear();
        y.clear();
    }
        
    struct PointView {
        double x, y;
    };
        
    PointView operator[](size_t i) const {
        // assert(i < size());
        return {x[i], y[i]};
    }
};

// A polyline represented as a sequence of points
using Polyline = std::vector<Point>; // maybe keep for now until other implementation are finised

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
PolylineSoA simplify(const PolylineSoA& input, 
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
