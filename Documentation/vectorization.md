# Pre - Vectorized Performance

| Run 1 | Run 2 | Run 3 |
| ----- | ----- | ----- |
| 11.4s | 11.1s | 10.5s |

# Simple Vectorization

Vectorization via use of the xmmintrin.h of the operations of the Vec3 struct:

| Run 1 | Run 2 | Run 3 |
| ----- | ----- | ----- |
| 9.45s | 9.48s | 9.59s |

Small improvements, but the conversion between Vec3 struct and \_m128 data type is unnecessary,
