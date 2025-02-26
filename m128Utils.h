#include <xmmintrin.h> // Vector instrinsics
#include <pmmintrin.h> // SSE3
#include <immintrin.h>

#include <string>
#include <cmath>
#include "tools.h"

namespace m128Calc
{
    inline float getX(__m128 data) { return _mm_cvtss_f32(data); }
    inline float getY(__m128 data) { return _mm_cvtss_f32(_mm_shuffle_ps(data, data, _MM_SHUFFLE(0, 0, 0, 1))); }
    inline float getZ(__m128 data) { return _mm_cvtss_f32(_mm_shuffle_ps(data, data, _MM_SHUFFLE(0, 0, 0, 2))); }
    inline float getW(__m128 data) { return _mm_cvtss_f32(_mm_shuffle_ps(data, data, _MM_SHUFFLE(0, 0, 0, 3))); }
    inline __m128 scale(__m128 a, float s)
    {
        __m128 scalar = _mm_set1_ps(s);
        return _mm_mul_ps(a, scalar);
    }
    inline float dot(__m128 a, __m128 b)
    {
        __m128 result = _mm_dp_ps(a, b, 0x7F);
        return _mm_cvtss_f32(result);
    }
    __m128 cross(__m128 a, __m128 b)
    {
        // Shuffle components for cross product calculation
        __m128 a_yzx = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)); // (a_y, a_z, a_x, 0)
        __m128 a_zxy = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2)); // (a_z, a_x, a_y, 0)
        __m128 b_yzx = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1)); // (b_y, b_z, b_x, 0)
        __m128 b_zxy = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2)); // (b_z, b_x, b_y, 0)

        // Cross product formula: c = (a_y * b_z - a_z * b_y, a_z * b_x - a_x * b_z, a_x * b_y - a_y * b_x)
        __m128 c = _mm_sub_ps(_mm_mul_ps(a_yzx, b_zxy), _mm_mul_ps(a_zxy, b_yzx));

        // Mask out the 4th component (padding)
        return _mm_and_ps(c, _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1)));
    }
    inline float norm2(__m128 v)
    {
        return dot(v, v);
    }
#include <immintrin.h> // For SIMD intrinsics

    __m128 normalized(__m128 v)
    {
        float y = norm2(v);
        float x2 = y * 0.5f;
        const float threehalves = 1.5f;

        // Use a union for type-punning safely
        union
        {
            float f;
            int32_t i;
        } floatIntUnion;

        floatIntUnion.f = y;
        floatIntUnion.i = 0x5f3759df - (floatIntUnion.i >> 1); // Magic bit manipulation
        y = floatIntUnion.f;

        y = y * (threehalves - (x2 * y * y)); // Newton iteration
        __m128 yv = _mm_set1_ps(y);
        return _mm_mul_ps(v, yv);
    }

    std::string toString(__m128 v)
    {
        return std::to_string(getX(v)) + ", " + std::to_string(getY(v)) + ", " + std::to_string(getZ(v));
    }
    std::string toStringW(__m128 v)
    {
        return std::to_string(getX(v)) + ", " + std::to_string(getY(v)) + ", " + std::to_string(getZ(v)) + ", " + std::to_string(getW(v));
    }
    inline __m128 mirrorToNormalized(__m128 v, __m128 mirror)
    {
        __m128 dotv = _mm_set1_ps(dot(v, mirror) * 2);
        __m128 scaledMirror = _mm_mul_ps(dotv, mirror);
        return _mm_sub_ps(v, scaledMirror);
    }
    /// @brief Randomly shift the vector by a tiny amount
    /// @param strength How far the changed vector is from the original
    inline __m128 scatter(__m128 v, float strength, unsigned int *seed)
    {
        __m128 scatter = random3(seed);
        __m128 strengthv = _mm_set1_ps(strength);
        return _mm_fmadd_ps(scatter, strengthv, v);
    }

    __m128 radToEuler(__m128 v)
    {
        __m128 mult = _mm_set1_ps(180.0f / M_PI);
        return _mm_mul_ps(mult, v);
    }
    __m128 eulerToRad(__m128 v)
    {
        __m128 mult = _mm_set1_ps(M_PI / 180.0f);
        return _mm_mul_ps(mult, v);
    }
    // __m128 sampleHemiSphere(__m128 normalDir, int *rngSeed)
    // {
    //     __m128 half = _mm_set1_ps(0.5f);
    //     __m128 halfNormalDir = _mm_mul_ps(normalDir, half);
    // }
}