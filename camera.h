#include <vector>
#include <iostream>
#include <omp.h>

#include "rendersettings.h"
#include "tools.h"

using namespace std;

class Camera
{
protected:
    vector<float> position;
    vector<float> lookAt;
    float fieldOfView;

public:
    Camera(vector<float> position, vector<float> lookAt, float fieldOfView)
    {
        this->position = position;
        this->lookAt = lookAt;
        this->fieldOfView = fieldOfView;
    }

    void RenderImage(RenderSettings settings)
    {
        double starttime = omp_get_wtime();
        std::string ppm = generate_PPM_header(settings);

        for (int y = 0; y < settings.resolution[1]; y++)
        {
            for (int x = 0; x < settings.resolution[0]; x++)
            {
                int r = ((float)x / settings.resolution[0]) * 255;
                int b = ((float)y / settings.resolution[0]) * 255;
                ppm += std::to_string(r) + " 0 " + std::to_string(b) + " ";
            }
            ppm += "\n";
        }

        std::cout << "Rendering done in " << (omp_get_wtime() - starttime) << std::endl;

        std::ofstream file(settings.output_path);

        if (file.is_open())
        {
            file << ppm;  // Write the string to the file
            file.close(); // Close the file after writing
        }
        else
        {
            std::cerr << "RENDER ERROR: UNABLE TO WRITE" << std::endl;
        }

        std::cout << "Writing to file done." << std::endl;

        return;
    }
};