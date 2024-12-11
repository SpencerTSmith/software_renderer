# Overview
Little software renderer, from scratch with minimal dependencies (SDL2 - for rendering pixel buffer to screen, stb_image - loading textures from pngs), make system is gcc only for now. There is an old branch with fewer features that comes as a visual studio project.

## Features
- Software rasterisation
- Custom linear algebra functions
- Backface-culling
- Frustum clipping
- Flat shading for untextured objects
- Perspective correct texture interpolation (Barycentric Weight)
- Fully functioned camera, including freelook and 6-directional movement

## Build
Need SDL2 and stb_image
```
make run
```
or
```
make fast
```
for optimized version

## Controls
- w, a, s, d for movement
- SPACE, c for up and down
- arrow keys for free look
- 1 for wire frame rendering
- 2 for wire + vertex highlight rendering
- 3 for filled triangles with shading
- 4 for wire + textured triangles
- 5 for textured triangles
- b to switch off and on backface-culling
