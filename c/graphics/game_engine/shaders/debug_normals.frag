#version 450
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragTangent;

layout(location = 0) out vec4 outColor;
layout(set = 1, binding = 1) uniform sampler2D normalMapSampler;

void main() {
    vec3 normalTex = texture(normalMapSampler, fragUV).rgb;
    normalTex = normalTex * 2.0 - 1.0;
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent.xyz);
    if (abs(dot(T, N)) > 0.9999) {
        T = normalize(cross(N, vec3(0.0, 1.0, 0.0)));
    } else {
        T = normalize(T - dot(T, N) * N);
    }
    vec3 B = cross(N, T) * fragTangent.w;
    mat3 TBN = mat3(T, B, N);
    vec3 finalNormal = normalize(TBN * normalTex);
    vec3 debugColor = (finalNormal + 1.0) * 0.5;
    outColor = vec4(debugColor, 1.0);
}
