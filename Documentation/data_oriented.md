# Performance with OOP

OOP casting in the render loop is very expensive.

Render time:

| Run 1 | Run 2 | Run 3 |
| ----- | ----- | ----- |
| 20s   | 23s   | 23s   |

# Idea: Data Oriented Render Cycle

Don't use Objects and classes inside the render loop.
Instead, bake the Objects into Memory, free from the OOP Structure.
We then know exactly where our data is inside memory and can skip casting and variable accessing.

Size needed in Memory for each Object:

| Information             | Memory Size in Bytes         | Memory Address |
| ----------------------- | ---------------------------- | -------------- |
| Object Type ID          | 1 (char)                     | 0              |
| Object Position         | 4 (Vector) \* 4 (float)      | 4              |
| Object Scale            | 4 (Vector) \* 4 (float)      | 20             |
| Other Object Attributes | 3 \* 4 (Vector) \* 4 (float) | 36, 52, 68     |
| Material: Color         | 4 (Vector) \* 4 (float)      | 84             |
| Material: intensity     | 4 (float)                    | 100            |
| Material: diffuse       | 4 (float)                    | 104            |
| **SUM**                 | **108**                      |                |

Object Type IDs:

0. Sphere
1. Plane

# Performance with Data Oriented Render Cycle

| Run 1 | Run 2 | Run 3 |
| ----- | ----- | ----- |
| 11.4s | 11.1s | 10.5s |

# New Data Structure after Vectorization

| Information             | Memory Size in Bytes         | Memory Address              |
| ----------------------- | ---------------------------- | --------------------------- |
| Object Position         | 4 (Vector) \* 4 (float)      | 0                           |
| Object Scale            | 4 (Vector) \* 4 (float)      | 16 (4F)                     |
| Other Object Attributes | 3 \* 4 (Vector) \* 4 (float) | 32 (8F), 48 (12F), 64 (16F) |
| Material: Color         | 4 (Vector) \* 4 (float)      | 80 (20F)                    |
| Material: intensity     | 4 (float)                    | 96 (24F)                    |
| Material: diffuse       | 4 (float)                    | 100 (25F)                   |
| Object Type ID          | 1 (char)                     | 104 (26F)                   |
| **SUM**                 | **112**                      | (28F)                       |

_112 for 16-byte alignment._
