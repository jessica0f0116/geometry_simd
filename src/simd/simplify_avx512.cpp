#include "geom_simd/internal/simplify_internal.h"
#include <immintrin.h>

namespace geom {
namespace internal {

#ifdef HAVE_AVX512

void rdpr_avx512(const PolylineSoA& points,
                               size_t start,
                               size_t end,
                               double tolerance_sq,
                               std::vector<bool>& keep) {
    // this function is potentially ~similar speed to scalar for polylines with
    // *randomly distributed* points, probably due to branch misprediction?
    // the more points that can be obviated, the less recursion, faster speedup
    if (end <= start + 1) {
        return;
    }
    
    auto p_start = points[start];
    auto p_end = points[end];
    double max_dist_sq = 0.0;
    // probably not worth it perf-wise to vectorize the max index tracking
    // since it would require an extra horizontal operation
    size_t max_idx = start;
    size_t i = start + 1;
    
    // Broadcast line segment endpoints (same for all 8 points)
    __m512d x1 = _mm512_set1_pd(p_start.x);
    __m512d y1 = _mm512_set1_pd(p_start.y);
    __m512d x2 = _mm512_set1_pd(p_end.x);
    __m512d y2 = _mm512_set1_pd(p_end.y);

    // init vector of max dist to all lanes
    __m512d max_vec = _mm512_set1_pd(0.0);
    
    // Precompute for all iterations
    __m512d dx = _mm512_sub_pd(x2, x1);
    __m512d dy = _mm512_sub_pd(y2, y1);
    // **maybe this could be more efficient, probably fine?
    __m512d mag_sq = _mm512_fmadd_pd(dx, dx, _mm512_mul_pd(dy, dy));
    
    // Hot loop
    // **refactor to separate function? cleaner
    for (; i + 7 < end; i += 8) {
        // contiguous stride 1 loads now :)
        __m512d px = _mm512_loadu_pd(&points.x[i]);
        __m512d py = _mm512_loadu_pd(&points.y[i]);

        // Calculate 8 distances in parallel
        // calculate some intermediate values
        __m512d dpx = _mm512_sub_pd(px, x1);
        __m512d dpy = _mm512_sub_pd(py, y1);
        __m512d cross1 = _mm512_mul_pd(dpx, dy);
        __m512d cross2 = _mm512_mul_pd(dpy, dx);

        // reduce intermediate products to cross product
        __m512d cross = _mm512_sub_pd(cross1, cross2);
        // calculate perpendicular distance by dividing segment length and square of cross product
        __m512d dist_sq = _mm512_div_pd(_mm512_mul_pd(cross, cross), mag_sq);
        
        // Find gt among these 8 
        __mmask8 gt_mask = _mm512_cmp_pd_mask(dist_sq, max_vec, _CMP_GT_OQ);
        // keep if gt
        max_vec = _mm512_mask_blend_pd(gt_mask, max_vec, dist_sq);
        // **this should be fine for now
        double dists[8];
        _mm512_storeu_pd(dists, dist_sq);
        for (int j = 0; j < 8; ++j) {
            if (dists[j] > max_dist_sq) {
                max_dist_sq = dists[j];
                max_idx = i + j;
            }
        }
    }

    // reduce outside the loop because it's horizontal
    max_dist_sq = _mm512_reduce_max_pd(max_vec);
    
    // Handle remainder with scalar code
    // Yes this is duplicative, maybe should refactor (make separate perpendicular_distance_outer?)
    for (; i < end; ++i) {
        // scalar version
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

    // If max distance exceeds epsilon, keep point and recurse
    if (max_dist_sq > tolerance_sq) {
        keep[max_idx] = true;
        rdpr_avx512(points, start, max_idx, tolerance_sq, keep);
        rdpr_avx512(points, max_idx, end, tolerance_sq, keep);
    }
}

PolylineSoA simplify_avx512(const PolylineSoA& input, double tolerance) {
    // wire for SoA
    // auto soa = to_soa(input);

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
    rdpr_avx512(input, 0, input.size() - 1, tolerance_sq, keep);
    
    // Build the result
    PolylineSoA result;
    // auto aos = to_aos(soa);
    result.reserve(input.size());  // Upper bound
    
    for (size_t i = 0; i < input.size(); ++i) {
        if (keep[i]) {
            result.push_back(input[i].x, input[i].y);
        }
    }
    
    return result;
}


#endif // HAVE_AVX512

} // namespace internal
} // namespace geom
