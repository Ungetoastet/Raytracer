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
        Vec3 center_origin_diff = ray.origin - position;

        float b = ray.direction.dot(center_origin_diff);
        float c = center_origin_diff.norm2() - scale.x * scale.x;
        float delta = b * b - c;

        if (delta < 0)
        {
            return {false, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
        }

        float dist = -b - sqrtf(delta);

        if (dist < 0)
        {
            return {false, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
        }

        Vec3 point = ray.origin + ray.direction * dist;
        Vec3 normal = (point - position).normalized();

        return {true, point, normal, ray.direction};
    }
};

class Cube : public Object
{
};

class Plane : public Object
{
};