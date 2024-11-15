#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <algorithm>

#include <cmath>

/// @brief Gives a random float in range 0 to 1
float random1(unsigned int *seed)
{
    *seed = (1140671485 * (*seed) + 12820163) % (1 << 24);
    return ((float)(*seed)) / (1 << 24);
}

/// @brief Gives a random float in range -1 to 1
float random2(unsigned int *seed)
{
    return random1(seed) * 2 - 1.0;
}

struct alignas(16) Vec3
{
    float x, y, z;
    float padding;

    /// @brief Creates a zero vector
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 self() const { return {x, y, z}; }
    Vec3 operator+(const Vec3 &other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3 operator-(const Vec3 &other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vec3 operator*(const float scale) const { return {x * scale, y * scale, z * scale}; };
    bool operator==(const Vec3 &other) const
    {
        return std::fabs(x - other.x) < 0.0001f &&
               std::fabs(y - other.y) < 0.0001f &&
               std::fabs(z - other.z) < 0.0001f;
    }
    // Checks if two vectors are parellel
    bool operator!=(const Vec3 &other)
    {
        Vec3 cross_product = this->cross(other);
        return std::fabs(cross_product.x) < 0.0001f &&
               std::fabs(cross_product.y) < 0.0001f &&
               std::fabs(cross_product.z) < 0.0001f;
    }
    float dot(const Vec3 &other) const { return x * other.x + y * other.y + z * other.z; }
    Vec3 cross(const Vec3 &other) const
    {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x};
    }
    Vec3 normalized() const
    {
        return self() * (1 / length());
    }
    float length() const
    {
        return sqrtf(norm2());
    }
    float norm2() const
    {
        return x * x + y * y + z * z;
    }
    std::string toString() const
    {
        return std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
    }
    Vec3 mirrorTo(Vec3 &mirror) const
    {
        Vec3 normalizedmirror = mirror.normalized();
        return self() - (normalizedmirror * self().dot(normalizedmirror)) * 2.0f;
    }
    Vec3 mirrorToNormalized(Vec3 &normalizedmirror)
    {
        return self() - (normalizedmirror * self().dot(normalizedmirror)) * 2.0f;
    }
    /// @brief Rotate a vector around three axis
    /// @param radRotation x, y, z Angles in radians
    /// @return Rotated Vector
    Vec3 rotate(Vec3 &radRotation) const
    {
        float sina = sinf(radRotation.x);
        float sinb = sinf(radRotation.y);
        float sing = sinf(radRotation.z);
        float cosa = cosf(radRotation.x);
        float cosb = cosf(radRotation.y);
        float cosg = cosf(radRotation.z);

        // https://en.wikipedia.org/wiki/Rotation_matrix#General_3D_rotations
        // Components of rotation matrix
        float rot11 = cosb * cosg;
        float rot12 = sina * sinb * cosg - cosa * sing;
        float rot13 = cosa * sinb * cosg + sina * sing;
        float rot21 = cosb * sing;
        float rot22 = sina * sinb * sing + cosa * cosg;
        float rot23 = cosa * sinb * sing - sina * cosg;
        float rot31 = -sinb;
        float rot32 = sina * cosb;
        float rot33 = cosa * cosb;

        // Calculate components of vector rot*v
        Vec3 res;
        res.x = rot11 * x + rot12 * y + rot13 * z;
        res.y = rot21 * x + rot22 * y + rot23 * z;
        res.z = rot31 * x + rot32 * y + rot33 * z;
        return res;
    }
    Vec3 radToEuler() const
    {
        float mult = 180.0f / M_PI;
        return {x * mult, y * mult, z * mult};
    }
    Vec3 eulerToRad() const
    {
        float mult = M_PI / 180.0f;
        return {x * mult, y * mult, z * mult};
    }
    /// @brief Randomly shift the vector by a tiny amount
    /// @param strength How far the changed vector is from the original
    Vec3 scatter(float strength, unsigned int *seed) const
    {
        Vec3 scattered;
        scattered.x = x + (random2(seed) * strength);
        scattered.y = y + (random2(seed) * strength);
        scattered.z = z + (random2(seed) * strength);
        return scattered.normalized();
    }
};

std::string readFile(const std::string &path_to_file)
{
    std::ifstream file(path_to_file);
    std::string line;
    std::string content;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            content += line;
        }
        file.close();
        return content;
    }
    else
    {
        std::cerr << "FILE ERROR: UNABLE TO OPEN" << std::endl;
        return "";
    }
}

std::string removeComments(const std::string &xmlContent)
{
    std::string result;
    size_t pos = 0;

    while (pos < xmlContent.size())
    {
        size_t start = xmlContent.find("<!--", pos);
        if (start == std::string::npos)
        {
            // No more comments, copy the rest of the string
            result += xmlContent.substr(pos);
            break;
        }

        // Copy text before the comment
        result += xmlContent.substr(pos, start - pos);

        // Find the end of the comment
        size_t end = xmlContent.find("-->", start);
        if (end == std::string::npos)
        {
            // No closing comment found, copy the rest of the string
            result += xmlContent.substr(start);
            break;
        }

        // Move position past the comment
        pos = end + 3; // 3 to skip over the closing "-->"
    }

    return result;
}

class XML_Node
{
public:
    std::string tag_name;
    std::vector<std::string> children;
    std::map<std::string, std::string> parameters;

    XML_Node(std::string tag_name, std::vector<std::string> children, std::map<std::string, std::string> parameters)
    {
        this->tag_name = tag_name;
        this->children = children;
        this->parameters = parameters;
    }
};

/// @brief Takes an xml structure string and returns an XML_Node
/// @param xml: string with xml type structure
/// @return xmlnode
XML_Node parse_xml_bracket(const std::string xml)
{
    // Find first tag
    size_t startPos = xml.find('<');
    if (startPos == std::string::npos)
    {
        std::cerr << "SCENE ERROR: TRYING TO PARSE EMPTY BRACKET" << std::endl;
    }

    size_t endPos = xml.find('>', startPos);
    if (endPos == std::string::npos)
    {
        std::cerr << "SCENE ERROR: NO CLOSING BRACKET" << std::endl;
    }

    std::string full_tag = xml.substr(startPos + 1, endPos - startPos - 1);

    // Extract tag name
    size_t tagname_end = full_tag.find(" ");
    std::string tag_name;
    std::map<std::string, std::string> parameters;

    if (tagname_end == std::string::npos)
    {
        tag_name = full_tag; // No parameters
    }
    else
    {
        tag_name = full_tag.substr(0, tagname_end);
        // Extract parameters
        size_t parameter_key_start = tagname_end + 1;

        while (parameter_key_start != std::string::npos)
        {
            size_t parameter_key_end = full_tag.find("=", parameter_key_start);
            if (parameter_key_end == std::string::npos)
            {
                break; // No more parameters
            }
            size_t parameter_value_start = full_tag.find("\"", parameter_key_end) + 1;
            size_t parameter_value_end = full_tag.find("\"", parameter_value_start);
            std::string parameter_key = full_tag.substr(parameter_key_start, parameter_key_end - parameter_key_start);
            std::string parameter_value = full_tag.substr(parameter_value_start, parameter_value_end - parameter_value_start);
            parameters[parameter_key] = parameter_value;
            parameter_key_start = full_tag.find(" ", parameter_value_end);
            if (parameter_key_start != std::string::npos)
            {
                parameter_key_start++;
            }
        }
    }

    // Extract children
    std::vector<std::string> children;

    // Find closing bracket of current node
    size_t closing_index = xml.find("</" + tag_name + ">", endPos);
    if (closing_index == std::string::npos)
    {
        size_t closing_index = xml.find("/>", startPos);
        if (closing_index == std::string::npos)
        {
            std::cerr << "SCENE ERROR: NO CLOSING TAG FOR " << tag_name << std::endl;
        }
        else
        {
            // Self closing node, no children
            return XML_Node(tag_name, children, parameters);
        }
    }
    std::string childstring = xml.substr(endPos + 1, closing_index - endPos - 1);

    // Find first bracket for children
    size_t child_start = childstring.find("<");
    while (child_start != std::string::npos)
    {
        size_t child_tag_close = childstring.find(">", child_start);
        if (child_tag_close == std::string::npos)
        {
            std::cerr << "SCENE ERROR: INCOMPLETE CHILD TAG" << std::endl;
            break;
        }

        std::string child_tag = childstring.substr(child_start + 1, child_tag_close - child_start - 1);
        std::string child_tag_name;
        size_t child_name_end = child_tag.find(" ");
        if (child_name_end == std::string::npos)
        {
            child_tag_name = child_tag; // No attributes, full tag is the name
        }
        else
        {
            child_tag_name = child_tag.substr(0, child_name_end);
        }

        // Check if it's a self-closing tag (ends with "/>")
        if (childstring[child_tag_close - 1] == '/')
        {
            // Self-closing tag, add it directly
            children.emplace_back(childstring.substr(child_start, child_tag_close - child_start + 1));
            child_start = child_tag_close + 1; // Move past the self-closing tag
        }
        else
        {
            // Find the corresponding closing tag for this child
            size_t child_end_tag = childstring.find("</" + child_tag_name + ">", child_tag_close);
            if (child_end_tag == std::string::npos)
            {
                std::cerr << "SCENE ERROR: NO CLOSING TAG FOR CHILD " << child_tag_name << std::endl;
                break;
            }
            else
            {
                // Extract the child node with opening and closing tags
                children.emplace_back(childstring.substr(child_start, child_end_tag + child_tag_name.size() + 3 - child_start));
                child_start = child_end_tag + child_tag_name.size() + 3; // Move past the closing tag
            }
        }

        // Find the next child starting from the new position
        child_start = childstring.find("<", child_start);
    }

    return XML_Node(tag_name, children, parameters);
}

/// @brief Returns the color in the gradient
/// @param points The colors inside the gradient
/// @param marks The positions where the colors are in the gradient, must be sorted. First must be 0, last must be 1.
/// @return Color inside gradient
Vec3 get_gradient(std::vector<Vec3> &points, std::vector<float> &marks, float position)
{
    for (size_t i = 1; i < marks.size(); i++)
    {
        if (marks[i] >= position)
        {
            float relative_diff = (position - marks[i - 1]) / (marks[i] - marks[i - 1]);
            Vec3 diff = points[i] - points[i - 1];
            return points[i - 1] + (diff * relative_diff);
        }
    }
    std::cerr << "TOOL ERROR: INVALID GRADIENT POSITION: " << position << std::endl;
    return {0, 0, 0};
}

struct Collision
{
    bool valid;
    Vec3 point;
    Vec3 normal;
    Vec3 incoming_direction;
};

const Collision NO_COLLISION = {false, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

std::pair<std::vector<size_t>, std::vector<float>> sortWithIndex(const std::vector<float> &arr)
{
    std::vector<std::pair<float, size_t>> valueIndexPairs;
    for (size_t i = 0; i < arr.size(); i++)
    {
        valueIndexPairs.emplace_back(arr[i], i);
    }

    std::sort(valueIndexPairs.begin(), valueIndexPairs.end());

    std::vector<float> sortedArray;
    std::vector<size_t> ogIndices;

    for (const auto &pair : valueIndexPairs)
    {
        sortedArray.push_back(pair.first);
        ogIndices.push_back(pair.second);
    }

    return {ogIndices, sortedArray};
}