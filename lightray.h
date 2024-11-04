#include <vector>

class LightRay
{
public:
    Vec3 origin;

    Vec3 direction;

    LightRay(Vec3 origin, Vec3 direction)
    {
        this->origin = origin;
        this->direction = direction;
    }
};