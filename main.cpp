#include <iostream>
#include <string>

#include "tools.h"
#include "lightray.h"
#include "rendersettings.h"
#include "rendertools.h"
#include "objects.h"
#include "scene.h"
#include "camera.h"
#include "light.h"
#include "materials.h"

int main()
{
    Scene testscene = Scene("./Documentation/test.scene");
    Sphere testsphere = Sphere({0, 0, 0}, 1);
    testscene.objects.push_back(&testsphere);
    Sphere testsphere2 = Sphere({2, 1, 0}, 0.5);
    testscene.objects.push_back(&testsphere2);

    RenderSettings rendersettings = RenderSettings("./Documentation/rendersettings.xml");
    Camera testcam = Camera({0, 0, -5}, {0, 0, 0}, 45, rendersettings, testscene);

    // Bind kernel
    auto boundKernel = std::bind(&Camera::kernel_flatObjects, &testcam, std::placeholders::_1, std::placeholders::_2);
    testcam.RenderImage(boundKernel);
    return 0;
}
