#include <string>

std::string generate_PPM_header(RenderSettings rs)
{
    // https://de.wikipedia.org/wiki/Portable_Anymap#Kopfdaten

    std::string header = "P3 ";                             // Magic number: Portable Pixmap (RGB), ASCII
    header += std::to_string(rs.resolution[0]) + " ";       // Define width
    header += std::to_string(rs.resolution[1]) + " ";       // Define height
    header += std::to_string(1 << rs.channel_depth) + "\n"; // Define color depth - 1<<x = 2^x
    return header;
}