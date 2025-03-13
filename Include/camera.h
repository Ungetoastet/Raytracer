#include <vector>
#include <iostream>
#include <omp.h>
#include <sstream>
#include <functional>

#include <smmintrin.h>

using namespace std;
using namespace m128Calc;

class Scene;

class Camera
{
    // Determines the color value for each pixel
    using RenderKernel = __m128 (*)(Camera *cam, int x, int y);

private:
    /// @brief Seed vector for randomness
    __m128i rng_seed;

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
        float closestDistance = INFINITY;
        float *closest_obj_ptr = 0;

        for (size_t i = 0; i < activeScene.objects.size(); i++)
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
            float intensity = *(closest_obj_ptr + 24);
            float diffuse = *(closest_obj_ptr + 25);

            // Scatter and bounce
            __m128 objCol = _mm_load_ps(closest_obj_ptr + 20);

            if (diffuse < 0)
            {
                // For emissive materials, Rückgabe Emissionsfarbe
                return objCol;
            }
            // wenn Material diffus & nicht emissiv
            __m128 resColor = _mm_setzero_ps();
            if (bounces == 0)
            {
                return resColor;
            }

            // Branchless: Sorgt dafuer, dass scatter immer mind 1 ist
            scatters = scatters * (scatters > 0) + (scatters <= 0);

            __m128 col_specular = _mm_setzero_ps();
            __m128 col_diffuse = _mm_setzero_ps();

            // Ausrechnen, wie viele scatter-rays reflektiert werden. Der Rest wird diffus gestreut
            int scatters_specular = (int)(scatters * intensity) + 1;

            int s = 0;
            // Spiegel-Reflektion
            for (; s < scatters_specular; s++)
            {
                __m128 specular_reflected = normalized(
                    scatter(
                        mirrorToNormalized(
                            closestCollision.incoming_direction,
                            closestCollision.normal),
                        diffuse,
                        rng_seed)); // Spiegelung plus zufällige Streuung

                __m128 hit_color = _mm_mul_ps(
                    objCol,
                    FullTrace(LightRay(closestCollision.point, specular_reflected),
                              bounces - 1,
                              scatters - scatterreduction,
                              scatterreduction,
                              sceneMem)); // Rekursion mit kleinerer bounces- und scatter-Anzahl
                col_specular = _mm_add_ps(col_specular, hit_color);
            }

            // Diffuse Reflektion
            for (; s < scatters + 1; s++)
            {
                __m128 diffuse_reflected = diffuseScatter(closestCollision.normal, rng_seed);

                __m128 hit_color = _mm_mul_ps(
                    objCol,
                    FullTrace(LightRay(closestCollision.point, diffuse_reflected),
                              bounces - 1,
                              scatters - scatterreduction,
                              scatterreduction,
                              sceneMem)); // Rekursion mit kleinerer bounces- und scatter-Anzahl
                col_diffuse = _mm_add_ps(col_diffuse, hit_color);
            }

            // Reflektierte Strahlen mitteln
            col_specular = _mm_mul_ps(col_specular, _mm_set1_ps(1.0f / scatters_specular));
            col_diffuse = _mm_mul_ps(col_diffuse, _mm_set1_ps(1.0f / (1 + scatters - scatters_specular)));

            // Diffuse und Reflektions Anteile mischen
            resColor = _mm_add_ps(
                _mm_mul_ps(col_specular, _mm_set1_ps(intensity)),
                _mm_mul_ps(col_diffuse, _mm_set1_ps(1 - intensity)));

            return resColor;
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
    int bounces;
    int scatterCount;
    int scatterRedux;

public:
    __m128 *skybox_colors;

    std::vector<float> skybox_marks = {
        0.0f, 0.15f, 0.46f, 0.52f, 0.6f, 1.1f};
    /// @param position World position of the camera
    /// @param lookAt World position thats in the center of the rendered image
    /// @param fieldOfView Vertical FOV of the Camera in Degrees
    Camera(const Vec3 &position, const Vec3 &lookAt, float fieldOfView, const RenderSettings &rs, const Scene &activeScene, bool skybox)
    {
        this->position = position.data;
        this->lookAt = lookAt.data;
        this->fieldOfView = fieldOfView;
        this->activeScene = activeScene;
        lookDirection = _mm_sub_ps(position.data, lookAt.data);
        renderSettings = rs;
        bounces = rs.bounces;
        scatterCount = rs.scatterbase;
        scatterRedux = rs.scatterredux;

        rng_seed = _mm_set_epi32(82, 42, 69, 2004);

        // Skybox
        if (skybox)
        {
            skybox_colors = (__m128 *)allocate_aligned(16, 6 * 16);
            skybox_colors[0] = Vec3{0.0f, 0.02f, 0.08f}.data;
            skybox_colors[1] = Vec3{0.3f, 0.2f, 0.5f}.data;
            skybox_colors[2] = Vec3{0.8314f, 0.8118f, 0.7922f}.data;
            skybox_colors[3] = Vec3{0.9331f, 0.8118f, 0.3922f}.data;
            skybox_colors[4] = Vec3{0.8039f, 0.8667f, 0.9294f}.data;
            skybox_colors[5] = Vec3{0.2353f, 0.2471f, 0.3686f}.data;
        }
        else
        {
            skybox_marks = {0.0f, 1.1f};
            skybox_colors = (__m128 *)allocate_aligned(16, 2 * 16);
            skybox_colors[0] = Vec3{0.0f, 0.0f, 0.0f}.data;
            skybox_colors[1] = Vec3{0.0f, 0.0f, 0.0f}.data;
        }
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

        vector<std::string> rows(renderSettings.resolution[1]); // Speicher für Zeilen des Bildes

        starttime = omp_get_wtime();

        // Allocate memory for imagedata
        __m128 **imageData = (__m128 **)malloc(renderSettings.resolution[0] * sizeof(__m128 *));
        __m128 **smoothedImageData = (__m128 **)malloc(renderSettings.resolution[0] * sizeof(__m128 *));
        for (int x = 0; x < renderSettings.resolution[0]; x++)
        {
            imageData[x] = (__m128 *)malloc(renderSettings.resolution[1] * sizeof(__m128));
            smoothedImageData[x] = (__m128 *)malloc(renderSettings.resolution[1] * sizeof(__m128));
        }

        // Compute color for each pixel
#pragma omp parallel for collapse(2)
        for (int y = 0; y < renderSettings.resolution[1]; y++)
        {
            for (int x = 0; x < renderSettings.resolution[0]; x++)
            {
                __m128 kernel_res = kernel(this, x, y);
                imageData[x][y] = _mm_mul_ps(kernel_res, calculatedChannelDepth);
            }
        }

        std::cout << "Rendering done in " << (omp_get_wtime() - starttime) << std::endl;

        if (renderSettings.smoothing)
        {
            starttime = omp_get_wtime();

            // 3x3 Gaussian blur kernel
            const __m128 gaussianKernel[3][3] = {
                {_mm_set1_ps(1.0f / 16), _mm_set1_ps(2.0f / 16), _mm_set1_ps(1.0f / 16)},
                {_mm_set1_ps(2.0f / 16), _mm_set1_ps(4.0f / 16), _mm_set1_ps(2.0f / 16)},
                {_mm_set1_ps(1.0f / 16), _mm_set1_ps(2.0f / 16), _mm_set1_ps(1.0f / 16)}};

#pragma omp parallel for collapse(2)
            for (int y = 1; y < renderSettings.resolution[1] - 1; y++)
            {
                for (int x = 1; x < renderSettings.resolution[0] - 1; x++)
                {
                    __m128 sum = _mm_setzero_ps();

                    for (int ky = -1; ky <= 1; ky++)
                    {
                        for (int kx = -1; kx <= 1; kx++)
                        {
                            __m128 pixel = imageData[x + kx][y + ky];
                            sum = _mm_add_ps(sum, _mm_mul_ps(pixel, gaussianKernel[ky + 1][kx + 1]));
                        }
                    }

                    smoothedImageData[x][y] = sum;
                }
            }

            std::cout << "Smoothing done in " << (omp_get_wtime() - starttime) << std::endl;
        }

        starttime = omp_get_wtime();

        // Bilddaten zu Strings wandeln, Zeilen in werden parallelisiert
#pragma omp parallel
        {
            std::vector<char> buffer(renderSettings.resolution[0] * 12); // Preallocate thread-local buffer
            char *buf_ptr;

#pragma omp for
            for (int y = 0; y < renderSettings.resolution[1]; y++)
            {
                buf_ptr = buffer.data();
                for (int x = 0; x < renderSettings.resolution[0]; x++)
                {
                    float components[4];
                    if (renderSettings.smoothing)
                    {
                        _mm_storeu_ps(components, smoothedImageData[x][y]);
                    }
                    else
                    {
                        _mm_storeu_ps(components, imageData[x][y]);
                    }

                    int r = std::min(255, std::max(0, static_cast<int>(components[0])));
                    int g = std::min(255, std::max(0, static_cast<int>(components[1])));
                    int b = std::min(255, std::max(0, static_cast<int>(components[2])));

                    buf_ptr += std::snprintf(buf_ptr, 12, "%d %d %d ", r, g, b);
                }
                rows[y] = std::string(buffer.data(), buf_ptr - buffer.data());
            }
        }
        std::cout << "Stringing done in " << (omp_get_wtime() - starttime) << std::endl;

        // Free all allocated memory
        for (int x = 0; x < renderSettings.resolution[0]; x++)
        {
            free(imageData[x]);
            free(smoothedImageData[x]);
        }

        free(imageData);
        free(smoothedImageData);

        free_aligned(sceneMemory);
        free_aligned(skybox_colors);

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
        float fovY = std::tan((fieldOfView * 0.5f) * (3.1415926535f / 180.0f));
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
        __m128 pixelWorld = _mm_fmadd_ps(right, psx, _mm_fmadd_ps(up, psy, forward));
        __m128 direction = normalized(pixelWorld);

        // Return the generated ray
        return LightRay{position, direction};
    }

    // RENDER KERNELS
    static __m128 kernel_colorTest(Camera *cam, int x, int y)
    {
        float r = (float)x / cam->renderSettings.resolution[0];
        float b = (float)y / cam->renderSettings.resolution[1];
        return _mm_setr_ps(r, 0.0f, b, 0);
    }

    static __m128 kernel_rayTest(Camera *cam, int x, int y)
    {
        LightRay lr = cam->GenerateRayFromPixel(x, y);
        return lr.direction;
    }

    static __m128 kernel_skyboxOnly(Camera *cam, int x, int y)
    {
        LightRay lr = cam->GenerateRayFromPixel(x, y);
        const float gradient_pos = (getY(lr.direction) * 0.5f) + 0.5f;

        return get_gradient(cam->skybox_colors, cam->skybox_marks, gradient_pos);
    }

    static __m128 kernel_flatObjects(Camera *cam, int x, int y)
    {
        LightRay lr = cam->GenerateRayFromPixel(x, y);
        for (Object *o : cam->activeScene.objects)
        {
            Collision c = o->CheckCollision(lr);
            if (c.valid)
            {
                return _mm_setzero_ps();
            }
        }
        const float gradient_pos = (getY(lr.direction) * 0.5f) + 0.5f;

        return get_gradient(cam->skybox_colors, cam->skybox_marks, gradient_pos);
    }

    static __m128 kernel_normals(Camera *cam, int x, int y)
    {
        LightRay lr = cam->GenerateRayFromPixel(x, y);

        Collision closestCollision = NO_COLLISION;
        float closestDistance = 9999;

        for (size_t i = 0; i < cam->activeScene.objects.size(); i++)
        {
            float *objOffset = cam->sceneMemory + (28 * i);
            Collision c = MemoryCollision(lr, objOffset); // Kollision prüfen
            if (c.valid && c.distance < closestDistance)  // wenn Kollision gültig und Objekt näher ist als Vorherige
            {
                closestDistance = c.distance;
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

        return get_gradient(cam->skybox_colors, cam->skybox_marks, gradient_pos);
    }

    /// @brief No scattering, no absorbsion, ten bounces
    static __m128 kernel_supershiny(Camera *cam, int x, int y)
    {
        LightRay lr = cam->GenerateRayFromPixel(x, y);
        for (int bounce = 0; bounce <= 10; bounce++)
        {
            bool hit = false;
            for (Object *o : cam->activeScene.objects)
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
            return get_gradient(cam->skybox_colors, cam->skybox_marks, gradient_pos);
        }

        // Max Bounces
        return _mm_setzero_ps();
    }

    static __m128 kernel_flatColors(Camera *cam, int x, int y)
    {
        __m128 final_color = _mm_setzero_ps();
        float step_width = 1.0f / cam->renderSettings.supersampling_steps;
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
                final_color = _mm_add_ps(final_color, cam->FullTrace(cam->GenerateRayFromPixel(subpixel_x, subpixel_y), 0, 0, 0, cam->sceneMemory)); // Strahl für jeden Subpixel erzeugt
            }
        }
        // kumulierte Farbe durch die Gesamtzahl der Subpixel berechnet
        // berechnet Durchschnitt der Subpixelfarben um endgültige Pixelfarbe zu bestimmen
        __m128 div = _mm_set1_ps(1 / (cam->renderSettings.supersampling_steps * cam->renderSettings.supersampling_steps));
        return _mm_mul_ps(final_color, div);
    }

    static __m128 kernel_scattertest(Camera *cam, int x, int y)
    {
        return cam->FullTrace(cam->GenerateRayFromPixel(x, y), 5, 10, 4, cam->sceneMemory);
    }

    // berechnet Farbe eines Pixels mit Supersampling
    // Supersampling: verbessert Bildqualität indem mehrere Strahlen pro Pixel simuliert und deren Ergebnisse dann gemittelt werden --> reduziert Bildrauschen und Treppeneffekte bei scharfen Kanten (Aliasing)
    static __m128 kernel_full(Camera *cam, int x, int y)
    {
        __m128 final_color = _mm_setzero_ps();
        int steps = cam->renderSettings.supersampling_steps;
        float step_width = 1.0f / steps;
        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);

        for (int i = 0; i < steps; i++)
        {
            float subpixel_offset_x = fx + (i + 0.5f) * step_width;
            for (int j = 0; j < steps; j++)
            {
                float subpixel_offset_y = fy + (j + 0.5f) * step_width;

                LightRay subpixel_ray = cam->GenerateRayFromPixel(subpixel_offset_x, subpixel_offset_y);
                __m128 subpixel_color = cam->FullTrace(subpixel_ray, cam->bounces, cam->scatterCount, cam->scatterRedux, cam->sceneMemory);

                final_color = _mm_add_ps(final_color, subpixel_color);
            }
        }

        __m128 div = _mm_set1_ps(1.0f / (steps * steps));
        return _mm_mul_ps(final_color, div);
    }
};