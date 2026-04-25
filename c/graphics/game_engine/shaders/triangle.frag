#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragTangent;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalMapSampler;

void main() {
    vec3 finalNormal;

    if (fragTangent.w == 0.0) {
        finalNormal = normalize(fragNormal);
    }
    else {
        vec3 normalTex = texture(normalMapSampler, fragUV).rgb;
        normalTex = normalTex * 2.0 - 1.0;
        vec3 N = normalize(fragNormal);
        vec3 T = normalize(fragTangent.xyz);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T) * fragTangent.w * -1.0;

        mat3 TBN = mat3(T, B, N);
        finalNormal = normalize(TBN * normalTex);
    }

    vec3 ambient = vec3(0.25);
    vec3 keyDir = normalize(vec3(1.0, 1.5, 1.0));
    vec3 keyColor = vec3(1.0, 0.95, 0.9);
    float keyDiff = max(dot(finalNormal, keyDir), 0.0);
    vec3 fillDir = normalize(vec3(-1.0, 0.5, -1.0));
    vec3 fillColor = vec3(0.8, 0.85, 1.0);
    float fillDiff = max(dot(finalNormal, fillDir), 0.0) * 0.4;
    vec3 totalLight = ambient + (keyColor * keyDiff) + (fillColor * fillDiff);
    vec4 texColor = texture(texSampler, fragUV) * fragColor;
    outColor = vec4(texColor.rgb * totalLight, texColor.a);
}
