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

    Material test_emissive = Material("testemi", Vec3(2.0, 2.0, 2.0), 0.3, -1);
    Material test_blue = Material("testblue", Vec3(0.1, 0.1, 1), 0.6, 0.5);
    Material test_red = Material("testred", Vec3(1, 0.1, 0.1), 0.6, 0.5);
    Material test_white = Material("testwhite", Vec3(1, 1, 1), 0.8, 0.5);
    Material test_mirror = Material("testmirror", Vec3(1, 1, 1), 0.9, 0.1);
    Material test_black = Material("testblack", Vec3(0.1, 0.1, 0.1), 0.4, 0.6);

    Plane light = Plane({0, 1.9f, 0}, Vec3{90, 0, 0}.eulerToRad(), {1, 1, 1}, test_emissive);
    testscene.objects.push_back(&light);
    Plane ceiling = Plane({0, 2, 0}, Vec3{90, 0, 0}.eulerToRad(), {5, 5, 5}, test_white);
    testscene.objects.push_back(&ceiling);
    Plane ground = Plane({0, -2, 0}, Vec3{90, 0, 0}.eulerToRad(), {5, 5, 5}, test_white);
    testscene.objects.push_back(&ground);
    Plane back_wall = Plane({0, 0, 5}, Vec3{0, 0, 0}.eulerToRad(), {5, 2, 5}, test_white);
    testscene.objects.push_back(&back_wall);
    Plane left_wall = Plane({3, 0, 0}, Vec3{0, 90, 0}.eulerToRad(), {5, 2, 5}, test_blue);
    testscene.objects.push_back(&left_wall);
    Plane right_wall = Plane({-3, 0, 0}, Vec3{0, 90, 0}.eulerToRad(), {5, 2, 5}, test_red);
    testscene.objects.push_back(&right_wall);
    Sphere ball_R = Sphere({-1.5f, -2, 0}, 1, test_mirror);
    testscene.objects.push_back(&ball_R);
    Sphere ball_L = Sphere({1, -1, -1}, 1.5f, test_white);
    testscene.objects.push_back(&ball_L);
    Sphere ball_B = Sphere({-1, -1, 1}, 1, test_black);
    testscene.objects.push_back(&ball_B);

    RenderSettings rendersettings = RenderSettings("./Documentation/rendersettings.xml");
    Camera testcam = Camera({0, 0, -8}, {0, 0, 0}, 45, rendersettings, testscene);

    // Bind kernel
    auto boundKernel = std::bind(&Camera::kernel_full, &testcam, std::placeholders::_1, std::placeholders::_2);
    testcam.RenderImage(boundKernel);
    return 0;
}
