#include <vector>
#include <iostream>
#include <cstring>

#include <immintrin.h>

using namespace std;
using namespace m128Calc;

class Object
{
protected:
    Vec3 rotation;

public:
    __m128 scale;
    __m128 position;
    Material mat;
    char object_type;
    virtual ~Object() = default;
    Object(Vec3 position, Vec3 rotation, Vec3 scale, Material mat, char type)
    {
        this->position = position.data;
        this->rotation = rotation;
        this->scale = scale.data;
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
        Vec3 center_origin_diff = Vec3(ray.origin) - Vec3(position);

        float b = Vec3(ray.direction).dot(center_origin_diff);
        float scaleX = getX(scale);
        float c = center_origin_diff.norm2() - scaleX * scaleX;
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

        return {true, point.data, normal.data, ray.direction, dist};
    }
};

class Cube : public Object
{
};

class Plane : public Object
{
public:
    __m128 normal;
    __m128 localX;
    __m128 localY;
    Plane(Vec3 position, Vec3 rotation, Vec3 scale, Material mat) : Object(position, rotation, scale, mat, 1)
    {
        normal = normalized(Vec3{0.0f, 0.0f, 1.0f}.rotate(rotation).data);
        localX = normalized(Vec3{1.0f, 0.0f, 0.0f}.rotate(rotation).data);
        localY = normalized(Vec3{0.0f, 1.0f, 0.0f}.rotate(rotation).data);
    };

    Collision CheckCollision(LightRay ray) override
    {
        float divider = Vec3(ray.direction).dot(normal);
        if (abs(divider) <= 0.01)
        {
            return NO_COLLISION;
        }

        float dist = dot(_mm_sub_ps(position, ray.origin), normal) / divider;
        if (dist <= 0.01)
        {
            return NO_COLLISION;
        }
        Vec3 point = ray.origin + ray.direction * dist;
        Vec3 diff = point - position;

        float x_dist = diff.cross(localY).length();
        if (abs(x_dist) > getX(scale))
        {
            return NO_COLLISION;
        }

        float y_dist = diff.cross(localX).length();
        if (abs(y_dist) > getY(scale))
        {
            return NO_COLLISION;
        }

        return {true, point.data, normal, ray.direction, dist};
    }
};

Collision MemoryCollision(LightRay &ray, const float *objectMemStart)
{
    __m128 position = _mm_load_ps(objectMemStart);
    __m128 scale = _mm_load_ps(objectMemStart + 4);

    if (*(char *)(objectMemStart + 26) == 0) // Sphere Collision
    {
        __m128 center_origin_diff = _mm_sub_ps(ray.origin, position);

        float b = dot(ray.direction, center_origin_diff);
        float scaleX = getX(scale);
        float c = norm2(center_origin_diff) - scaleX * scaleX;
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
        __m128 distv = _mm_set_ps1(dist);
        __m128 point = _mm_fmadd_ps(ray.direction, distv, ray.origin);
        __m128 normal = normalized(_mm_sub_ps(point, position));

        return {true, point, normal, ray.direction, dist};
    }
    else // Plane Collision
    {
        __m128 normal = _mm_load_ps(objectMemStart + 8);
        __m128 localX = _mm_load_ps(objectMemStart + 12);
        __m128 localY = _mm_load_ps(objectMemStart + 16);

        float divider = dot(ray.direction, normal);
        if (abs(divider) <= 0.01)
        {
            return NO_COLLISION;
        }

        float dist = dot(_mm_sub_ps(position, ray.origin), normal) / divider;
        if (dist <= 0.01)
        {
            return NO_COLLISION;
        }
        __m128 distv = _mm_set_ps1(dist);
        __m128 point = _mm_fmadd_ps(ray.direction, distv, ray.origin);
        __m128 diff = _mm_sub_ps(point, position);

        float x_dist = sqrt(norm2(cross(diff, localY)));
        if (abs(x_dist) > getX(scale))
        {
            return NO_COLLISION;
        }

        float y_dist = sqrt(norm2(cross(diff, localX)));
        if (abs(y_dist) > getY(scale))
        {
            return NO_COLLISION;
        }

        return {true, point, normal, ray.direction, dist};
    }
}