#include <vector>
#include <iostream>
#include <cstring>

using namespace std;

class Object
{
protected:
    Vec3 rotation;

public:
    Vec3 scale;
    Vec3 position;
    Material mat;
    char object_type;
    virtual ~Object() = default;
    Object(Vec3 position, Vec3 rotation, Vec3 scale, Material mat, char type)
    {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        this->mat = mat;
        object_type = type;
    }
    virtual Collision CheckCollision(LightRay ray) = 0;
};

class Sphere : public Object
{
public:
    Sphere(Vec3 position, float size, Material mat) : Object(position, {0, 0, 0}, {size, size, size}, mat, 0) {};

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

        if (dist <= 0.01)
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
public:
    Vec3 normal;
    Vec3 localX;
    Vec3 localY;
    Plane(Vec3 position, Vec3 rotation, Vec3 scale, Material mat) : Object(position, rotation, scale, mat, 1)
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
        Vec3 diff = point - position;

        float x_dist = diff.cross(localY).length();
        if (abs(x_dist) > scale.x)
        {
            return NO_COLLISION;
        }

        float y_dist = diff.cross(localX).length();
        if (abs(y_dist) > scale.y)
        {
            return NO_COLLISION;
        }

        return {true, point, normal, ray.direction, dist};
    }
};

Collision MemoryCollision(LightRay &ray, char *objectMemStart)
{
    Vec3 position;
    std::memcpy(&position, objectMemStart + 1, 12);

    Vec3 scale;
    std::memcpy(&scale, objectMemStart + 13, 12);

    if (*(char *)objectMemStart == 0) // Sphere Collision
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

        if (dist <= 0.01)
        {
            return NO_COLLISION;
        }

        Vec3 point = ray.origin + ray.direction * dist;
        Vec3 normal = (point - position).normalized();

        return {true, point, normal, ray.direction, dist};
    }
    else // Plane Collision
    {
        Vec3 normal;
        std::memcpy(&normal, objectMemStart + 25, 12);
        Vec3 localX;
        std::memcpy(&localX, objectMemStart + 37, 12);
        Vec3 localY;
        std::memcpy(&localY, objectMemStart + 49, 12);

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
        Vec3 diff = point - position;

        float x_dist = diff.cross(localY).length();
        if (abs(x_dist) > scale.x)
        {
            return NO_COLLISION;
        }

        float y_dist = diff.cross(localX).length();
        if (abs(y_dist) > scale.y)
        {
            return NO_COLLISION;
        }

        return {true, point, normal, ray.direction, dist};
    }
}