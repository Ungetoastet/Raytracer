#include <vector>
#include <iostream>

using namespace std;

class Material
{
protected:
    string id;
    vector<float> color;
    float intensity;
    float shininess;

public:
    Material(string id, vector<float> color, float intensity, float shininess)
    {
        this->id = id;
        this->color = color;
        this->intensity = intensity;
        this->shininess = shininess;
    }
};

class ShinyRed : Material{

};

class MatteBlue : Material{

};

class MatteGrey : Material{

};