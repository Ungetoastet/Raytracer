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
            return NO_COLLISION;
        }

        float dist = -b - sqrtf(delta);

        if (dist < 0)
        {
            return NO_COLLISION;
        }

        Vec3 point = ray.origin + ray.direction * dist;
        Vec3 normal = (point - position).normalized();

        return {true, point, normal, ray.direction};
    }
};

class Plane : public Object
{
private:
    Vec3 normal;
    Vec3 localX;
    Vec3 localY;

public:
    Plane(Vec3 position, Vec3 rotation, Vec3 scale) : Object(position, rotation, scale)
    {
        normal = Vec3{0.0f, 0.0f, 1.0f}.rotate(rotation).normalized();
        localX = Vec3{1.0f, 0.0f, 0.0f}.rotate(rotation).normalized();
        localY = Vec3{0.0f, 1.0f, 0.0f}.rotate(rotation).normalized();
    };

    Collision CheckCollision(LightRay ray)
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

        return {true, point, normal, ray.direction};
    }
};

void CreateCube(Vec3 position, Vec3 rotation, Vec3 scale, Scene &s)
{
    Vec3 localX = Vec3(scale.x, 0, 0);
    Vec3 localY = Vec3(0, scale.y, 0);
    Vec3 localZ = Vec3(0, 0, scale.z);

    localX = localX.rotate(rotation);
    localY = localY.rotate(rotation);
    localZ = localZ.rotate(rotation);

    vector<Vec3> localRotate;
    localRotate.push_back(Vec3(0, 0, 0));
    localRotate.push_back(Vec3(0, 90, 0).eulerToRad());
    localRotate.push_back(Vec3(90, 0, 0).eulerToRad());

    Plane p1 = Plane(position + localX, localRotate[0], Vec3(1, 1, 1));
    Plane p2 = Plane(position + localY, localRotate[2], Vec3(1, 1, 1));
    Plane p3 = Plane(position + localZ, localRotate[1], Vec3(1, 1, 1));
    Plane p4 = Plane(position - localX, localRotate[0], Vec3(1, 1, 1));
    Plane p5 = Plane(position - localY, localRotate[2], Vec3(1, 1, 1));
    Plane p6 = Plane(position - localZ, localRotate[1], Vec3(1, 1, 1));

    s.objects.push_back(&p1);
    s.objects.push_back(&p2);
    s.objects.push_back(&p3);
    s.objects.push_back(&p4);
    s.objects.push_back(&p5);
    s.objects.push_back(&p6);
};