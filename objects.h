#include <vector>
#include <iostream>

using namespace std;

class Object
{
protected:
    vector<float> position;
    vector<float> rotation;
    vector<float> scale;

public:
    Object(vector<float> position, vector<float> rotation, vector<float> scale)
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