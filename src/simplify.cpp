#include "geom_simd/geom_simd.h"
#include "geom_simd/internal/simplify_internal.h"
#include <stdexcept>

#ifdef __x86_64__
#include <cpuid.h>
#endif

namespace geom {

namespace {

// Runtime CPU feature detection
SIMDCapabilities detect_capabilities() {
    SIMDCapabilities caps{false, false, false};
    
#ifdef __x86_64__
    unsigned int eax, ebx, ecx, edx;
    
    // Check for AVX2 (CPUID.7.0.EBX[5])
    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        caps.avx2_available = (ebx & (1 << 5)) != 0;
        caps.avx512_available = (ebx & (1 << 16)) != 0; // AVX512F
    }
#endif

#ifdef __aarch64__
    // On Apple Silicon and modern ARM, NEON is always available
    caps.neon_available = true;
#endif

    return caps;
}

} // anonymous namespace

SIMDCapabilities get_simd_capabilities() {
    static SIMDCapabilities caps = detect_capabilities();
    return caps;
}

PolylineSoA simplify(const PolylineSoA& input, 
                  double tolerance,
                  SimplifyAlgorithm algorithm) {
    // Early exit for trivial cases
    if (input.size() <= 2) {
        return input;
    }
    
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be positive");
    }
    
    // Dispatch to appropriate implementation
    if (algorithm == SimplifyAlgorithm::AUTO) {
        auto caps = get_simd_capabilities();
        
        // Prefer fastest available implementation
#ifdef HAVE_AVX512
        if (caps.avx512_available) {
            return internal::simplify_avx512(input, tolerance);
        }
#endif
#ifdef HAVE_AVX2
        if (caps.avx2_available) {
            return internal::simplify_avx2(input, tolerance);
        }
#endif
#ifdef HAVE_NEON
        if (caps.neon_available) {
            return internal::simplify_neon(input, tolerance);
        }
#endif
        // Fall back to scalar
        return internal::simplify_scalar(input, tolerance);
    }
    
    // Explicit algorithm selection
    switch (algorithm) {
        case SimplifyAlgorithm::SCALAR:
            return internal::simplify_scalar(input, tolerance);
            
#ifdef HAVE_AVX2
        case SimplifyAlgorithm::AVX2:
            if (!get_simd_capabilities().avx2_available) {
                throw std::runtime_error("AVX2 not available on this CPU");
            }
            return internal::simplify_avx2(input, tolerance);
#endif

#ifdef HAVE_AVX512
        case SimplifyAlgorithm::AVX512:
            if (!get_simd_capabilities().avx512_available) {
                throw std::runtime_error("AVX-512 not available on this CPU");
            }
            return internal::simplify_avx512(input, tolerance);
#endif

#ifdef HAVE_NEON
        case SimplifyAlgorithm::NEON:
            if (!get_simd_capabilities().neon_available) {
                throw std::runtime_error("NEON not available on this CPU");
            }
            return internal::simplify_neon(input, tolerance);
#endif

        default:
            throw std::runtime_error("Requested SIMD implementation not compiled");
    }
}

} // namespace geom
