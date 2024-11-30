#include <vector>
#include <iostream>
#include <omp.h>
#include <sstream>
#include <functional>

using namespace std;
using namespace m128Calc;

class Camera
{
    // Determines the color value for each pixel
    using RenderKernel = __m128 (*)(Camera *cam, int x, int y);

private:
    /// @brief Seed for randomness
    unsigned int rng_seed = 42;

    /// @brief Generates the full recursion for a single ray
    /// @param scene active scene
    /// @param bounces How many times the ray can bounce before it is interrupted
    /// @param scatter Into how many rays does the ray scatter on impact?
    /// @param scatterreduction How many scatter rays to lose after each bounce
    /// @param sceneMem Pointer to the start of baked scene memory
    /// @return
    __m128 FullTrace(LightRay lr, int bounces, int scatters, int scatterreduction, float *sceneMem)
    {
        // Check Object Collisions
        Collision closestCollision = NO_COLLISION;
        float closestDistance = 9999;
        float *closest_obj_ptr = 0;

        for (int i = 0; i < activeScene.objects.size(); i++)
        {
            float *objOffset = sceneMem + (28 * i);
            Collision c = MemoryCollision(lr, objOffset); // Kollision prüfen
            if (c.valid && c.distance < closestDistance)  // wenn Kollision gültig und Objekt näher ist als Vorherige
            {
                closestDistance = c.distance;
                closest_obj_ptr = objOffset;
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
            float intensity = *(float *)(closest_obj_ptr + 24);
            __m128 intv = _mm_set_ps1(intensity);
            __m128 intvr = _mm_set_ps1(1 - intensity);
            float diffuse = *(float *)(closest_obj_ptr + 25);

            // Scatter and bounce
            __m128 objCol = _mm_load_ps(closest_obj_ptr + 20);

            if (diffuse < 0 || bounces == 0 || scatters <= 0)
            {
                // For emissive materials, Rückgabe Emissionsfarbe
                return objCol;
            }
            // wenn Material diffus & nicht emissiv
            __m128 resColor = _mm_setzero_ps();
            for (int s = 0; s < scatters; s++)
            {
                __m128 reflected = normalized(scatter(mirrorToNormalized(closestCollision.incoming_direction, closestCollision.normal), diffuse, &rng_seed));                                                             // normalisierte Spiegelung, hinzufügen zufällige Streuung
                __m128 hit_color = _mm_add_ps(_mm_mul_ps(objCol, intvr), _mm_mul_ps(FullTrace(LightRay(closestCollision.point, reflected), bounces - 1, scatters - scatterreduction, scatterreduction, sceneMem), intv)); // Rekursion mit kleinerer bounces- und scatter-Anzahl
                resColor = _mm_add_ps(resColor, hit_color);
            }

            __m128 scatter_inv = _mm_set_ps1(1.0f / scatters);
            __m128 blended = _mm_mul_ps(resColor, scatter_inv); // ohne Streustrahlung direkt Eigenfarbe des Objektes zurückgeben

            return blended;
        }
        else // wenn keine Kollision gefunden, Farbe des Hintergrundes berechnen
        {
            float gradient_pos = (getY(lr.direction) * 0.5f) + 0.5f;
            return get_gradient(skybox_colors, skybox_marks, gradient_pos);
        }
    }

protected:
    __m128 lookDirection;
    int pixelCenterX;
    int pixelCenterY;
    RenderSettings renderSettings;
    __m128 position;
    __m128 lookAt;
    float fieldOfView;
    Scene activeScene;
    float *sceneMemory;

public:
    static __m128 kernel_full_redirect(Camera *cam, int x, int y)
    {
        return cam->kernel_full(x, y);
    }

    __m128 *skybox_colors;

    std::vector<float> skybox_marks = {
        0.0f, 0.15f, 0.46f, 0.52f, 0.6f, 1.1f};
    /// @param position World position of the camera
    /// @param lookAt World position thats in the center of the rendered image
    /// @param fieldOfView Vertical FOV of the Camera in Degrees
    Camera(const Vec3 &position, const Vec3 &lookAt, float fieldOfView, const RenderSettings &rs, const Scene &activeScene)
    {
        this->position = position.data;
        this->lookAt = lookAt.data;
        this->fieldOfView = fieldOfView;
        this->activeScene = activeScene;
        lookDirection = _mm_sub_ps(position.data, lookAt.data);
        renderSettings = rs;

        // Skybox
        skybox_colors = (__m128 *)allocate_aligned(16, 6 * 16);
        skybox_colors[0] = Vec3{0.0f, 0.02f, 0.08f}.data;
        skybox_colors[1] = Vec3{0.3f, 0.2f, 0.5f}.data;
        skybox_colors[2] = Vec3{0.8314f, 0.8118f, 0.7922f}.data;
        skybox_colors[3] = Vec3{0.9331f, 0.8118f, 0.3922f}.data;
        skybox_colors[4] = Vec3{0.8039f, 0.8667f, 0.9294f}.data;
        skybox_colors[5] = Vec3{0.2353f, 0.2471f, 0.3686f}.data;
    }

    /// @brief Renders the complete image using the given settings
    void RenderImage(RenderKernel kernel)
    {
        std::string ppm = generate_PPM_header(renderSettings);                                      // Header der PPM-Datei erstellt --> Infos wie Bildauflösung, Channel-Depth
        const __m128 calculatedChannelDepth = _mm_set_ps1((1 << renderSettings.channel_depth) - 1); // Berechnung Channel-Depth

        // Prepare Memory
        double starttime = omp_get_wtime();
        std::cout << "Starting scene bake..." << std::endl;
        sceneMemory = bake_into_memory(activeScene.objects);
        std::cout << "Baking scene done in " << omp_get_wtime() - starttime << std::endl;

        vector<std::string> rows(renderSettings.resolution[1]);    // Speicher für Zeilen des Bildes
        vector<float> load_rows(renderSettings.resolution[1] / 5); // Speicher für Berechnungslast jeder Zeile

        starttime = omp_get_wtime();

        // Compute load for each row
#pragma omp parallel for
        for (int y = 0; y < renderSettings.resolution[1] / 5; y++)
        {
            int row_start = omp_get_wtime();
            for (int x = 0; x < renderSettings.resolution[0] / 5; x++)
            {
                kernel(this, x * 5, y * 5);
            }
            load_rows[y] = omp_get_wtime() - row_start;
        }

        auto [row_numbers, sorted_loads] = sortWithIndex(load_rows); // Zeilen nach berechneter Last sortieren

        // Distribute load
        vector<vector<int>> thread_row_table = vector<vector<int>>(omp_get_max_threads()); // gibt an welche Zeilen von welchem Thread bearbeitet werden
        vector<float> thread_loads = vector<float>(omp_get_max_threads());

        // Give biggest load to least loaded thread
        for (size_t i = 0; i < row_numbers.size(); i++)
        {
            float smallestload = thread_loads[0];
            size_t leastLoadedThread = 0;
            for (size_t tnum = 1; tnum < thread_loads.size(); tnum++)
            {
                if (thread_loads[tnum] < smallestload)
                {
                    smallestload = thread_loads[tnum];
                    leastLoadedThread = tnum;
                }
            }
            thread_row_table[leastLoadedThread].push_back(row_numbers[i]);
            thread_loads[leastLoadedThread] += sorted_loads[i];
        }

        std::cout << "Load balancing done in " << (omp_get_wtime() - starttime) << std::endl;
        starttime = omp_get_wtime();

        // Compute color for each pixel
        // Zeilen in thread_row_table werden parallelisiert
        // jeder Thread rendert die ihm zugewiesenen Zeitbündel (je 5 Zeilen)
        int rows_done = 0;
#pragma omp parallel
        {
            for (int rowbundle : thread_row_table[omp_get_thread_num()])
            {
                size_t startrow = rowbundle * 5;
                for (size_t y = startrow; y < startrow + 5; y++)
                {
                    std::string row;
                    row.reserve(renderSettings.resolution[0] * 3 * 4);
                    for (int x = 0; x < renderSettings.resolution[0]; x++)
                    {
                        __m128 kernel_res = kernel(this, x, y);
                        __m128 color = _mm_mul_ps(kernel_res, calculatedChannelDepth);
                        float components[4];
                        _mm_storeu_ps(components, color);
                        int r = std::min(255, std::max(0, static_cast<int>(components[0])));
                        int g = std::min(255, std::max(0, static_cast<int>(components[1])));
                        int b = std::min(255, std::max(0, static_cast<int>(components[2])));
                        row.append(std::to_string(r)).append(" ").append(std::to_string(g)).append(" ").append(std::to_string(b)).append(" ");
                    }
                    rows[y] = row;
                }
// hier wird Gesamtzahl der gerenderten Zeilen (rows_done) aktualisiert und ein Fortschrittsbalken in Konsole angezeigt
#pragma omp critical
                {
                    rows_done += 5;
                    std::cout << "\r[" << get_progress_bar((float)rows_done / renderSettings.resolution[1]) << "]" << std::flush;
                }
            }
        }

        std::cout << "\nRendering done in " << (omp_get_wtime() - starttime) << std::endl;

        free(sceneMemory);

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
        // Direction of the camera (forward vector)
        __m128 forward = normalized(_mm_sub_ps(lookAt, position));

        // Right and up vectors for the camera coordinate system
        __m128 upReference = (abs(getY(forward)) > 0.99f) ? Vec3(0, 0, 1).data : Vec3(0, 1, 0).data;
        __m128 right = normalized(cross(forward, upReference));
        __m128 up = normalized(cross(right, forward));

        // Field of view and aspect ratio
        float aspectRatio = static_cast<float>(renderSettings.resolution[0]) / renderSettings.resolution[1];
        float fovY = std::tan((fieldOfView * 0.5f) * (M_PI / 180.0f));
        float fovX = fovY * aspectRatio;

        // Normalized device coordinates for the pixel
        float pixelNDC_X = (x + 0.5f) / renderSettings.resolution[0];
        float pixelNDC_Y = (y + 0.5f) / renderSettings.resolution[1];

        // Screen space coordinates
        float pixelScreen_X = (2.0f * pixelNDC_X - 1.0f) * fovX;
        float pixelScreen_Y = (1.0f - 2.0f * pixelNDC_Y) * fovY;

        // Prepare SIMD versions of screen coordinates
        __m128 psx = _mm_set_ps1(pixelScreen_X);
        __m128 psy = _mm_set_ps1(pixelScreen_Y);

        // Calculate the direction of the ray
        __m128 right_scaled = _mm_mul_ps(right, psx);
        __m128 up_scaled = _mm_mul_ps(up, psy);
        __m128 pixelWorld = _mm_add_ps(_mm_add_ps(forward, right_scaled), up_scaled);
        __m128 direction = normalized(pixelWorld);

        // Return the generated ray
        return LightRay{position, direction};
    }

    // RENDER KERNELS
    __m128 kernel_colorTest(int x, int y)
    {
        float r = (float)x / renderSettings.resolution[0];
        float b = (float)y / renderSettings.resolution[1];
        return _mm_setr_ps(r, 0.0f, b, 0);
    }

    __m128 kernel_rayTest(int x, int y)
    {
        LightRay lr = GenerateRayFromPixel(x, y);
        return lr.direction;
    }

    // std::vector<Vec3> skybox_colors = {
    //     {0.0f, 0.0f, 0.0f},
    //     {0.0f, 0.0f, 0.0f},
    // };

    // std::vector<float> skybox_marks = {
    //     0.0f, 1.1f};

    __m128 kernel_skyboxOnly(int x, int y)
    {
        LightRay lr = GenerateRayFromPixel(x, y);
        const float gradient_pos = (getY(lr.direction) * 0.5f) + 0.5f;

        return get_gradient(skybox_colors, skybox_marks, gradient_pos);
    }

    __m128 kernel_flatObjects(int x, int y)
    {
        LightRay lr = GenerateRayFromPixel(x, y);
        for (Object *o : activeScene.objects)
        {
            Collision c = o->CheckCollision(lr);
            if (c.valid)
            {
                return _mm_setzero_ps();
            }
        }
        const float gradient_pos = (getY(lr.direction) * 0.5f) + 0.5f;

        return get_gradient(skybox_colors, skybox_marks, gradient_pos);
    }

    __m128 kernel_normals(int x, int y)
    {
        LightRay lr = GenerateRayFromPixel(x, y);

        Collision closestCollision = NO_COLLISION;
        float closestDistance = 9999;
        float *closest_obj_ptr = 0;

        for (int i = 0; i < activeScene.objects.size(); i++)
        {
            float *objOffset = sceneMemory + (28 * i);
            Collision c = MemoryCollision(lr, objOffset); // Kollision prüfen
            if (c.valid && c.distance < closestDistance)  // wenn Kollision gültig und Objekt näher ist als Vorherige
            {
                closestDistance = c.distance;
                closest_obj_ptr = objOffset;
                closestCollision = c;
            }
            else
            {
                continue;
            }
        }

        if (closestCollision.valid)
        {
            __m128 half = _mm_set1_ps(0.5);
            __m128 col = _mm_add_ps(_mm_mul_ps(closestCollision.normal, half), half);
            return col;
        }

        const float gradient_pos = (getY(lr.direction) * 0.5f) + 0.5f;

        return get_gradient(skybox_colors, skybox_marks, gradient_pos);
    }

    /// @brief No scattering, no absorbsion, ten bounces
    __m128 kernel_supershiny(int x, int y)
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
                    __m128 reflected = mirrorToNormalized(c.incoming_direction, c.normal);
                    lr = LightRay(c.point, reflected);
                    hit = true;
                    break;
                }
            }
            if (hit)
            {
                continue;
            }

            const float gradient_pos = (getY(lr.direction) * 0.5f) + 0.5f;
            return get_gradient(skybox_colors, skybox_marks, gradient_pos);
        }

        // Max Bounces
        return _mm_setzero_ps();
    }

    __m128 kernel_flatColors(int x, int y)
    {
        return FullTrace(GenerateRayFromPixel(x, y), 0, 0, 0, sceneMemory);
    }

    __m128 kernel_scattertest(int x, int y)
    {
        return FullTrace(GenerateRayFromPixel(x, y), 5, 10, 4, sceneMemory);
    }

    // berechnet Farbe eines Pixels mit Supersampling
    // Supersampling: verbessert Bildqualität indem mehrere Strahlen pro Pixel simuliert und deren Ergebnisse dann gemittelt werden --> reduziert Bildrauschen und Treppeneffekte bei scharfen Kanten (Aliasing)
    __m128 kernel_full(int x, int y)
    {
        __m128 final_color;
        float step_width = 1.0f / renderSettings.supersampling_steps;
        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);
        // innerhalb jedes Pixels mehrere Unterpixel simulieren
        for (float i = 0; i <= 1 - step_width + 0.001f; i += step_width)
        {
            for (float j = 0; j <= 1 - step_width + 0.001f; j += step_width)
            {
                // jedes Subpixel durch kleine Verschiebungen berechnet --> gleichmäßig verteilte Subpixel-Koordinaten
                float subpixel_x = fx + i;
                float subpixel_y = fy + j;
                final_color = _mm_add_ps(final_color, FullTrace(GenerateRayFromPixel(subpixel_x, subpixel_y), 0, 0, 0, sceneMemory)); // Strahl für jeden Subpixel erzeugt
                // final_color = _mm_add_ps(final_color, FullTrace(GenerateRayFromPixel(subpixel_x, subpixel_y), 3, 5, 2, sceneMemory)); // Strahl für jeden Subpixel erzeugt
            }
        }
        // kumulierte Farbe durch die Gesamtzahl der Subpixel berechnet
        // berechnet Durchschnitt der Subpixelfarben um endgültige Pixelfarbe zu bestimmen
        __m128 div = _mm_set1_ps(renderSettings.supersampling_steps * renderSettings.supersampling_steps);
        return _mm_div_ps(final_color, div);
    }
};