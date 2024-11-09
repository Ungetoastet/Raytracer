#pragma once
#include <string>
#include <vector>

class Scene
{
private:
    /// @brief Parses the scene file into a scene object
    /// @param path_to_file Path to .scene file
    /// @return Scene object
    void parseFromFile(std::string path_to_file)
    {
        std::string content = removeComments(readFile(path_to_file));
        XML_Node scene_root = parse_xml_bracket(content);
        if (scene_root.tag_name != "scene")
        {
            std::cerr << "SCENE ERROR: ROOT NODE MUST BE SCENE";
        }
        for (std::string scene : scene_root.children)
        {
            XML_Node current_scene = parse_xml_bracket(scene);
            if (current_scene.tag_name == "objects")
            {
                ParseObjects(current_scene.children);
            }
            else if (current_scene.tag_name == "lights")
            {
                ParseLights(current_scene.children);
            }
            else if (current_scene.tag_name == "camera")
            {
                ParseCamera(current_scene.children);
            }
            else if (current_scene.tag_name == "materials")
            {
                ParseMaterials(current_scene.children);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN SCENE" << current_scene.tag_name << std::endl;
            }
        }
    }

    void ParseObjects(std::vector<string> objectStrings)
    {
        for (std::string object : objectStrings)
        {
            XML_Node current_object = parse_xml_bracket(object);
            if (current_object.tag_name == "Sphere")
            {
                CreateSphere(current_object.parameters);
            }
            else if (current_object.tag_name == "Cube")
            {
                CreateCube(current_object.parameters);
            }
            else if (current_object.tag_name == "Plane")
            {
                CreatePlane(current_object.parameters);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN OBJECT" << current_object.tag_name << std::endl;
            }
        }
    }

    void ParseLights(std::vector<string> lightStrings)
    {
        for (std::string light : lightStrings)
        {
            XML_Node current_light = parse_xml_bracket(light);
            if (current_light.tag_name == "PointLight")
            {
                CreatePointLight(current_light.parameters);
            }
            else if (current_light.tag_name == "DirectionalLight")
            {
                CreateDirectionalLight(current_light.parameters);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN LIGHT" << current_light.tag_name << std::endl;
            }
        }
    }

    void ParseCamera(std::vector<string> cameraStrings)
    {
        for (std::string camera : cameraStrings)
        {
            XML_Node current_camera = parse_xml_bracket(camera);
            if (current_camera.tag_name == "position")
            {
                CreatePosition(current_position.parameters);
            }
            else if (current_camera.tag_name == "lookAt")
            {
                CreateLookAt(current_lookAt.parameters);
            }
            else if (current_camera.tag_name == "fieldOfView")
            {
                CreateFieldOfView(current_fieldOfView.parameters);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN CAMERA" << current_camera.tag_name << std::endl;
            }
        }
    }

    void ParseMaterials(std::vector<string> materialsStrings)
    {
        for (std::string material : materialsStrings)
        {
            XML_Node current_materials = parse_xml_bracket(material);
            if (current_materials.tag_name == "material")
            {
                CreateMaterialS(current_materials.parameters, current_materials.children);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN MATERIAL" << current_materials.tag_name << std::endl;
            }
        }
    }

    void CreateSphere(std::map<std::string, std::string> sphereParams)
    {
        // Variablen erstellen
        for (const auto &[key, value] : sphereParams)
        {
            if (key == "position")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
            }
            else
            {
                std::cerr << "RENDERSETTINGS ERROR: UNKNOWN OUTPUTPATH PARAMETER" << std::endl;
            }
        }
        // Hier checken, ob alle variablen belegt wurden
        if ()
        {
            std::cerr << "RENDERSETTINGS ERROR: MISSING OUTPUTPATH PARAMETER" << std::endl;
        }
        // Variablen an constructor übergeben
        Sphere sphere;
    }

    std::vector<Object> objects;

public:
    /// @brief Generates a scene object based on the .scene xml file
    /// @param path_to_file
    Scene(std::string path_to_file)
    {
        parseFromFile(path_to_file);
    }
};
