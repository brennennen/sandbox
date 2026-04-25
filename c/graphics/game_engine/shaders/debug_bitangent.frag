#version 450
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragTangent;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent.xyz);
    vec3 B = cross(N, T) * fragTangent.w;

    vec3 debugColor = (normalize(B) + 1.0) * 0.5;
    outColor = vec4(debugColor, 1.0);
}
