#include <iostream>
#include <string>

#include "Include/tools.h"
#include "Include/m128Utils.h"
#include "Include/lightray.h"
#include "Include/rendersettings.h"
#include "Include/rendertools.h"
#include "Include/materials.h"
#include "Include/objects.h"
#include "Include/scene.h"
#include "Include/memprep.h"
#include "Include/camera.h"

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
    if (argc < 3)
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
