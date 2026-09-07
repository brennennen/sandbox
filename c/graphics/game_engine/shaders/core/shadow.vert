#version 450

layout(location = 0) in vec3 in_pos;
// You can leave the other vertex inputs (normal, uv, etc.) in your C vertex struct,
// Vulkan is smart enough to ignore them if they aren't declared here.

layout(set = 0, binding = 0) uniform global_ubo {
    mat4 view;
    mat4 proj;
    mat4 light_space_matrix;
    vec4 camera_pos;
    vec4 sun_direction;
    vec4 sun_color;
} ubo;

layout(push_constant) uniform push_constants {
    mat4 transform;
    // uint is_alpha_masked;
    // uint debug_mode;
    // float metallic_factor;
    // float roughness_factor;
} push;

void main() {
    // Project the vertex into the Sun's Orthographic view
    gl_Position = ubo.light_space_matrix * push.transform * vec4(in_pos, 1.0);
    //gl_Position = ubo.proj * ubo.view * push.transform * vec4(in_pos, 1.0);
}
