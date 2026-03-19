# -------------------------------------------------------------------------------
# SIMD Feature Detection and Multi-Implementation Support
# -------------------------------------------------------------------------------

message(STATUS "SIMD Support Detection:")

# Include CheckCSourceCompiles for feature detection
include(CheckCSourceCompiles)

# ===== Portable C Implementation (Always compiled) =====
set(VL_SIMD_IMPLEMENTATIONS "portable_c")
message(STATUS "  SIMD Implementations available:")
message(STATUS "    - Portable C (always compiled)")

# ===== x86/x86-64 Feature Detection =====
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i686|x86")
    message(STATUS "  Detected x86/x86-64 architecture - checking for CPU features...")

    # Check for SSE4.2
    set(CMAKE_REQUIRED_FLAGS "-msse2")
    check_c_source_compiles("
        #include <emmintrin.h>
        int main() {
            __m128i v = _mm_set1_epi32(0);
            v = _mm_add_epi32(v, v);
            return 0;
        }"
            VL_SIMD_SSE2_AVAILABLE)
    unset(CMAKE_REQUIRED_FLAGS)

    if(VL_SIMD_SSE2_AVAILABLE)
        list(APPEND VL_SIMD_IMPLEMENTATIONS "sse2")
        message(STATUS "    - SSE2 (available)")
    else()
        message(STATUS "    - SSE2 (not available)")
    endif()

    # Check for AVX2
    set(CMAKE_REQUIRED_FLAGS "-mavx2 -mfma")
    check_c_source_compiles("
            #include <immintrin.h>
            int main() {
                __m256 v = _mm256_set1_ps(1.0f);
                v = _mm256_mul_ps(v, v);
                return 0;
            }
        " VL_SIMD_AVX2_AVAILABLE)
    unset(CMAKE_REQUIRED_FLAGS)

    if(VL_SIMD_AVX2_AVAILABLE)
        list(APPEND VL_SIMD_IMPLEMENTATIONS "avx2")
        message(STATUS "    - AVX2 (available)")
    else()
        message(STATUS "    - AVX2 (not available)")
    endif()

    # ===== ARM/ARM64 Feature Detection =====
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armv8|aarch64|arm64")
    message(STATUS "  Detected ARM architecture - checking for NEON...")

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv8|aarch64|arm64")
        # ARMv8/ARM64: NEON is mandatory
        set(VL_SIMD_NEON64_AVAILABLE TRUE)
        list(APPEND VL_SIMD_IMPLEMENTATIONS "neon64")
        message(STATUS "    - NEON64 (mandatory on ARMv8)")
    else()
        # ARMv7: NEON is optional
        set(CMAKE_REQUIRED_FLAGS "-mfpu=neon")
        check_c_source_compiles("
                #include <arm_neon.h>
                int main() {
                    float32x4_t v = vdupq_n_f32(1.0f);
                    v = vmulq_f32(v, v);
                    return 0;
                }
            " VL_SIMD_NEON_AVAILABLE)
        unset(CMAKE_REQUIRED_FLAGS)

        if(VL_SIMD_NEON_AVAILABLE)
            list(APPEND VL_SIMD_IMPLEMENTATIONS "neon")
            message(STATUS "    - NEON (available)")
        else()
            message(STATUS "    - NEON (not available)")
        endif()
    endif()
else()
    message(STATUS "  Unknown processor (${CMAKE_SYSTEM_PROCESSOR}) - using portable C only")
endif()

message(STATUS "  Total implementations to compile: ${VL_SIMD_IMPLEMENTATIONS}")

# Store the list for use in component configuration
set(VL_SIMD_IMPLEMENTATIONS ${VL_SIMD_IMPLEMENTATIONS} CACHE INTERNAL "Available SIMD implementations")

# ===== SIMD Implementation Helper =====
function(vl_add_simd_implementation impl_name source_file)
    add_library(vl_simd_${impl_name}_obj OBJECT ${source_file})

    # Set compiler flags based on implementation and compiler
    if(MSVC)
        if(impl_name STREQUAL "sse2")
            set(flags /arch:SSE2)
        elseif(impl_name STREQUAL "avx2")
            set(flags /arch:AVX2)
        elseif(impl_name STREQUAL "neon")
            set(flags /arch:ARMV7)
        else()
            set(flags "")  # neon64 and unknown
        endif()
    else()
        if(impl_name STREQUAL "sse2")
            set(flags -msse2)
        elseif(impl_name STREQUAL "avx2")
            set(flags -mavx2 -mfma)
        elseif(impl_name STREQUAL "neon")
            set(flags -mfpu=neon -mfloat-abi=hard)
        elseif(impl_name STREQUAL "neon64")
            set(flags -march=armv8-a+simd)
        else()
            set(flags "")
        endif()
    endif()

    # Apply flags if any
    if(flags)
        target_compile_options(vl_simd_${impl_name}_obj PRIVATE ${flags})
    endif()

    # Add include directories
    target_include_directories(vl_simd_${impl_name}_obj PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../../include/core/
            ${VL_CONFIG_HEADER_INCLUDE}
    )

    # Add to parent scope list
    set(VL_SIMD_OBJECTS ${VL_SIMD_OBJECTS} $<TARGET_OBJECTS:vl_simd_${impl_name}_obj> PARENT_SCOPE)
    message(STATUS "    ${impl_name} object library created")
endfunction()