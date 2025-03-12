#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <memory>

// Cross-platform aligned allocation
char *allocate_aligned(size_t alignment, size_t size)
{
#ifdef _WIN32
    return static_cast<char *>(_aligned_malloc(size, alignment)); // Use _aligned_malloc on Windows
#else
    return static_cast<char *>(aligned_alloc(alignment, size)); // Use aligned_alloc on Linux
#endif
}

// Cross-platform aligned free
void free_aligned(void *ptr)
{
#ifdef _WIN32
    _aligned_free(ptr); // Use _aligned_free on Windows
#else
    free(ptr); // Use free on Linux
#endif
}

/// @brief Bakes the objects inside the scene into memory
/// @param objectsInScene Scene Objects in OOP
/// @return Start of memory block
float *bake_into_memory(std::vector<Object *> &objectsInScene)
{
    size_t objectCount = objectsInScene.size();
    float *memory_start = (float *)allocate_aligned(16, 112 * objectCount); // void* arithmetic causes warnings, use float* instead
    for (size_t i = 0; i < objectCount; i++)
    {
        float *object_memory_start = memory_start + 28 * i;

        _mm_store_ps(object_memory_start, objectsInScene[i]->position);
        _mm_store_ps(object_memory_start + 4, objectsInScene[i]->scale);

        // Other object data
        if (objectsInScene[i]->object_type == 1) // Plane
        {
            // Normal
            _mm_store_ps(object_memory_start + 8, ((Plane *)(objectsInScene[i]))->normal);
            // Local X
            _mm_store_ps(object_memory_start + 12, ((Plane *)(objectsInScene[i]))->localX);
            // Local Y
            _mm_store_ps(object_memory_start + 16, ((Plane *)(objectsInScene[i]))->localY);
        }

        // Material color
        _mm_store_ps(object_memory_start + 20, objectsInScene[i]->mat.color);

        // Material intensity
        std::memcpy(object_memory_start + 24, &(objectsInScene[i]->mat.intensity), 4);

        // Material diffuse
        std::memcpy(object_memory_start + 25, &(objectsInScene[i]->mat.diffuse), 4);

        // Obj type
        std::memcpy(object_memory_start + 26, &(objectsInScene[i]->object_type), 1);
    }
    return memory_start;
}