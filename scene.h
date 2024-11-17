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
                CreatePosition(current_camera.parameters);
            }
            else if (current_camera.tag_name == "lookAt")
            {
                CreateLookAt(current_camera.parameters);
            }
            else if (current_camera.tag_name == "fieldOfView")
            {
                CreateFieldOfView(current_camera.parameters);
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
                ParseMaterial(current_materials.children);
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
        Vec3 position;
        float size;
        string material_id;
        for (const auto &[key, value] : sphereParams)
        {
            if (key == "position")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
                position = stringToVec3(value);
            }
            else if (key == "size")
            {
                size = stringToFloat(value);
            }
            else if (key == "material")
            {
                material_id = value;
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN VALUE " << std::endl;
            }
        }
        // Variablen an constructor übergeben
        Sphere sphere(position, size, material_id);
    }

    void CreateCube(std::map<std::string, std::string> cubeParams)
    {
        // Variablen erstellen
        Vec3 position;
        float size;
        string material_id;
        for (const auto &[key, value] : cubeParams)
        {
            if (key == "position")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
                position = stringToVec3(value);
            }
            else if (key == "size")
            {
                size = stringToFloat(value);
            }
            else if (key == "material")
            {
                material_id = value;
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN VALUE " << std::endl;
            }
        }
        // Variablen an constructor übergeben
        Cube cube(position, size, material);
    }

    void CreatePlane(std::map<std::string, std::string> planeParams)
    {
        // Variablen erstellen
        Vec3 position;
        Vec3 rotation;
        float size;
        string material_id;
        for (const auto &[key, value] : planeParams)
        {
            if (key == "position")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
                position = stringToVec3(value);
            }
            else if (key == "size")
            {
                size = stringToFloat(value);
            }
            else if (key == "material")
            {
                material_id = value;
            }
            else if (key == "rotation")
            {
                rotation = stringToVec3(value);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN VALUE " << std::endl;
            }
        }
        // Variablen an constructor übergeben
        Plane plane(position, size, material, rotation);
    }

    void CreatePointLight(std::map<std::string, std::string> pointLightParams)
    {
        // Variablen erstellen
        Vec3 color;
        Vec3 position;
        float intensity;
        for (const auto &[key, value] : pointLightParams)
        {
            if (key == "position")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
                position = stringToVec3(value);
            }
            else if (key == "color")
            {
                color = stringToVec3(value);
            }
            else if (key == "intensity")
            {
                intensity = stringToFloat(value);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN VALUE " << std::endl;
            }
        }
        // Variablen an constructor übergeben
        PointLight pointLight(position, color, intensity);
    }

    void CreateDirectionalLight(std::map<std::string, std::string> directionalLightParams)
    {
        // Variablen erstellen
        Vec3 direction;
        Vec3 color;
        float intensity;
        for (const auto &[key, value] : directionalLightParams)
        {
            if (key == "direction")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
                direction = stringToVec3(value);
            }
            else if (key == "color")
            {
                color = stringToVec3(value);
            }
            else if (key == "intensity")
            {
                intensity = stringToFloat(value);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN VALUE " << std::endl;
            }
        }
        // Variablen an constructor übergeben
        DirectionalLight directionalLight(direction, color, intensity);
    }

    void CreatePosition(std::map<std::string, std::string> sphereParams)
    {
        // Variablen erstellen
        float x;
        float y;
        float z;
        for (const auto &[key, value] : sphereParams)
        {
            if (key == "x")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
                x = stringToFloat(value);
            }
            else if (key == "y")
            {
                y = stringToFloat(value);
            }
            else if (key == "z")
            {
                z = stringToFloat(value);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN VALUE " << std::endl;
            }
        }
        // Variablen an constructor übergeben
        Vec3 position(x, y, z);
    }

    void CreateLookAt(std::map<std::string, std::string> lookAtParams)
    {
        // Variablen erstellen
        float x;
        float y;
        float z;
        for (const auto &[key, value] : lookAtParams)
        {
            if (key == "x")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
                x = stringToFloat(value);
            }
            else if (key == "y")
            {
                y = stringToFloat(value);
            }
            else if (key == "z")
            {
                z = stringToFloat(value);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN VALUE " << std::endl;
            }
        }
        // Variablen an constructor übergeben
        Vec3 lookAt(x, y, z);
    }

    void CreateFieldOfView(std::map<std::string, std::string> fieldOfViewParams)
    {
        // Variablen erstellen
        float val;
        for (const auto &[key, value] : fieldOfViewParams)
        {
            if (key == "value")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
                val = stringToFloat(value);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN VALUE " << std::endl;
            }
        }
        // Variablen an constructor übergeben
        float fieldOfView(val);
    }

    void CreateDiffuse(std::map<std::string, std::string> diffuseParams)
    {
        // Variablen erstellen
        string materialID;
        for (const auto &[key, value] : materialsParams)
        {
            if (key == "material")
            {
                // Position speichern in variable;
                // String zu vektor methode aufrufen (musst du noch schreiben (lassen))
                materialID = value;
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN VALUE " << std::endl;
            }
        }
        // Variablen an constructor übergeben
        Material material(materialID);
    }

public:
    std::vector<Object *> objects;
    /// @brief Generates a scene object based on the .scene xml file
    /// @param path_to_file
    Scene(std::string path_to_file)
    {
        parseFromFile(path_to_file);
    }
};