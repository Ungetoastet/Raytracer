#include <vector>
#include <iostream>
#include <omp.h>
#include <sstream>
#include <functional>

#ifdef _WIN32
#include <corecrt_math_defines.h> // Windows fix
#endif

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
    Scene activeScene;

public:
    /// @param position World position of the camera
    /// @param lookAt World position thats in the center of the rendered image
    /// @param fieldOfView Vertical FOV of the Camera in Degrees
    Camera(const Vec3 &position, const Vec3 &lookAt, float fieldOfView, const RenderSettings &rs, const Scene &activeScene)
    {
        this->position = position;
        this->lookAt = lookAt;
        this->fieldOfView = fieldOfView;
        this->activeScene = activeScene;
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
    /// @return Light Ray that influences the pixel, direction is normalized
    LightRay GenerateRayFromPixel(int x, int y)
    {
        // Direction of the camera
        Vec3 forward = (lookAt - position).normalized();

        // Right and up vectors for the camera coordinate system
        Vec3 right = forward.cross({0, 1, 0}).normalized();
        Vec3 up = right.cross(forward).normalized();

        // Field of view in radians
        float aspectRatio = static_cast<float>(renderSettings.resolution[0]) / renderSettings.resolution[1];
        float fovY = std::tan((fieldOfView * 0.5f) * (M_PI / 180.0f));
        float fovX = fovY * aspectRatio; // Corrected this line

        // Normalized device coordinates for the pixel
        float pixelNDC_X = (x + 0.5f) / renderSettings.resolution[0];
        float pixelNDC_Y = (y + 0.5f) / renderSettings.resolution[1];

        // Screen space coordinates
        float pixelScreen_X = (2.0f * pixelNDC_X - 1.0f) * fovX;
        float pixelScreen_Y = (1.0f - 2.0f * pixelNDC_Y) * fovY;

        // Calculate the direction of the ray based on the camera's position and orientation
        Vec3 pixelWorld = forward + right * pixelScreen_X + up * pixelScreen_Y;
        Vec3 direction = pixelWorld.normalized();

        // Return the generated ray
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
        return lr.direction;
    }

    std::vector<Vec3> skybox_colors = {
        {0.0f, 0.02f, 0.08f},
        {0.3f, 0.2f, 0.5f},
        {0.8314f, 0.8118f, 0.7922f},
        {0.9331f, 0.8118f, 0.3922f},
        {0.8039f, 0.8667f, 0.9294f},
        {0.2353f, 0.2471f, 0.3686f}};

    std::vector<float> skybox_marks = {
        0.0f, 0.15f, 0.46f, 0.52f, 0.6f, 1.0f};

    Vec3 kernel_skyboxOnly(int x, int y)
    {
        LightRay lr = GenerateRayFromPixel(x, y);
        const float gradient_pos = (lr.direction.y * 0.5f) + 0.5f;

        return get_gradient(skybox_colors, skybox_marks, gradient_pos);
    }

    Vec3 kernel_flatObjects(int x, int y)
    {
        LightRay lr = GenerateRayFromPixel(x, y);
        for (Object *o : activeScene.objects)
        {
            Collision c = o->CheckCollision(lr);
            if (c.valid)
            {
                return {1.0f, 0.0f, 0.0f};
            }
        }
        const float gradient_pos = (lr.direction.y * 0.5f) + 0.5f;

        return get_gradient(skybox_colors, skybox_marks, gradient_pos);
    }
    Vec3 kernel_normals(int x, int y)
    {
        LightRay lr = GenerateRayFromPixel(x, y);
        for (Object *o : activeScene.objects)
        {
            Collision c = o->CheckCollision(lr);
            if (c.valid)
            {
                Vec3 col = c.normal * 0.5f + Vec3(0.5f, 0.5f, 0.5f);
                return col;
            }
        }
        const float gradient_pos = (lr.direction.y * 0.5f) + 0.5f;

        return get_gradient(skybox_colors, skybox_marks, gradient_pos);
    }

    /// @brief No scattering, no absorbsion, ten bounces
    Vec3 kernel_supershiny(int x, int y)
    {
        LightRay lr = GenerateRayFromPixel(x, y);
        for (int bounce = 0; bounce <= 10; bounce++)
        {
            bool hit = false;
            for (Object *o : activeScene.objects)
            {
                Collision c = o->CheckCollision(lr);
                if (c.valid)
                {
                    Vec3 reflected = c.incoming_direction.mirrorToNormalized(c.normal);
                    lr = LightRay(c.point, reflected);
                    hit = true;
                    break;
                }
            }
            if (hit)
                continue;

            const float gradient_pos = (lr.direction.y * 0.5f) + 0.5f;
            return get_gradient(skybox_colors, skybox_marks, gradient_pos);
        }

        // Max Bounces
        return {1.0f, 0.0f, 0.0f};
    }
};