# Untitled
A custom 3D game engine made from scratch in C with uv lightmapping, path traced audio, physics, and more. Independently developed entirely on a mobile 1050 to bring high-fidelity graphics to constrained hardware.

## Features
- **Rendering:** Clustered forward shading, GPU-driven rendering (MDI), indirect lighting probe volumes, reflection probes, and a software path-traced UV lightmapper using BVH traversal on the GPU.
- **Audio:** Path-traced audio probe volumes computed offline via Schroeder integration, rotated in real-time, convolved with an HRTF, and partitioned for overlap-add as it approaches the audio thread.
- **Physics:** 3D physics component pipeline featuring incremental hull generation, GJK/EPA collision detection, clipped manifolds, and BVH acceleration structures.
- **Systems:** ECS, UI and text rendering, skeletal animations, screen-space effects, PBR, and an editor with full undo/redo capabilities via custom linked lists and byte-offset pointer arithmetic.

## Included Tech
- **Language:** C
- **Graphics:** OpenGL, GLSL
- **Libraries:** SDL2 (Windowing), miniaudio (Spatialization), FFTW (Audio FFT), Nuklear (Debug UI), cJSON (Serialization), uthash (Hashing), stb (Image Loading)

## Building and Running

### Prerequisites
***Currently*** only Windows is supported due to directory parsing.
You will need:
- `SDL2`
- `FFTW3`
- `GLEW`

### Build
```bash
make
```
Place glew32.dll, SDL2.dll, and libfftw3f-f.dll alongside the binary in bin/. If you have any further trouble, try using the pkg-config included in the makefile.

For inputs and controls, refer to input.c.
