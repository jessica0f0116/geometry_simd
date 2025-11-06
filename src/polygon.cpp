#include "geom_simd/polygon.h"
#include <cmath>
#include <algorithm>

namespace geom {

bool Polygon::is_closed() const {
    if (vertices.size() < 2) return false;
    
    size_t last = vertices.size() - 1;
    double dx = vertices.x[0] - vertices.x[last];
    double dy = vertices.y[0] - vertices.y[last];
    
    return (dx * dx + dy * dy) < 1e-10;
}

double Polygon::signed_area() const {
    if (vertices.size() < 3) return 0.0;
    
    // Shoelace formula: 0.5 * sum(x[i] * y[i+1] - x[i+1] * y[i])
    double area = 0.0;
    size_t n = vertices.size();
    
    // Handle case where polygon might not be explicitly closed
    size_t limit = is_closed() ? n - 1 : n;
    
    for (size_t i = 0; i < limit; ++i) {
        size_t j = (i + 1) % n;
        area += vertices.x[i] * vertices.y[j];
        area -= vertices.x[j] * vertices.y[i];
    }
    
    return area * 0.5;
}

double Polygon::area() const {
    return std::abs(signed_area());
}

bool Polygon::contains(double px, double py) const {
    if (vertices.size() < 3) return false;
    
    // Ray casting algorithm: Cast ray from point to the right
    // Count how many times it crosses polygon edges
    // Odd count = inside, Even count = outside
    
    bool inside = false;
    size_t n = vertices.size();
    size_t limit = is_closed() ? n - 1 : n;
    
    for (size_t i = 0; i < limit; ++i) {
        size_t j = (i + 1) % n;
        
        double xi = vertices.x[i];
        double yi = vertices.y[i];
        double xj = vertices.x[j];
        double yj = vertices.y[j];
        
        // Check if ray crosses this edge
        if (((yi > py) != (yj > py)) &&
            (px < (xj - xi) * (py - yi) / (yj - yi) + xi)) {
            inside = !inside;
        }
    }
    
    return inside;
}

void Polygon::close() {
    if (is_closed()) return;
    
    vertices.x.push_back(vertices.x[0]);
    vertices.y.push_back(vertices.y[0]);
}

void Polygon::reverse() {
    std::reverse(vertices.x.begin(), vertices.x.end());
    std::reverse(vertices.y.begin(), vertices.y.end());
}

} // namespace geom
