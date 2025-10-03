# GLSL Sandbox
Explorations and notes around shaders written in GLSL (openGL Shading Language).

Shaders are functions running on the gpu that operate on ever pixel/vertice/etc. of data. On a 1920x1080 screen for example, a pixel shader is executed 2,073,600 times, once for each pixel. Because of the large quantity of work being performed, processing is typically performed on the gpu, where much larger batches of work can take place. This kind of parallelization requires certain restrictions that are foreign to standard programming practices (i.e. you don't always have a way to get at the data you want or think you should be able to access).

All files in this folder should be executable via the "shadertoy" vscode plugin or by copy pasting into shadertoy.

## Concepts/Terms/Technologies
 * Swizzling - Aliasing/field masks for vectors, ex: `my_vec.rgb` or `my_vec.xyz` or `my_vec.zyx` or `my_vec.x`. Note, not just name aliasing, can also be used to swap ordering (`xyz` vs `yxz` vs `zxy`).
 * SPIR-V - Standard Portable Intermediate Representation 5 - Intermediate language for GPUs. Currently on version "SPIR-V 1.6".
 * API Proccessor Pipelines - Dedicated systems to handle the tasks a gpu can perform: Vertex, Tessellation Control, Tessellation Evaluation, Geometry, Fragment, and Compute

## Notes
 * Recursion is forbidden.

## Shader Processors
GLSL contains several "API Processor Pipelines" to handle several different tasks to be performed on a GPU: Vertex, Tessellation Control, Tessellation Evaluation, Geometry, Fragment, and Compute.

Scripts written for each processor are called "shaders" and compiled into "shader executables. For example, the "Vertex Processor" is a programmable unit that operates on incoming vertices and their associated data. Scripts written to run on the vertex processor are called "Vertex Shaders". When the vertex shader is compiled and linked, they result in a "Vertex Shader Executable".

Processors
 * Vertex - Operates on vertices and their associated data. Operates on a single vertex at a time.
 * Tessellation Control - Operates on a patch of incoming vertices and their associated data, emitting a new output patch. Defines the amount of subdivisions to be made by the "Tessellation Primitive Generator". Patches for tesselation shadres are like work groups for compute shaders.
 * Tessellation Evaluation - Operates on data emitted by the "Tessellation Primitive Generator", which is controlled by the "Tesselation Control" processor, and determines where to actually place the vertices defined in the control processor.
 * Geometry - Operates on a "primitive" with a fixed number of vertices and emits a variable number of vertices that are assembled into primitives of a declared output primitive type.
 * Fragment - Operates on fragment values and their associated data. Fragment shaders can NOT change a gragment's (x, y) position. Access to neighboring fragments is NOT allowed. The values computed are ultimately used to update framebuffer memory or texture memory. Often called "pixel shader".
 * Compute - Operates independently from the other shaders. Operates on textures, buffers, image variables, and atomic counters. Outputs/side-effects are through changes to images, storage buffers, and atomic counters.


### Vertex Processor


### Tessellation Control Processor
Notes:
 * Run once for every vertex in a patch.
 * Data in a patch is shared by all vertices in the patch.
 * Sits between vertex and fragment shaders, ex: vertex -> tessellation control -> tessellation evaluation -> fragment

Default Input Variables:
 * `int gl_PatchVerticesIn` - Number of vertices per patch.
 * `int gl_primitiveID` - Index of the current patch.
 * `int gl_InvocationID` - Index of the current vertex within this patch.

Other Variables:
 * `out float gl_TessLevelInner[2];` - Inner tessellation levels for the current patch.
 * `out float gl_TessLevelOuter[4];` - Outer tessellation levels for the current patch. Different patch types may use less than the 4 allowed members of the array (ex a triangle patch only has 3 sides and only uses 3 members of the array). It can be easy to cause gaps by modifying these value between patches on curved surfaces, be careful.

### Tessellation Evaluation Processor
Layout arguments:
 * (Example: `layout (triangles, equal_spacing, ccw) in`)
 * Primitive Mode:
   * `quad`
   * `triangle`
   * `isoline`
 * Vertex Spacing:
   * `equal_spacing`
   * `fractional_even_spacing`
   * `fractional_odd_spacing`
 * Order:
   * `ccw` - Counter Clockwise
   * `cw` - Clockwise

Default Input Variables:
 * `vec3 gl_TessCoord` - Coordinates of the current  vertex within the patch.
 * `int gl_PatchVerticesIn` - Number of vertices in the current patch.
 * `int gl_PrimitiveID` - Index of the current patch.

Other Variables:
 * `in float gl_TessLevelInner[2];` - Inner tessellation levels for the current patch
 * `in float gl_TessLevelOuter[2];` - Inner tessellation levels for the current patch

### Pixel/Fragment Processor
The values computed are ultimately used to update framebuffer memory or texture memory.
Pixel (aka Fragment) shader.

### Compute Processor
Operates on a group of "work items" called a "workgroup".

## Variables
### Variable Types
 * Uniform - The same value is passed to every function execution in a batch (ex: every pixel shader function gets the same value for `iTime` for a frame.).
 * Varying - Different values can be passed to every function execution in a batch(ex: pixel shader functions can get different values `FragColor`.).

### Input Variables
 * iTime -
 * FragCoord -


### Output Variables


## Common Functions
 * `mix`
 * `smoothstep` - ease in with ease out between 2 values ()
 * `normalize`
 * `inversesqrt`
 * `clamp`
 * `length`
 * `distance`
 * `dot`
 * `cross`
 * `reflect`
 * `refract`
 * `min`
 * `max`

## Precision
You can change the precision of floating point calculations (at the cost of speed) by using the `precision` keyword.
```glsl
precision highp float;
precision mediump float;
precision lowp float;
```

## Resources
 * Latest GLSL spec (2023-08-13) - https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.pdf
   * GLSL specs directory - https://registry.khronos.org/OpenGL/specs/gl/
 * OpenGl wiki - https://www.khronos.org/opengl/wiki/Data_Type_(GLSL)
 * Victor Gordan OpenGl Tutorials Playlist - https://www.youtube.com/playlist?list=PLPaoO-vpZnumdcb4tZc4x5Q-v7CkrQ6M-
 * The Art of Code - https://www.youtube.com/watch?v=u5HAYVHsasc
   * Youtube channel with intro to shadertoy tutorial videos and complex shader tutorial videos
 * Shader Exercise Website - https://shader-tutorial.dev/
   * A handful of exercises gradually increasing in complexity to learn glsl (a little bit like rustlings/ziglings).
 * The Book of Shaders - https://thebookofshaders.com
 * 3D Game Shaders For Beginners - https://lettier.github.io/3d-game-shaders-for-beginners/index.html
 * Graphing tool - https://www.geogebra.org/calculator
