#include <vector>
#include <iostream>

using namespace std;

class Camera
{
protected:
    vector<float> position;
    vector<float> lookAt;
    float fieldOfView;

public:
    Camera(vector<float> position, vector<float> lookAt, float fieldOfView)
    {
        this->position = position;
        this->lookAt = lookAt;
        this->fieldOfView = fieldOfView;
    }
};