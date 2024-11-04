#include <vector>
#include <iostream>

using namespace std;

class Object
{
protected:
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;

public:
    Object(Vec3 position, Vec3 rotation, Vec3 scale)
    {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
    }
};

class Sphere : Object
{
};

class Cube : Object
{
};

class Plane : Object
{
};