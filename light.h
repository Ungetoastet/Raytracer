#include <vector>
#include <iostream>

using namespace std;

class Light
{
protected:
    vector<float> position;
    vector<float> color;
    float intensity;

public:
    Light(vector<float> position, vector<float> color, float intensity)
    {
        this->position = position;
        this->color = color;
        this->intensity = intensity;
    }
};

class PointLight : Light{

};

class DirectionalLight : Light{

};