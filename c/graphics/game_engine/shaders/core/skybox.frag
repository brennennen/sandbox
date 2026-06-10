#version 450

layout(location = 0) in vec3 v_direction;
layout(location = 0) out vec4 out_color;
layout(set = 0, binding = 1) uniform samplerCube env_map;

void main() {
    vec3 color = texture(env_map, normalize(v_direction)).rgb;
    vec3 test_color = color * 0.05;
    out_color = vec4(test_color, 1.0);
}
