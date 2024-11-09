#include <iostream>
#include <string>

#include "tools.h"
#include "rendersettings.h"
#include "rendertools.h"
#include "objects.h"
#include "scene.h"
#include "lightray.h"
#include "camera.h"
#include "light.h"
#include "materials.h"

int main()
{
    Scene("./Documentation/test.scene");
    RenderSettings rendersettings = RenderSettings("./Documentation/rendersettings.xml");
    Camera testcam = Camera({0, 0, -5}, {0, 0, 0}, 40, rendersettings);

    // Bind kernel
    auto boundKernel = std::bind(&Camera::kernel_skyboxOnly, &testcam, std::placeholders::_1, std::placeholders::_2);
    testcam.RenderImage(boundKernel);
    return 0;
}
