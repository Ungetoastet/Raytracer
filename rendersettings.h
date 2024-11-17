#pragma once
#include <vector>
#include <string>

class RenderSettings
{
private:
    void SetResolution(std::map<std::string, std::string> xml_params)
    {
        resolution = {0, 0};
        for (const auto &[key, value] : xml_params)
        {
            if (key == "x")
            {
                resolution[0] = std::stoi(value);
                if (resolution[0] % 5 != 0)
                {
                    std::cerr << "RENDERSETTINGS ERROR: X AXIS RESOLUTION MUST BE DIVISIBLE BY 5" << std::endl;
                }
            }
            else if (key == "y")
            {
                resolution[1] = std::stoi(value);
                if (resolution[1] % 5 != 0)
                {
                    std::cerr << "RENDERSETTINGS ERROR: Y AXIS RESOLUTION MUST BE DIVISIBLE BY 5" << std::endl;
                }
            }
            else
            {
                std::cerr << "RENDERSETTINGS ERROR: UNKNOWN RESOLUTION PARAMETER" << std::endl;
            }
        }
        if (resolution[0] == 0 || resolution[1] == 0)
        {
            std::cerr << "RENDERSETTINGS ERROR: MISSING RESOLUTION PARAMETER" << std::endl;
        }
    }

    void SetDepth(std::map<std::string, std::string> xml_params)
    {
        channel_depth = -1;
        for (const auto &[key, value] : xml_params)
        {
            if (key == "b")
            {
                channel_depth = std::stoi(value);
            }
            else
            {
                std::cerr << "RENDERSETTINGS ERROR: UNKNOWN DEPTH PARAMETER" << std::endl;
            }
        }
        if (channel_depth == -1)
        {
            std::cerr << "RENDERSETTINGS ERROR: MISSING DEPTH PARAMETER" << std::endl;
        }
        else if (!(channel_depth == 8 || channel_depth == 16))
        {
            channel_depth = 8;
            std::cerr << "RENDERSETTINGS ERROR: CHANNEL DEPTH MUST BE 8 OR 16 BITS, DEFAULTING TO 8" << std::endl;
        }
    }

    void SetOutputpath(std::map<std::string, std::string> xml_params)
    {
        output_path = "";
        for (const auto &[key, value] : xml_params)
        {
            if (key == "path")
            {
                output_path = value;
            }
            else
            {
                std::cerr << "RENDERSETTINGS ERROR: UNKNOWN OUTPUTPATH PARAMETER" << std::endl;
            }
        }
        if (output_path == "")
        {
            std::cerr << "RENDERSETTINGS ERROR: MISSING OUTPUTPATH PARAMETER" << std::endl;
        }
    }

    void SetSupersampling(std::map<std::string, std::string> xml_params)
    {
        for (const auto &[key, value] : xml_params)
        {
            if (key == "steps")
            {
                supersampling_steps = stoi(value);
            }
            else
            {
                std::cerr << "RENDERSETTINGS ERROR: UNKNOWN SUPERSAMPLING PARAMETER" << std::endl;
            }
        }
    }

public:
    /// @brief Width, Height
    std::vector<int> resolution;
    std::string output_path;
    int supersampling_steps;

    /// @brief How many bits to use for one RGB channel
    int channel_depth;
    /// @brief Default render settings: FullHD, 8bit depth
    RenderSettings() // Default settings
    {
        resolution = {1920, 1080};
        output_path = "render.ppm";
        channel_depth = 8;
        supersampling_steps = 1;
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
                SetResolution(current_setting.parameters);
            }
            else if (current_setting.tag_name == "depth")
            {
                SetDepth(current_setting.parameters);
            }
            else if (current_setting.tag_name == "outputpath")
            {
                SetOutputpath(current_setting.parameters);
            }
            else if (current_setting.tag_name == "supersampling")
            {
                SetSupersampling(current_setting.parameters);
            }
            else
            {
                std::cerr << "RENDERSETTINGS ERROR: UNKNOWN TAG " << current_setting.tag_name << std::endl;
            }
        }
    }
};
