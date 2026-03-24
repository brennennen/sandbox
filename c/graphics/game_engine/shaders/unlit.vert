#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;     // Ignored
layout(location = 3) in vec3 inNormal; // Ignored

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
} push;

void main() {
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPos, 1.0);
    fragColor = inColor;
}
