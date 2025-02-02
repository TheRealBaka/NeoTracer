# NeoTracer: A stroll through the blinding lights of Cyber City

NeoTracer is a ray tracer that we created, written on top of _lightwave_ for Computer Graphics I course (Winter 2024/25). Besides 
basic features for a typical ray tracer, neotracer also supports volumetric path tracing with MIS support.

# lightwave

Welcome to _lightwave_, an educational framework for writing ray tracers that can render photo-realistic images!
Lightwave provides the boring boilerplate, so you can focus on writing the insightful parts.
It aims to be minimal enough to remain comprehensible, yet flexible enough to provide a solid foundation even for sophisticated rendering algorithms.

## What's Included
Out of the box, lightwave is unable to produce any images, as it lacks all necessary rendering functionality to do so.
It is your job to write the various components that make this possible: You will write camera models, intersect shapes, program their appearance, and orchestrate how rays are traced throughout the virtual scene.
Lightwave supports you in this endeavour by supplying tedious to implement boilerplate, including:

* Modularity
  * Modern APIs flexible enough for sophisticated algorithms
  * Shapes, materials, etc are implemented as plugins
  * Whatever you can think of, you can make a plugin for it
* Basic math library
  * Vector operations
  * Matrix operations
  * Transforms
* File I/O
  * An XML parser for the lightwave scene format
  * Reading and writing various image formats
  * Reading and representing triangle meshes
  * Streaming images to the [tev](https://github.com/Tom94/tev) image viewer
* Multi-threading
  * Rendering is parallelized across all available cores
  * Parallelized scene loading (image loading, BVH building, etc)
* BVH acceleration structure
  * Data-structure and traversal is supplied by us
  * Split-in-the-middle build is supplied as well
  * It's your job to implement more sophisticated building
* Useful utilities
  * Thread-safe logger functionality
  * Assert statements that provide extra context
  * An embedded profiler to identify bottlenecks of your code
  * Random number generators
* A Blender exporter
  * You can easily build and render your own scenes

## Additional features

Besides the assignments, the following features were also implemented:
* Basic features
  * Shading normals: `tests/basic_features/shading_normals.xml`, `tests/basic_features/normal_map.xml`.
  * Image denoising: `tests/basic_features/image_denoising.xml`.
  * Rough dielectric: `tests/basic_features/bunny_frosted.xml` (compare with `tests/basic_features/bunny_glass.xml` with glass)
  * Area lights: `tests/basic_features/bunny_area.xml` (compare with `tests/basic_features/bunny_point.xml`)
  * Improved area lights: `tests/basic_features/bunny_area_improved.xml` (compare with `tests/basic_features/bunny_area.xml`)
  * Bloom: `tests/basic_features/bloom.xml`
* Advanced features
  * Bunny fog (homogeneous participating medium): `tests/advanced_features/bunny_fog.xml`
  * Room fog (homogeneous participating medium): `tests/advanced_features/room_fog.xml`
  * MIS path tracer: `tests/advanced_features/mis.xml` (compare with `tests/advanced_features/no-mis.xml`). We try the path tracer on test based on Eric Veach's thesis.

## Contributors
Lightwave was written by [Alexander Rath](https://graphics.cg.uni-saarland.de/people/rath.html), with contributions from [Ömercan Yazici](https://graphics.cg.uni-saarland.de/people/yazici.html) and [Philippe Weier](https://graphics.cg.uni-saarland.de/people/weier.html).
Many of our design decisions were heavily inspired by [Nori](https://wjakob.github.io/nori/), a great educational renderer developed by Wenzel Jakob.
We would also like to thank the teams behind our dependencies: [ctpl](https://github.com/vit-vit/CTPL), [miniz](https://github.com/richgel999/miniz), [stb](https://github.com/nothings/stb), [tinyexr](https://github.com/syoyo/tinyexr), [tinyformat](https://github.com/c42f/tinyformat), [pcg32](https://github.com/wjakob/pcg32), and [catch2](https://github.com/catchorg/Catch2).
