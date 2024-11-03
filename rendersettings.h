#pragma once
#include <vector>
#include <string>

class RenderSettings
{

public:
    /// @brief Width, Height
    std::vector<int> resolution;
    std::string output_path;
    /// @brief How many bits to use for one RGB channel
    int channel_depth;
    /// @brief Default render settings: FullHD, 8bit depth
    RenderSettings() // Default settings
    {
        resolution = {1920, 1080};
        output_path = "render.ppm";
        channel_depth = 8;
    }

    /// @brief Manually set render settings
    /// @param resolution Resolution of the rendered image [x, y]
    /// @param output_path Path and file name of the rendered image
    /// @param channel_depth How many bits to use for each color channel. Must be 8 or 16
    RenderSettings(std::vector<int> resolution, std::string output_path, int channel_depth)
    {
        this->resolution = resolution;
        this->output_path = output_path;
        if (!(channel_depth == 8 || channel_depth == 16))
        {
            channel_depth = 8;
            std::cerr << "RENDERSETTINGS ERROR: CHANNEL DEPTH MUST BE 8 OR 16 BITS, DEFAULTING TO 8" << std::endl;
        }
        this->channel_depth = channel_depth;
    }

    /// @brief Imports render settings from xml file
    /// @param path_to_render_settings
    RenderSettings(std::string path_to_render_settings)
    {
        std::string content = removeComments(readFile(path_to_render_settings));
        XML_Node settings_root = parse_xml_bracket(content);
        if (settings_root.tag_name != "rendersettings")
        {
            std::cerr << "RENDERSETTINGS ERROR: ROOT NODE MUST BE RENDERSETTINGS" << std::endl;
        }
        for (std::string setting : settings_root.children)
        {
            XML_Node current_setting = parse_xml_bracket(setting);
            if (current_setting.tag_name == "resolution")
            {
            }
            else if (current_setting.tag_name == "depth")
            {
            }
            else if (current_setting.tag_name == "outputpath")
            {
            }
        }
    }
};