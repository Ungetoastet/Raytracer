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
    virtual Collision CheckCollision(LightRay ray)
    {
        std::cerr << "COLLISION ERROR: CANNOT CALCULATE COLLISION FOR OBJECT" << std::endl;
        return {false, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    };
};

class Sphere : public Object
{
public:
    Sphere(Vec3 position, float size) : Object(position, {0, 0, 0}, {size, size, size}) {};
    Collision CheckCollision(LightRay ray) override
    {
        // https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
        Vec3 center_origin_diff = ray.origin - position;
        float delta = powf(ray.direction.dot(center_origin_diff), 2) - (center_origin_diff.norm2() - scale.x * scale.x);
        if (delta <= 0)
        {
            return {false, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
        }
        return {true, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}; // TODO
    }
};

class Cube : public Object
{
};

class Plane : public Object
{
};