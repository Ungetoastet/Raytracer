#include <vector>

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