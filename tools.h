#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <algorithm>

#include <xmmintrin.h> // Vector instrinsics
#include <pmmintrin.h> // SSE3

#include <cmath>

/// @brief Gives a random float in range 0 to 1
float random1(unsigned int *seed)
{
    *seed = (1140671485 * (*seed) + 12820163) % (1 << 24);
    return ((float)(*seed)) / (1 << 24);
}

/// @brief Gives a random float in range -1 to 1
inline float random2(unsigned int *seed)
{
    return random1(seed) * 2 - 1.0;
}

// Do not use in render kernel
struct alignas(16) Vec3
{
    // 4. Component is padding
    __m128 data;

    float x() const { return _mm_cvtss_f32(data); }
    float y() const { return _mm_cvtss_f32(_mm_shuffle_ps(data, data, _MM_SHUFFLE(0, 0, 0, 1))); }
    float z() const { return _mm_cvtss_f32(_mm_shuffle_ps(data, data, _MM_SHUFFLE(0, 0, 0, 2))); }

    /// @brief Creates a zero vector
    Vec3()
    {
        data = _mm_set1_ps(0);
    }
    Vec3(__m128 data)
    {
        (*this).data = data;
    }
    Vec3(float x, float y, float z)
    {
        data = _mm_setr_ps(x, y, z, 0);
    }
    Vec3 operator+(const Vec3 &other) const
    {
        return _mm_add_ps(other.data, data);
    }
    Vec3 operator-(const Vec3 &other) const
    {
        return _mm_sub_ps(data, other.data);
    }
    Vec3 operator*(const float scale) const
    {
        __m128 scalar = _mm_set1_ps(scale);
        return _mm_mul_ps(data, scalar);
    };
    float dot(const Vec3 &other) const
    {
        __m128 mult = _mm_mul_ps(data, other.data);
        __m128 temp = _mm_hadd_ps(mult, mult);
        temp = _mm_hadd_ps(temp, temp);
        return _mm_cvtss_f32(temp);
    }
    Vec3 cross(const Vec3 &other) const
    {
        // Shuffle components for cross product calculation
        __m128 a_yzx = _mm_shuffle_ps(data, data, _MM_SHUFFLE(3, 0, 2, 1));             // (a_y, a_z, a_x, 0)
        __m128 a_zxy = _mm_shuffle_ps(data, data, _MM_SHUFFLE(3, 1, 0, 2));             // (a_z, a_x, a_y, 0)
        __m128 b_yzx = _mm_shuffle_ps(other.data, other.data, _MM_SHUFFLE(3, 0, 2, 1)); // (b_y, b_z, b_x, 0)
        __m128 b_zxy = _mm_shuffle_ps(other.data, other.data, _MM_SHUFFLE(3, 1, 0, 2)); // (b_z, b_x, b_y, 0)

        // Cross product formula: c = (a_y * b_z - a_z * b_y, a_z * b_x - a_x * b_z, a_x * b_y - a_y * b_x)
        __m128 c = _mm_sub_ps(_mm_mul_ps(a_yzx, b_zxy), _mm_mul_ps(a_zxy, b_yzx));

        // Mask out the 4th component (padding)
        return _mm_and_ps(c, _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1)));
    }
    Vec3 normalized() const
    {
        // https://en.wikipedia.org/wiki/Fast_inverse_square_root
        float y = norm2();
        float x2 = y * 0.5;
        long i;
        const float threehalves = 1.5;

        i = *(long *)&y;                      // For bit manipulation
        i = 0x5f3759df - (i >> 1);            // Magic
        y = *(float *)&i;                     // Back to float for calculating
        y = y * (threehalves - (x2 * y * y)); // Newton iteration
        return (*this) * y;
    }
    float length() const
    {
        return sqrtf(norm2());
    }
    float norm2() const
    {
        __m128 mult = _mm_mul_ps(data, data);
        __m128 sum = _mm_add_ps(mult, _mm_shuffle_ps(mult, mult, _MM_SHUFFLE(0, 3, 2, 1)));
        sum = _mm_add_ps(sum, _mm_shuffle_ps(sum, sum, _MM_SHUFFLE(0, 0, 0, 2)));
        return _mm_cvtss_f32(sum);
    }
    std::string toString() const
    {
        return std::to_string(x()) + ", " + std::to_string(y()) + ", " + std::to_string(z());
    }
    /// @brief Rotate a vector around three axis
    /// @attention VERY SLOW! Do not use in render loop
    /// @param radRotation x, y, z Angles in radians
    /// @return Rotated Vector
    Vec3 rotate(Vec3 &radRotation) const
    {
        // Not called in render loop, performance not important
        float sina = sinf(radRotation.x());
        float sinb = sinf(radRotation.y());
        float sing = sinf(radRotation.z());
        float cosa = cosf(radRotation.x());
        float cosb = cosf(radRotation.y());
        float cosg = cosf(radRotation.z());

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
        float x = rot11 * (*this).x() + rot12 * (*this).y() + rot13 * (*this).z();
        float y = rot21 * (*this).x() + rot22 * (*this).y() + rot23 * (*this).z();
        float z = rot31 * (*this).x() + rot32 * (*this).y() + rot33 * (*this).z();
        return _mm_setr_ps(x, y, z, 0);
    }
    Vec3 radToEuler() const
    {
        __m128 mult = _mm_set1_ps(180.0f / M_PI);
        return _mm_mul_ps(mult, data);
    }
    Vec3 eulerToRad() const
    {
        __m128 mult = _mm_set1_ps(M_PI / 180.0f);
        return _mm_mul_ps(mult, data);
    }
    /// @return The rotation of the vector in radians
    Vec3 toRotation() const
    {
        Vec3 zAxis(0.0f, 0.0f, 1.0f);
        Vec3 targetNormalized = normalized();

        // Angle between vectors (using dot product)
        float angle = std::acos(zAxis.dot(targetNormalized));

        // Find rotation axis using cross product
        Vec3 axis = zAxis.cross(targetNormalized).normalized();

        // Convert axis-angle to Euler angles
        float yaw = std::atan2(axis.y(), axis.x()); // Rotation around Y-axis
        float pitch = angle;                        // Rotation around X-axis
        float roll = 0.0f;                          // No roll for this case

        return Vec3(pitch, yaw, roll);
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
__m128 get_gradient(__m128 *points, std::vector<float> &marks, float position)
{
    for (size_t i = 1; i < marks.size(); i++)
    {
        if (marks[i] >= position)
        {
            float relative_diff = (position - marks[i - 1]) / (marks[i] - marks[i - 1]);
            __m128 relative_diff_v = _mm_set_ps1(relative_diff);
            __m128 diff = _mm_sub_ps(points[i], points[i - 1]);
            return _mm_add_ps(points[i - 1], _mm_mul_ps(diff, relative_diff_v));
        }
    }
    std::cerr << "TOOL ERROR: INVALID GRADIENT POSITION: " << position << std::endl;
    return _mm_setzero_ps();
}

struct Collision
{
    bool valid;
    __m128 point;
    __m128 normal;
    __m128 incoming_direction;
    float distance;
};

const Collision NO_COLLISION = {false, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, 0};

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

std::string get_progress_bar(float progress, int width = 50)
{
    int filled_length = static_cast<int>(progress * width);
    int unfilled_length = width - filled_length;

    // Create the progress bar string
    std::string bar(filled_length, '#');
    bar.append(unfilled_length, ' ');

    return bar;
}