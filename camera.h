#include <vector>
#include <iostream>
#include <omp.h>
#include <sstream>
#include <functional>

using namespace std;

class Camera
{
    // Determines the color value for each pixel
    using RenderKernel = std::function<Vec3(int x, int y)>;

private:
    /// @brief Seed for randomness
    unsigned int rng_seed = 42;

    /// @brief Generates the full recursion for a single ray
    /// @param scene active scene
    /// @param bounces How many times the ray can bounce before it is interrupted
    /// @param scatter Into how many rays does the ray scatter on impact?
    /// @param scatterreduction How many scatter rays to lose after each bounce
    /// @return
    Vec3 FullTrace(LightRay lr, int bounces, int scatter, int scatterreduction)
    {
        // Check Object Collisions
        Object *closestObject;
        Collision closestCollision = NO_COLLISION;
        float closestDistance = 9999;

        for (Object *o : activeScene.objects)
        {
            Collision c = o->CheckCollision(lr);         // Kollision prüfen // TODO TEUER
            if (c.valid && c.distance < closestDistance) // wenn Kollision gültig und Objekt näher ist als Vorherige
            {
                closestDistance = c.distance;
                closestObject = o;
                closestCollision = c;
            }
            else
            {
                continue;
            }
        }

        if (closestCollision.valid) // wenn Kollision gefunden
        {
            // Objekteigenschaften auslesen
            float diffuse = closestObject->mat.diffuse;
            float intensity = closestObject->mat.intensity;
            // Scatter
            if (bounces == 0) // wenn max Anzahl von Kollisionen erreicht
            {
                return {0, 0, 0}; // Rückgabe von schwarz
            }
            if (diffuse < 0)
            {
                // For emissive materials, Rückgabe Emissionsfarbe
                return closestObject->mat.color;
            }
            // wenn Material diffus & nicht emissiv
            Vec3 resColor = {0, 0, 0};
            for (int s = 0; s < scatter; s++)
            {
                Vec3 reflected = closestCollision.incoming_direction.mirrorToNormalized(closestCollision.normal) + Vec3{0, 0, 0}.scatter(diffuse, &rng_seed); // normalisierte Spiegelung, hinzufügen zufällige Streuung
                reflected = reflected.normalized();
                Vec3 hit_color = (closestObject->mat.color * (1 - intensity)) + (FullTrace(LightRay(closestCollision.point, reflected), bounces - 1, scatter - scatterreduction, scatterreduction) * intensity); // Rekursion mit kleinerer bounces- und scatter-Anzahl
                resColor = resColor + hit_color;
            }

            return (closestObject->mat.color * (scatter <= 0)) + ((resColor * (1.0f / scatter)) * (scatter > 0)); // ohne Streustrahlung direkt Eigenfarbe des Objektes zurückgeben
        }
        else // wenn keine Kollision gefunden, Farbe des Hintergrundes berechnen
        {
            float gradient_pos = (lr.direction.y * 0.5f) + 0.5f;
            return get_gradient(skybox_colors, skybox_marks, gradient_pos);
        }
    }

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
        std::string ppm = generate_PPM_header(renderSettings);                      // Header der PPM-Datei erstellt --> Infos wie Bildauflösung, Channel-Depth
        const int calculatedChannelDepth = (1 << renderSettings.channel_depth) - 1; // Berechnung Channel-Depth

        vector<std::string> rows(renderSettings.resolution[1]); // Speicher für Zeilen des Bildes

        std::cout << "Starting render" << std::endl;

        float starttime = omp_get_wtime();

        // Compute color for each pixel
        // Zeilen in thread_row_table werden parallelisiert
        // jeder Thread rendert die ihm zugewiesenen Zeitbündel (je 5 Zeilen)
        int rows_done = 0;
#pragma omp parallel
        {
#pragma omp single
            {
                int bundlesize = 1;
                for (int rowbundle = 0; rowbundle < renderSettings.resolution[1] / bundlesize; rowbundle++)
                {
#pragma omp task
                    {
                        size_t startrow = rowbundle * bundlesize;
                        for (size_t y = startrow; y < startrow + bundlesize; y++)
                        {
                            std::string row;
                            row.reserve(renderSettings.resolution[0] * 3 * 4);
                            for (int x = 0; x < renderSettings.resolution[0]; x++)
                            {
                                Vec3 color = kernel(x, y) * calculatedChannelDepth;
                                int r = std::min(255.0f, std::max(0.0f, color.x));
                                int g = std::min(255.0f, std::max(0.0f, color.y));
                                int b = std::min(255.0f, std::max(0.0f, color.z));
                                row.append(std::to_string(r)).append(" ").append(std::to_string(g)).append(" ").append(std::to_string(b)).append(" ");
                            }
                            rows[y] = row;
                        }
#pragma omp critical
                        {
                            rows_done += bundlesize;
                            std::cout << "\r[" << get_progress_bar((float)rows_done / renderSettings.resolution[1]) << "]" << std::flush;
                        }
                    }
                    // hier wird Gesamtzahl der gerenderten Zeilen (rows_done) aktualisiert und ein Fortschrittsbalken in Konsole angezeigt
                }
            }
        }

        std::cout << "\nRendering done in " << (omp_get_wtime() - starttime) << std::endl;
        starttime = omp_get_wtime();

        for (const string &row : rows)
        {
            ppm += row + "\n"; // alle Zeilen in rows in PPM-String zusammengefügt
        }

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

        std::cout << "Writing to file done in " << omp_get_wtime() - starttime << std::endl;

        return;
    }

    /// @param x Sub Pixel Coordinate
    /// @param y Sub Pixel Coordinate
    /// @return Light Ray that influences the pixel, direction is normalized
    LightRay
    GenerateRayFromPixel(float x, float y)
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

    // std::vector<Vec3> skybox_colors = {
    //     {0.0f, 0.02f, 0.08f},
    //     {0.3f, 0.2f, 0.5f},
    //     {0.8314f, 0.8118f, 0.7922f},
    //     {0.9331f, 0.8118f, 0.3922f},
    //     {0.8039f, 0.8667f, 0.9294f},
    //     {0.2353f, 0.2471f, 0.3686f}};

    // std::vector<float> skybox_marks = {
    //     0.0f, 0.15f, 0.46f, 0.52f, 0.6f, 1.1f};

    std::vector<Vec3> skybox_colors = {
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
    };

    std::vector<float> skybox_marks = {
        0.0f, 1.1f};

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
            {
                continue;
            }

            const float gradient_pos = (lr.direction.y * 0.5f) + 0.5f;
            return get_gradient(skybox_colors, skybox_marks, gradient_pos);
        }

        // Max Bounces
        return {1.0f, 0.0f, 0.0f};
    }

    Vec3 kernel_scattertest(int x, int y)
    {
        return FullTrace(GenerateRayFromPixel(x, y), 5, 10, 0);
    }

    // berechnet Farbe eines Pixels mit Supersampling
    // Supersampling: verbessert Bildqualität indem mehrere Strahlen pro Pixel simuliert und deren Ergebnisse dann gemittelt werden --> reduziert Treppeneffekte bei scharfen Kanten (Aliasing)
    Vec3 kernel_full(int x, int y)
    {
        Vec3 final_color;
        // innerhalb jedes Pixels mehrere Unterpixel simulieren
        for (int i = 0; i < renderSettings.supersampling_steps; i++)
        {
            for (int j = 0; j < renderSettings.supersampling_steps; j++)
            {
                // jedes Subpixel durch kleine Verschiebungen berechnet --> gleichmäßig verteilte Subpixel-Koordinaten
                float subpixel_x = (float)x + (static_cast<float>(i) / renderSettings.supersampling_steps);
                float subpixel_y = (float)y + (static_cast<float>(j) / renderSettings.supersampling_steps);
                final_color = final_color + FullTrace(GenerateRayFromPixel(subpixel_x, subpixel_y), 3, 10, 2); // Strahl für jeden Subpixel erzeugt
            }
        }
        // kumulierte Farbe durch die Gesamtzahl der Subpixel berechnet
        // berechnet Durchschnitt der Subpixelfarben um endgültige Pixelfarbe zu bestimmen
        return final_color * (1.0f / (renderSettings.supersampling_steps * renderSettings.supersampling_steps));
    }
};