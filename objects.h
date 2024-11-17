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
    Material mat;
    virtual ~Object() = default;
    Object(Vec3 position, Vec3 rotation, Vec3 scale, Material mat)
    {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        this->mat = mat;
    }
    virtual Collision CheckCollision(LightRay ray) = 0;
};

class Sphere : public Object
{
public:
    Sphere(Vec3 position, float size, Material mat) : Object(position, {0, 0, 0}, {size, size, size}, mat) {};

    Collision CheckCollision(LightRay ray) override
    {
        Vec3 center_origin_diff = ray.origin - position;

        float b = ray.direction.dot(center_origin_diff);
        float c = center_origin_diff.norm2() - scale.x * scale.x;
        float delta = b * b - c;

        if (delta < 0)
        {
            return NO_COLLISION;
        }

        float dist = -b - sqrtf(delta);

        if (dist < 0)
        {
            return NO_COLLISION;
        }

        Vec3 point = ray.origin + ray.direction * dist;
        Vec3 normal = (point - position).normalized();

        return {true, point, normal, ray.direction, dist};
    }
};

class Cube : public Object
{
};

class Plane : public Object
{
private:
    Vec3 normal;
    Vec3 localX;
    Vec3 localY;

public:
    Plane(Vec3 position, Vec3 rotation, Vec3 scale, Material mat) : Object(position, rotation, scale, mat)
    {
        normal = Vec3{0.0f, 0.0f, 1.0f}.rotate(rotation).normalized();
        localX = Vec3{1.0f, 0.0f, 0.0f}.rotate(rotation).normalized();
        localY = Vec3{0.0f, 1.0f, 0.0f}.rotate(rotation).normalized();
    };

    Collision CheckCollision(LightRay ray) override
    {
        float divider = ray.direction.dot(normal);
        if (abs(divider) <= 0.01)
        {
            return NO_COLLISION;
        }

        float dist = (position - ray.origin).dot(normal) / divider;
        if (dist <= 0.01)
        {
            return NO_COLLISION;
        }
        Vec3 point = ray.origin + ray.direction * dist;

        float x_dist = (point - position).cross(localY).length();
        if (abs(x_dist) > scale.x)
        {
            return NO_COLLISION;
        }

        float y_dist = (point - position).cross(localX).length();
        if (abs(y_dist) > scale.y)
        {
            return NO_COLLISION;
        }

        return {true, point, normal, ray.direction, dist};
    }
};

Collision checkWithSafeDowncast(Object *o, LightRay &lr)
{
    Sphere *s = dynamic_cast<Sphere *>(o);
    if (s)
    {
        return s->CheckCollision(lr);
    }
    Plane *p = static_cast<Plane *>(o);
    if (p)
    {
        return p->CheckCollision(lr);
    }

    std::cerr << "CORRUPTED OBJECT POINTER" << std::endl;
    return NO_COLLISION;
}