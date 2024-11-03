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
