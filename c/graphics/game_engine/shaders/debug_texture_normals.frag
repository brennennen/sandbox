#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform sampler2D normalMapSampler;

void main() {
    outColor = texture(normalMapSampler, fragUV);
}
