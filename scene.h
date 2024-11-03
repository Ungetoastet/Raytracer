#pragma once
#include <string>

#include "objects.h"
#include "tools.h"

using namespace std;

class scene
{
private:
    /// @brief Parses the scene file into a scene object
    /// @param path_to_file Path to .scene file
    /// @return Scene object
    void parseFromFile(std::string path_to_file)
    {
        string content = removeComments(readFile(path_to_file));
        XML_Node scene_root = parse_xml_bracket(content);
        if (scene_root.tag_name != "scene")
        {
            std::cerr << "SCENE ERROR: Root node must be scene";
        }
    }

    vector<Object> objects;

public:
    scene(std::string path_to_file)
    {
        parseFromFile(path_to_file);
    }
};
