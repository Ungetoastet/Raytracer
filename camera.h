#include <vector>
#include <iostream>
#include <omp.h>
#include <sstream>
#include <functional>
#include <corecrt_math_defines.h> // Windows fix

using namespace std;

class Camera
{
    using RenderKernel = std::function<Vec3(int x, int y)>;

protected:
    Vec3 lookDirection;
    int pixelCenterX;
    int pixelCenterY;
    RenderSettings renderSettings;
    Vec3 position;
    Vec3 lookAt;
    float fieldOfView;

public:
    /// @param position World position of the camera
    /// @param lookAt World position thats in the center of the rendered image
    /// @param fieldOfView Vertical FOV of the Camera in Degrees
    Camera(const Vec3 &position, const Vec3 &lookAt, float fieldOfView, const RenderSettings &rs)
    {
        this->position = position;
        this->lookAt = lookAt;
        this->fieldOfView = fieldOfView;
        lookDirection = position - lookAt;
        renderSettings = rs;
    }

    /// @brief Renders the complete image using the given settings
    void RenderImage(RenderKernel kernel)
    {
        double starttime = omp_get_wtime();
        std::string ppm = generate_PPM_header(renderSettings);
        const int calculatedChannelDepth = (1 << renderSettings.channel_depth) - 1;

        vector<std::string> rows(renderSettings.resolution[1]);

#pragma omp parallel for
        for (int y = 0; y < renderSettings.resolution[1]; y++)
        {
            std::ostringstream rowStream;
            for (int x = 0; x < renderSettings.resolution[0]; x++)
            {
                Vec3 color = kernel(x, y) * calculatedChannelDepth;
                rowStream << static_cast<int>(std::max(0.0f, std::min(color.x, 255.0f))) << " "
                          << static_cast<int>(std::max(0.0f, std::min(color.y, 255.0f))) << " "
                          << static_cast<int>(std::max(0.0f, std::min(color.z, 255.0f))) << " ";
            }
            rows[y] = rowStream.str();
        }

        for (const string &row : rows)
        {
            ppm += row + "\n";
        }

        std::cout << "Rendering done in " << (omp_get_wtime() - starttime) << std::endl;

        std::ofstream file(renderSettings.output_path);

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

    /// @param x Pixel Coordinate
    /// @param y Pixel Coordinate
    /// @return Light Ray that influences the pixel
    LightRay GenerateRayFromPixel(int x, int y)
    {
        // Richtung der Kamera berechnen
        Vec3 forward = (lookAt - position).normalized();

        // Rechtes und oberes Vektorsystem für die Kamera berechnen
        Vec3 right = forward.cross({0, 1, 0}).normalized();
        Vec3 up = right.cross(forward).normalized();

        // Field of view in Bogenmaß umrechnen
        float aspectRatio = static_cast<float>(renderSettings.resolution[0]) / renderSettings.resolution[1];
        float fovY = std::tan((fieldOfView * 0.5f) * (M_PI / 180.0f));
        float fovX = std::tan((fieldOfView * aspectRatio * 0.5f) * (M_PI / 180.0f));

        // Normalisierte Koordinaten für das Pixel berechnen
        float pixelNDC_X = (x + 0.5f) / renderSettings.resolution[0];
        float pixelNDC_Y = (y + 0.5f) / renderSettings.resolution[0];

        // Koordinaten im Sichtfeld berechnen
        float pixelScreen_X = (2.0f * pixelNDC_X - 1.0f) * fovX * aspectRatio;
        float pixelScreen_Y = (1.0f - 2.0f * pixelNDC_Y) * fovY;

        // Die Richtung des Rays basierend auf der Position und der Orientierung der Kamera berechnen
        Vec3 pixelWorld = forward + right * pixelScreen_X + up * pixelScreen_Y;
        Vec3 direction = pixelWorld.normalized();

        // Rückgabe des generierten Rays
        return LightRay{position, direction};
    }

    // RENDER KERNELS
    Vec3 kernel_colorTest(int x, int y)
    {
        float r = (float)x / renderSettings.resolution[0];
        float b = (float)y / renderSettings.resolution[1];
        return {r, 0.0f, b};
    }

    Vec3 kernel_rayTest(int x, int y)
    {
        LightRay lr = GenerateRayFromPixel(x, y);
        return lr.direction.normalized();
    }
};