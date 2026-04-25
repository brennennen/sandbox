#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inTangent;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec4 outTangent;

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
} push;

void main() {
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPos, 1.0);
    fragColor = inColor;
    fragUV = inUV;

    fragNormal = normalize(mat3(push.model) * inNormal);

    vec3 worldTangent = normalize(mat3(push.model) * inTangent.xyz);
    outTangent = vec4(worldTangent, inTangent.w);
}