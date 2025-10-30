# Core library sources
set(GEOM_SIMD_SOURCES
    geometry.cpp
    simplify.cpp
    simplify_scalar.cpp
)

# SIMD-specific sources with appropriate compiler flags
if(HAVE_AVX2)
    list(APPEND GEOM_SIMD_SOURCES simd/simplify_avx2.cpp)
    set_source_files_properties(simd/simplify_avx2.cpp PROPERTIES COMPILE_FLAGS "-mavx2 -mfma")
endif()

if(HAVE_AVX512)
    list(APPEND GEOM_SIMD_SOURCES simd/simplify_avx512.cpp)
    set_source_files_properties(simd/simplify_avx512.cpp PROPERTIES COMPILE_FLAGS "-mavx512f")
endif()

if(HAVE_NEON)
    list(APPEND GEOM_SIMD_SOURCES simd/simplify_neon.cpp)
endif()

# Create the library
add_library(geom_simd STATIC ${GEOM_SIMD_SOURCES})

target_include_directories(geom_simd
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
)

# Link math library on Unix
if(UNIX)
    target_link_libraries(geom_simd PUBLIC m)
endif()
