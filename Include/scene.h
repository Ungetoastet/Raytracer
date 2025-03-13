#pragma once
#include <string>
#include <vector>
#include <sstream>

class Camera;

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
        bool defined_objects = false;
        bool defined_camera = false;
        for (std::string scene : scene_root.children)
        {
            XML_Node current_scene = parse_xml_bracket(scene);
            if (current_scene.tag_name == "objects")
            {
                ParseObjects(current_scene.children);
                defined_objects = true;
            }
            else if (current_scene.tag_name == "camera")
            {
                ParseCamera(current_scene.parameters);
                defined_camera = true;
            }
            else if (current_scene.tag_name == "materials")
            {
                ParseMaterials(current_scene.children);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN TAG " << current_scene.tag_name << std::endl;
            }
        }
        if (!defined_camera || !defined_objects)
        {
            std::cerr << "SCENE ERROR: EITHER NO CAMERA OR NO OBJECTS DEFINED!" << std::endl;
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
            else if (current_object.tag_name == "Plane")
            {
                CreatePlane(current_object.parameters);
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN OBJECT TYPE " << current_object.tag_name << std::endl;
            }
        }
    }

    void ParseCamera(std::map<std::string, std::string> sphereParams);

    void ParseMaterials(std::vector<string> materialsStrings)
    {
        for (std::string material : materialsStrings)
        {
            XML_Node current_material = parse_xml_bracket(material);
            if (current_material.tag_name != "material")
            {
                std::cerr << "SCENE ERROR: UNKNOWN MATERIAL TAG " << current_material.tag_name << std::endl;
            }
            string id = "";
            Vec3 color;
            float reflectiveness = 0;
            float roughness = 0;

            for (const auto &[key, value] : current_material.parameters)
            {
                if (key == "id")
                {
                    id = value;
                }
                else if (key == "color")
                {
                    color = parseVec3(value);
                }
                else if (key == "reflection")
                {
                    reflectiveness = std::stof(value);
                }
                else if (key == "roughness")
                {
                    roughness = std::stof(value);
                }
                else
                {
                    std::cerr << "SCENE ERROR: UNKNOWN MATERIAL PARAMETER " << key << std::endl;
                }
            }
            this->materials.push_back(Material(id, color, reflectiveness, roughness));
        }
    }

    void CreateSphere(std::map<std::string, std::string> sphereParams)
    {
        Vec3 position;
        float radius = 1;
        string materialID;

        for (const auto &[key, value] : sphereParams)
        {
            if (key == "position")
            {
                position = parseVec3(value);
            }
            else if (key == "radius")
            {
                radius = std::stof(value);
            }
            else if (key == "material")
            {
                materialID = value;
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN SPHERE PARAMETER" << std::endl;
            }
        }
        Material material = getMaterial(materialID);
        this->objects.push_back(new Sphere(position, radius, material));
    }

    void CreatePlane(std::map<std::string, std::string> planeParams)
    {
        Vec3 position;
        Vec3 rotation;
        Vec3 scale;
        string materialID;

        for (const auto &[key, value] : planeParams)
        {
            if (key == "position")
            {
                position = parseVec3(value);
            }
            else if (key == "rotation")
            {
                rotation = parseVec3(value);
                rotation = rotation.eulerToRad();
            }
            else if (key == "scale")
            {
                scale = parseVec3(value);
            }
            else if (key == "size")
            {
                float s = std::stof(value);
                scale = Vec3(s, s, s);
            }
            else if (key == "material")
            {
                materialID = value;
            }
            else
            {
                std::cerr << "SCENE ERROR: UNKNOWN PLANE PARAMETER" << std::endl;
            }
        }
        Material material = getMaterial(materialID);
        this->objects.push_back(new Plane(position, rotation, scale, material));
    }

    Material getMaterial(string id)
    {
        for (Material m : this->materials)
        {
            if (id == m.id)
            {
                return m;
            }
        }
        std::cerr << "SCENE ERROR: COULD NOT FIND MATERIAL ID " << id << ". Hint: Materials need to be defined before Objects." << std::endl;
        return Material("MISSING", Vec3(0, 0, 0), 0, 0);
    }

public:
    std::vector<Object *> objects;
    std::vector<Material> materials;
    Camera *cam;
    RenderSettings rs;

    /// @brief Generates a scene object based on the .scene xml file
    /// @param path_to_file
    Scene(std::string path_to_file, RenderSettings rs)
    {
        this->rs = rs;
        parseFromFile(path_to_file);
    }
    Scene() {};
    void cleanup();
};
