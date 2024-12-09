#include <cstring>
#include <immintrin.h>

class alignas(32) LightRay
{
public:
    __m128 origin;

    __m128 direction;

    LightRay(__m128 origin, __m128 direction)
    {
        this->origin = origin;
        this->direction = direction;
    }
};

class alignas(128) LightRayBundle
{
public:
    __m512 origins;
    __m512 directions;

    LightRayBundle(__m512 origins, __m512 directions)
    {
        this->origins = origins;
        this->directions = directions;
    }
    LightRayBundle(LightRay l1, LightRay l2, LightRay l3, LightRay l4)
    {
        alignas(64) float tempOrigins[16];
        alignas(64) float tempDirections[16];

        memcpy(&tempOrigins + 00, &l1.origin, 16);
        memcpy(&tempOrigins + 16, &l2.origin, 16);
        memcpy(&tempOrigins + 32, &l3.origin, 16);
        memcpy(&tempOrigins + 48, &l4.origin, 16);
        this->origins = _mm512_load_ps(tempOrigins);

        memcpy(&tempDirections + 00, &l1.direction, 16);
        memcpy(&tempDirections + 16, &l2.direction, 16);
        memcpy(&tempDirections + 32, &l3.direction, 16);
        memcpy(&tempDirections + 48, &l4.direction, 16);
        this->directions = _mm512_load_ps(tempDirections);
    }
};