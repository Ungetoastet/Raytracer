#include <stdlib.h>
#include <string.h>

/// @brief Bakes the objects inside the scene into memory
/// @param objectsInScene Scene Objects in OOP
/// @return Start of memory block
char *bake_into_memory(std::vector<Object *> &objectsInScene)
{
    size_t objectCount = objectsInScene.size();
    char *memory_start = (char *)malloc(81 * objectCount); // void* arithmetic causes warnings, use char* instead
    for (int i = 0; i < objectCount; i++)
    {
        int object_memory_start = 81 * i;

        // Object type
        std::memcpy(memory_start + object_memory_start, &(objectsInScene[i]->object_type), 1);
        // Object position
        std::memcpy(memory_start + object_memory_start + 1, &(objectsInScene[i]->position), 12);
        // Size
        std::memcpy(memory_start + object_memory_start + 13, &(objectsInScene[i]->scale), 12);

        // Other object data
        if (objectsInScene[i]->object_type == 1) // Plane
        {
            // Normal
            std::memcpy(memory_start + object_memory_start + 25, &(((Plane *)objectsInScene[i])->normal), 12);
            // Local X
            std::memcpy(memory_start + object_memory_start + 37, &(((Plane *)objectsInScene[i])->localX), 12);
            // Local Y
            std::memcpy(memory_start + object_memory_start + 49, &(((Plane *)objectsInScene[i])->localY), 12);
        }

        // Material color
        std::memcpy(memory_start + object_memory_start + 61, &(objectsInScene[i]->mat.color.x), 12);

        // Material intensity
        std::memcpy(memory_start + object_memory_start + 73, &(objectsInScene[i]->mat.intensity), 4);

        // Material diffuse
        std::memcpy(memory_start + object_memory_start + 77, &(objectsInScene[i]->mat.color), 4);
    }
    return memory_start;
}