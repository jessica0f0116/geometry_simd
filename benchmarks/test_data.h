#pragma once

#include "geom_simd/geom_simd.h"
#include <random>
#include <cmath>

namespace geom {
namespace benchmark_data {

/**
 * Generate a random polyline with given number of points
 */
inline PolylineSoA generate_random_line(size_t num_points, double scale = 100.0, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, scale);
    
    PolylineSoA line;
    line.reserve(num_points);
    
    for (size_t i = 0; i < num_points; ++i) {
        line.push_back(dist(rng), dist(rng));
    }
    
    return line;
}

/**
 * Generate a smooth sine wave line
 */
inline PolylineSoA generate_sine_wave(size_t num_points, double amplitude = 10.0, double frequency = 1.0) {
    PolylineSoA line;
    line.reserve(num_points);
    
    for (size_t i = 0; i < num_points; ++i) {
        double x = static_cast<double>(i);
        double y = amplitude * std::sin(frequency * x * 2.0 * M_PI / num_points);
        line.push_back(x, y);
    }
    
    return line;
}

/**
 * Generate a noisy line (straight line with noise)
 */
inline PolylineSoA generate_noisy_line(size_t num_points, double noise_level = 1.0, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::normal_distribution<double> noise(0.0, noise_level);
    
    PolylineSoA line;
    line.reserve(num_points);
    
    for (size_t i = 0; i < num_points; ++i) {
        double x = static_cast<double>(i);
        double y = x + noise(rng);
        line.push_back(x, y);
    }
    
    return line;
}

/**
 * Generate a complex coastline-like shape
 */
inline PolylineSoA generate_coastline(size_t num_points, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> angle_dist(-0.3, 0.3);
    std::uniform_real_distribution<double> length_dist(0.5, 2.0);
    
    PolylineSoA line;
    line.reserve(num_points);
    
    double x = 0.0, y = 0.0;
    double heading = 0.0;
    
    line.push_back(x, y);
    
    for (size_t i = 1; i < num_points; ++i) {
        heading += angle_dist(rng);
        double length = length_dist(rng);
        
        x += length * std::cos(heading);
        y += length * std::sin(heading);
        
        line.push_back(x, y);
    }
    
    return line;
}

} // namespace benchmark_data
} // namespace geom
