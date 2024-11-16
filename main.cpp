#include <iostream>
#include <string>

#include "tools.h"
#include "lightray.h"
#include "rendersettings.h"
#include "rendertools.h"
#include "materials.h"
#include "objects.h"
#include "scene.h"
#include "camera.h"
#include "light.h"

int main()
{
    Scene testscene = Scene("./Documentation/test.scene");

    Material test_red = Material("testred", Vec3(1.0, 0.0, 0.0), 0.4, 0.1);
    Material test_blue = Material("testblue", Vec3(0.3, 0.3, 1.0), 0.3, 0.05);
    Material mirror = Material("mirror", Vec3(1, 1, 1), 1, 0.01);
    Material shiny_black = Material("shinyblack", Vec3(0.1, 0.1, 0.1), 0.3, 0.01);

    Sphere testsphere = Sphere({0, 0, 0}, 1, mirror);
    testscene.objects.push_back(&testsphere);
    Sphere testsphere2 = Sphere({2, 1, -1}, 0.5, test_blue);
    testscene.objects.push_back(&testsphere2);
    Plane testplane = Plane({2, -2, 0}, Vec3{45, 0, 45}.eulerToRad(), {2, 2, 2}, test_red);
    testscene.objects.push_back(&testplane);
    Plane testplane2 = Plane({-3, 0, 3}, Vec3{5, -35, 0}.eulerToRad(), {1, 2, 1}, mirror);
    testscene.objects.push_back(&testplane2);
    Plane testplane3 = Plane({1, 0, 2}, {0, 0, 0}, {2, 2, 2}, shiny_black);
    testscene.objects.push_back(&testplane3);

    RenderSettings rendersettings = RenderSettings("./Documentation/rendersettings.xml");
    Camera testcam = Camera({0, 0, -5}, {0, 0, 0}, 45, rendersettings, testscene);

    // Bind kernel
    auto boundKernel = std::bind(&Camera::kernel_scattertest, &testcam, std::placeholders::_1, std::placeholders::_2);
    testcam.RenderImage(boundKernel);
    return 0;
}
