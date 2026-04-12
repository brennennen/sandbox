#version 450

// Input
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;

// Output
layout(location = 0) out vec4 outColor;

void main() {
    outColor = fragColor;
}