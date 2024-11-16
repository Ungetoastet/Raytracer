#include <vector>
#include <iostream>

using namespace std;

class Material
{
public:
    string id;
    Vec3 color;
    float intensity;
    float diffuse;

    Material()
    {
    }

    /// @param id Name of the material, should be unique
    /// @param color Base color of the material
    /// @param intensity How strong is the reflection visible
    /// @param diffuse How matte is the material
    Material(string id, Vec3 color, float intensity, float diffuse)
    {
        this->id = id;
        this->color = color;
        this->intensity = intensity;
        this->diffuse = diffuse;
    }
};
