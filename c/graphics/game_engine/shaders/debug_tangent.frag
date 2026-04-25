#version 450
layout(location = 3) in vec4 fragTangent;

layout(location = 0) out vec4 outColor;

void main() {
    // Map vector [-1, 1] to color [0, 1]
    vec3 debugColor = (normalize(fragTangent.xyz) + 1.0) * 0.5;
    outColor = vec4(debugColor, 1.0);
}
