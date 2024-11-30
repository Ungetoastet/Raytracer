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
char *bake_into_memory(std::vector<Object *> &objectsInScene)
{
    size_t objectCount = objectsInScene.size();
    char *memory_start = allocate_aligned(16, 108 * objectCount); // void* arithmetic causes warnings, use char* instead
    for (int i = 0; i < objectCount; i++)
    {
        int object_memory_start = 108 * i;

        std::memcpy(memory_start + object_memory_start, &(objectsInScene[i]->object_type), 1);
        std::memcpy(memory_start + object_memory_start + 4, &(objectsInScene[i]->position), 16);
        std::memcpy(memory_start + object_memory_start + 20, &(objectsInScene[i]->scale), 16);

        // Other object data
        if (objectsInScene[i]->object_type == 1) // Plane
        {
            // Normal
            std::memcpy(memory_start + object_memory_start + 36, &(((Plane *)objectsInScene[i])->normal), 16);
            // Local X
            std::memcpy(memory_start + object_memory_start + 52, &(((Plane *)objectsInScene[i])->localX), 16);
            // Local Y
            std::memcpy(memory_start + object_memory_start + 68, &(((Plane *)objectsInScene[i])->localY), 16);
        }

        // Material color
        std::memcpy(memory_start + object_memory_start + 84, &(objectsInScene[i]->mat.color), 16);

        // Material intensity
        std::memcpy(memory_start + object_memory_start + 100, &(objectsInScene[i]->mat.intensity), 4);

        // Material diffuse
        std::memcpy(memory_start + object_memory_start + 104, &(objectsInScene[i]->mat.diffuse), 4);
    }
    return memory_start;
}