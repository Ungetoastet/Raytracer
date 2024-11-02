#include <iostream>
#include <fstream>
#include <string>

int main()
{
    std::ifstream file("./Documentation/test.scene"); // replace with your file path
    std::string line;

    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            std::cout << line << std::endl;
        }
        file.close();
    }
    else
    {
        std::cerr << "Unable to open file" << std::endl;
    }

    return 0;
}
