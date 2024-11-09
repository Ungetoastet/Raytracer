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

class Sphere : public Object
{
public:
    Sphere(Vec3 position, float size) : Object(position, {0, 0, 0}, {size, size, size}) {};
};

class Cube : public Object
{
};

class Plane : public Object
{
};