#include <iostream>
#include <string>

#include "tools.h"
#include "m128Utils.h"
#include "lightray.h"
#include "rendersettings.h"
#include "rendertools.h"
#include "materials.h"
#include "objects.h"
#include "scene.h"
#include "memprep.h"
#include "camera.h"

void Scene::ParseCamera(std::map<std::string, std::string> camParams)
{
    Vec3 position;
    Vec3 lookAt;
    float fov = 45;
    bool skybox = false;

    for (const auto &[key, value] : camParams)
    {
        if (key == "position")
        {
            position = parseVec3(value);
        }
        else if (key == "lookAt")
        {
            lookAt = parseVec3(value);
        }
        else if (key == "fieldOfView")
        {
            fov = std::stof(value);
        }
        else if (key == "skybox")
        {
            skybox = (value == "true");
        }
        else
        {
            std::cerr << "SCENE ERROR: CAMERA PARAMETER " << key << std::endl;
        }
    }
    this->cam = new Camera(position, lookAt, fov, this->rs, *this, skybox);
}

void Scene::cleanup()
{
    for (Object *obj : objects)
    {
        delete obj;
    }
    objects.clear();

    delete cam;
    cam = nullptr;

    materials.clear();
}

int main(int argc, char *argv[])
{
    if (argc == 0)
    {
        std::cerr << "No argument provided. Usage: renderer.exe <scene_path> <rendersetttings_path>" << std::endl;
        return 1;
    }

    RenderSettings rendersettings = RenderSettings(argv[2]);
    Scene testscene = Scene(argv[1], rendersettings);

    testscene.cam->RenderImage(Camera::kernel_full);
    testscene.cleanup();
    return 0;
}
