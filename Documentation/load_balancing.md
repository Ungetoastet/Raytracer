# Render Settings

All tests were conducted after scattering was implemented.
No materials where used and 5 bounces and 10 scatters for every ray.

# Naive Approach

The naive approach parallizes the rendering of the pixel rows of the image.

Kernel_Scattertest, serial:
**6.0s**

Test with Scattering on 6 Threads:
**4.2s**

This works fine, but the load is really unbalanced, as there are many rows where all pixels end in the skybox without any collision.
In contrast, the rays of other pixels reflect and scatter many times, resulting in huge loads.
This leaves some cores empty and others at full capacity.

# Ray based approach

Each ray gets parallized. This solves the balancing problem resulting from scattering and bouncing.
BUT: **Massive** parallization overhead:

Ray based approach:
**45.5s**

# Collapsed Render Loops

Instead of every row being one parallisation unit, parallize every pixel.
This doesnt solve the unbalanced load caused by scattering and bouncing but speeds up execution a bit.

Pixel based approach:
**3.4s**

# Weighted Row Approach

We render a low-resolution image using the naive parallisation approach and record the complexity for each row of the image.
Then we can assign each thread a group of rows while balancing the load ourselves using a greedy algorithm.
