# geom-simd

SIMD-optimized geospatial algorithms for polygon and polyline operations. I wanted to practice maintaining an actual library and learn AVX512 intrinsics just for fun, and maybe to make my fuzzers more efficient. Geometry is familiar territory. I will implement some polyline algorithms, starting with RDP first, before moving on to polygonal operations. Hopefully some are faster than GEOS (solid benchmarking is another problem to tackle).

## Project Goals

- Provide SIMD-accelerated implementations of common GIS algorithms
- Support multiple architectures: AVX2, AVX-512, ARM NEON (maybe?)
- Maintain correctness against reference implementations (GEOS)
- Learn modern C++ development practices: CI/CD, benchmarking, testing

## Current Status

🎃 Douglas-Peucker polyline simplification
- [x] Scalar baseline implementation
- [x] Unit tests with known geometries
- [x] Benchmark harness
- [ ] AVX2 implementation
- [x] AVX-512 implementation
- [ ] ARM NEON implementation
- [ ] Property tests / integration tests
- [ ] Add topology-preserving variant (Visvalingam-Whyatt is less amenable to vectorization, although we could broaden the goal to just being faster than GEOS)

🍰 Polygon clipping algos
- [x] Add Polygon type
- [ ] Implement basic poly operations (area+contains point)
- [x] Add edge intersections (scalar+AVX)
- [x] Unit tests and benchmarks for edge intersection
- [ ] Implement Sutherland-Hodgman (convex only) and vectorize core loops
- [ ] Implement Greiner-Hormann and vectorize intersection finding
- [ ] Edge cases (degenerate polys, touching edges, and so forth)

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Build Options

- `BUILD_TESTS=ON` - Build unit tests (default: ON)
- `BUILD_BENCHMARKS=ON` - Build benchmark suite (default: ON)
- `ENABLE_AVX2=ON` - Enable AVX2 optimizations (default: auto-detect)
- `ENABLE_AVX512=ON` - Enable AVX-512 optimizations (default: auto-detect)
- `ENABLE_NEON=ON` - Enable ARM NEON optimizations (default: auto-detect)

## Testing

```bash
cd build
ctest --output-on-failure
```

## Benchmarking

```bash
cd build
./benchmarks/bench_simplify
```

## Algorithm Reference

### Douglas-Peucker Simplification

Recursively simplifies polylines by removing points that deviate less than a tolerance threshold from the simplified line.

**References:**
- Original paper: Douglas & Peucker (1973)
- [GEOS implementation](https://github.com/libgeos/geos/blob/main/src/simplify/DouglasPeuckerLineSimplifier.cpp)

