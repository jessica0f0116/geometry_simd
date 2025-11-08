#include "geom_simd/clip.h"

#ifdef HAVE_AVX512
#include <immintrin.h>
#include <cmath>

namespace geom {
namespace intersect {

/**
 * Vectorized line-edges intersection algorithm
 * 
 * @param ax1 start of segment a
 * @param ay1 start of segment a
 * @param ax2 end of segment a
 * @param ay2 end of segment a
 * @param b_vertices vector of vertices (size must be >= 9)
 * @param start_idx index into vector (for multiple iterations)
 * @param results intersection test for all 8 lanes
 */
void edge_intersect_avx512(
    double ax1, double ay1, double ax2, double ay2,
    const PolylineSoA& b_vertices,
    size_t start_idx,
    EdgeIntersection results[8]
) {
    /*
     * Vectorized edge-edge intersection:
     * Test one edge from polygon A against 8 edges from polygon B simultaneously.
     * 
     * Each edge in B is formed by consecutive vertices:
     *   Edge i: (b_vertices[start_idx+i], b_vertices[start_idx+i+1])
     * 
     * We'll broadcast edge A's coordinates and load 8 edges from B in parallel.
     */
    
    // Broadcast edge A's coordinates to all lanes
    __m512d vax1 = _mm512_set1_pd(ax1);
    __m512d vay1 = _mm512_set1_pd(ay1);
    __m512d vax2 = _mm512_set1_pd(ax2);
    __m512d vay2 = _mm512_set1_pd(ay2);
    
    // Direction vector for edge A: (dx_a, dy_a)
    __m512d dx_a = _mm512_sub_pd(vax2, vax1);
    __m512d dy_a = _mm512_sub_pd(vay2, vay1);
    
    // Load 8 edges from polygon B
    // Edge i goes from b_vertices[start_idx+i] to b_vertices[start_idx+i+1]
    __m512d bx1 = _mm512_loadu_pd(&b_vertices.x[start_idx]);
    __m512d by1 = _mm512_loadu_pd(&b_vertices.y[start_idx]);
    __m512d bx2 = _mm512_loadu_pd(&b_vertices.x[start_idx + 1]);
    __m512d by2 = _mm512_loadu_pd(&b_vertices.y[start_idx + 1]);
    
    // Direction vectors for 8 edges in B: (dx_b, dy_b)
    __m512d dx_b = _mm512_sub_pd(bx2, bx1);
    __m512d dy_b = _mm512_sub_pd(by2, by1);
    
    // Calculate denominator: (A2 - A1) × (B2 - B1) = dx_a * dy_b - dy_a * dx_b
    __m512d denominator = _mm512_fmsub_pd(dx_a, dy_b, _mm512_mul_pd(dy_a, dx_b));
    
    // Vector from A1 to B1 for each of the 8 edges
    __m512d dx_ab = _mm512_sub_pd(bx1, vax1);
    __m512d dy_ab = _mm512_sub_pd(by1, vay1);
    
    // Calculate numerators for t and u
    // numerator_t = (B1 - A1) × (B2 - B1) = dx_ab * dy_b - dy_ab * dx_b
    __m512d numerator_t = _mm512_fmsub_pd(dx_ab, dy_b, _mm512_mul_pd(dy_ab, dx_b));
    
    // numerator_u = (B1 - A1) × (A2 - A1) = dx_ab * dy_a - dy_ab * dx_a
    __m512d numerator_u = _mm512_fmsub_pd(dx_ab, dy_a, _mm512_mul_pd(dy_ab, dx_a));
    
    // Calculate t and u
    __m512d t = _mm512_div_pd(numerator_t, denominator);
    __m512d u = _mm512_div_pd(numerator_u, denominator);
    
    // Check if t and u are in valid range [0, 1]
    __m512d zero = _mm512_setzero_pd();
    __m512d one = _mm512_set1_pd(1.0);
    
    __mmask8 t_valid = _mm512_cmp_pd_mask(t, zero, _CMP_GE_OQ) & 
                       _mm512_cmp_pd_mask(t, one, _CMP_LE_OQ);
    __mmask8 u_valid = _mm512_cmp_pd_mask(u, zero, _CMP_GE_OQ) & 
                       _mm512_cmp_pd_mask(u, one, _CMP_LE_OQ);
    
    // Check for non-zero denominator (not parallel)
    __m512d epsilon = _mm512_set1_pd(1e-10);
    __m512d abs_denom = _mm512_abs_pd(denominator);
    __mmask8 not_parallel = _mm512_cmp_pd_mask(abs_denom, epsilon, _CMP_GT_OQ);
    
    // Intersection valid if all conditions met
    __mmask8 intersects = t_valid & u_valid & not_parallel;
    
    // Calculate intersection points for all 8 edges
    // ix = ax1 + t * dx_a
    // iy = ay1 + t * dy_a
    __m512d ix = _mm512_fmadd_pd(t, dx_a, vax1);
    __m512d iy = _mm512_fmadd_pd(t, dy_a, vay1);
    
    // Extract results to scalar array
    double t_array[8], u_array[8], ix_array[8], iy_array[8];
    _mm512_storeu_pd(t_array, t);
    _mm512_storeu_pd(u_array, u);
    _mm512_storeu_pd(ix_array, ix);
    _mm512_storeu_pd(iy_array, iy);
    
    // Fill results array
    for (int i = 0; i < 8; ++i) {
        if (intersects & (1 << i)) {
            results[i] = EdgeIntersection(
                true,
                t_array[i],
                u_array[i],
                ix_array[i],
                iy_array[i]
            );
        } else {
            results[i] = EdgeIntersection();  // No intersection
        }
    }
}

} // namespace intersect
} // namespace geom

#endif // HAVE_AVX512
