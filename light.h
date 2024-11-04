#include <vector>
#include <iostream>

using namespace std;

class Light
{
protected:
    Vec3 position;
    Vec3 color;
    float intensity;

public:
    Light(Vec3 position, Vec3 color, float intensity)
    {
        this->position = position;
        this->color = color;
        this->intensity = intensity;
    }
};

class PointLight : Light
{
};

class DirectionalLight : Light
{
};