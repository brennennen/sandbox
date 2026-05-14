#version 450

layout(location = 0) in vec3 v_direction;
layout(location = 0) out vec4 out_color;

void main() {
    vec3 dir = normalize(v_direction);
    float t = 0.5 * (dir.z + 1.0);
    vec3 horizon_color = vec3(0.7, 0.75, 0.8);
    vec3 zenith_color  = vec3(0.1, 0.3, 0.6);
    vec3 final_color = mix(horizon_color, zenith_color, t);
    out_color = vec4(final_color, 1.0);
}
