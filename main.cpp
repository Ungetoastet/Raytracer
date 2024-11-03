#include <iostream>
#include <string>

#include "scene.h"
#include "camera.h"
#include "rendersettings.h"

int main()
{
    Scene("./Documentation/test.scene");
    RenderSettings rendersettings = RenderSettings();
    Camera testcam = Camera({0, 0, 0}, {0, 0, 0}, 50);
    testcam.RenderImage(rendersettings);
    return 0;
}
