#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "../Include/m128Utils.h"
#include <immintrin.h>

TEST_CASE("Vector extraction", "[m128Calc]")
{
    __m128 v = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    REQUIRE(m128Calc::getX(v) == Catch::Approx(1.0f));
    REQUIRE(m128Calc::getY(v) == Catch::Approx(2.0f));
    REQUIRE(m128Calc::getZ(v) == Catch::Approx(3.0f));
    REQUIRE(m128Calc::getW(v) == Catch::Approx(4.0f));
}

TEST_CASE("Vector Scaling", "[m128Calc]")
{
    __m128 v = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 scaled = m128Calc::scale(v, 2.0f);
    REQUIRE(m128Calc::getX(scaled) == Catch::Approx(2.0f));
    REQUIRE(m128Calc::getY(scaled) == Catch::Approx(4.0f));
    REQUIRE(m128Calc::getZ(scaled) == Catch::Approx(6.0f));
    REQUIRE(m128Calc::getW(scaled) == Catch::Approx(8.0f));
}

TEST_CASE("Dot product", "[m128Calc]")
{
    __m128 a = _mm_set_ps(0.0f, 3.0f, 2.0f, 1.0f);
    __m128 b = _mm_set_ps(0.0f, 6.0f, 4.0f, 2.0f);
    REQUIRE(m128Calc::dot(a, b) == Catch::Approx(28.0f));
}

TEST_CASE("Cross product", "[m128Calc]")
{
    __m128 a = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
    __m128 b = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
    __m128 result = m128Calc::cross(a, b);
    REQUIRE(m128Calc::getX(result) == Catch::Approx(1.0f));
    REQUIRE(m128Calc::getY(result) == Catch::Approx(0.0f));
    REQUIRE(m128Calc::getZ(result) == Catch::Approx(0.0f));
}

TEST_CASE("Normalization", "[m128Calc]")
{
    __m128 v = _mm_set_ps(0.0f, 0.0f, 3.0f, 4.0f);
    __m128 norm = m128Calc::normalized(v);
    float normFactor = 1.0f / std::sqrt(3.0f * 3.0f + 4.0f * 4.0f);
    REQUIRE(m128Calc::getX(norm) == Catch::Approx(4.0f * normFactor));
    REQUIRE(m128Calc::getY(norm) == Catch::Approx(3.0f * normFactor));
}

TEST_CASE("Random in box", "[m128Calc]")
{
    __m128i seed = _mm_set_epi32(1234, 5678, 91011, 1213);
    for (int i = 0; i < 100; i++)
    {
        __m128 r = m128Calc::randomvec(seed);
        REQUIRE((m128Calc::getX(r) <= 1.0f && m128Calc::getX(r) >= -1.0f));
        REQUIRE((m128Calc::getY(r) <= 1.0f && m128Calc::getY(r) >= -1.0f));
        REQUIRE((m128Calc::getZ(r) <= 1.0f && m128Calc::getZ(r) >= -1.0f));
        REQUIRE((m128Calc::getX(r) != m128Calc::getY(r) && m128Calc::getY(r) != m128Calc::getZ(r)));
    }
}

TEST_CASE("Random in unit sphere", "[m128Calc]")
{
    __m128i seed = _mm_set_epi32(1234, 5678, 91011, 1213);
    for (int i = 0; i < 100; i++)
    {
        __m128 randomVec = m128Calc::random_in_unit_sphere(seed);
        REQUIRE(m128Calc::norm2(randomVec) < 1.0f);
    }
}
