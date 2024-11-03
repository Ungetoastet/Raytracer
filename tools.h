#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

std::string readFile(std::string path_to_file)
{
    std::ifstream file(path_to_file); // replace with your file path
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
        std::cerr << "Unable to open file" << std::endl;
        return "";
    }
}

std::string removeComments(const std::string xmlContent)
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
        std::cerr << "SCENE ERROR: NO CLOSING TAG FOR " << tag_name << std::endl;
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

        // Find the corresponding closing tag for this child
        size_t child_end_tag = childstring.find("</" + child_tag_name + ">", child_tag_close);
        if (child_end_tag == std::string::npos)
        {
            std::cerr << "SCENE ERROR: NO CLOSING TAG FOR CHILD " << child_tag_name << std::endl;
        }

        // Extract the child node
        children.emplace_back(childstring.substr(child_start, child_end_tag + child_tag_name.size() + 3 - child_start));

        // Move to the next child
        child_start = childstring.find("<", child_end_tag + child_tag_name.size() + 3);
    }

    return XML_Node(tag_name, children, parameters);
}
