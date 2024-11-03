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
    RenderSettings()
    {
        resolution = {720, 420};    // Default resolution
        output_path = "render.ppm"; // Default path
        channel_depth = 8;          // Default depth
    }
};