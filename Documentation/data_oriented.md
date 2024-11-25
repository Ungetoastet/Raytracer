# Performance with OOP

OOP casting in the render loop is very expensive.

Render time:
20s, 23s, 23s

# Idea: Data Oriented Render Cycle

Don't use Objects and classes inside the render loop.
Instead, bake the Objects into Memory, free from the OOP Structure.
We then know exactly where our data is inside memory and can skip casting and variable accessing.

Size needed in Memory for each Object:

| Information             | Memory Size in Bytes         | Memory Address |
| ----------------------- | ---------------------------- | -------------- |
| Object Type ID          | 1 (char)                     | 0              |
| Object Position         | 3 (Vector) \* 4 (float)      | 1              |
| Object Scale            | 3 (Vector) \* 4 (float)      | 13             |
| Other Object Attributes | 3 \* 3 (Vector) \* 4 (float) | 25, 37, 49     |
| Material: Color         | 3 (Vector) \* 4 (float)      | 61             |
| Material: intensity     | 4 (float)                    | 73             |
| Material: diffuse       | 4 (float)                    | 77             |
| **SUM**                 | **81**                       |                |

Object Type IDs:

0. Sphere
1. Plane
