#include <iostream>
#include <string>

#include "tools.h"
#include "rendersettings.h"
#include "rendertools.h"
#include "objects.h"
#include "scene.h"
#include "camera.h"
#include "light.h"
#include "materials.h"

int main()
{
    Scene("./Documentation/test.scene");
    RenderSettings rendersettings = RenderSettings("./Documentation/rendersettings.xml");
    Camera testcam = Camera({0, 0, 0}, {0, 0, 0}, 50);
    testcam.RenderImage(rendersettings);
    return 0;
}
