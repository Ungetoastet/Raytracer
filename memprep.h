#include <stdlib.h>
#include <string.h>

/// @brief Bakes the objects inside the scene into memory
/// @param objectsInScene Scene Objects in OOP
/// @return Start of memory block
float *bake_into_memory(std::vector<Object *> &objectsInScene)
{
    size_t objectCount = objectsInScene.size();
    float *memory_start = (float *)aligned_alloc(16, 108 * objectCount); // void* arithmetic causes warnings, use float* instead
    for (int i = 0; i < objectCount; i++)
    {
        int object_memory_start = 108 * i;

        std::memcpy(memory_start + object_memory_start, &(objectsInScene[i]->position), 16);
        std::memcpy(memory_start + object_memory_start + 16, &(objectsInScene[i]->scale), 16);

        // Other object data
        if (objectsInScene[i]->object_type == 1) // Plane
        {
            // Normal
            std::memcpy(memory_start + object_memory_start + 32, &(((Plane *)objectsInScene[i])->normal), 16);
            // Local X
            std::memcpy(memory_start + object_memory_start + 48, &(((Plane *)objectsInScene[i])->localX), 16);
            // Local Y
            std::memcpy(memory_start + object_memory_start + 64, &(((Plane *)objectsInScene[i])->localY), 16);
        }

        // Material color
        std::memcpy(memory_start + object_memory_start + 80, &(objectsInScene[i]->mat.color), 16);

        // Material intensity
        std::memcpy(memory_start + object_memory_start + 96, &(objectsInScene[i]->mat.intensity), 4);

        // Material diffuse
        std::memcpy(memory_start + object_memory_start + 100, &(objectsInScene[i]->mat.diffuse), 4);

        // Obj type
        std::memcpy(memory_start + object_memory_start, &(objectsInScene[i]->object_type), 1);
    }
    return memory_start;
}