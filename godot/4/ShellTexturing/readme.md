# Shell Texturing Prototype
Small toy program to understand the concept of shell texturing better. This godot project involves spawning a `MultiMeshInstance3D` to generate "shells" around a mesh. Then a shader takes those shells and modifies the vertice positions outwards along the normals to create a russian nesting doll of the original mesh. Then the shader hides (sets fragment/pixel shader alpha to 0) sections of these outer shells to create the illusion of form. I used a worley noise here, which gives a very "organic" look. When I had more time, I'd like to explore modifying the noise in a more "hair strand" or "fur" looking way (one of the large use cases for this effect is animal fur).

Video demo:
https://drive.google.com/file/d/1PjstZJx85XunH_6rhDk6sK1brdcqMNFs/view?usp=drive_link

## Further Explorations
* Explore adding second layer of noise to create "tufts" of hair that cycle into each other? maybe
do this at the vertex shader stage by manipulating normals?
* Explore reading asset vertex color (maybe red?) for where to paint fur and where not to?
* Explore triplanar mapping to hide the seam that's in the default mesh instance 3d sphere

## References
* GFur technique article - https://gim.studio/animalia/an-introduction-to-shell-based-fur-technique/
* Acerola video - https://www.youtube.com/watch?v=9dr-tRQzij4&
	* Acerola unity code - https://github.com/GarrettGunnell/Shell-Texturing/blob/main/Assets/SimpleShell.cs
* Godot 3 shell texturing addon - https://github.com/Arnklit/ShellFurGodot
	* Partial port to 4 - https://github.com/radair6000/ShellFurGodot4/tree/godot_4_1
* Book of shaders for noise generation techniques: https://thebookofshaders.com/12/
