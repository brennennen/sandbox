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
layout(location = 4) out vec3 fragPos;

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 transform;
    uint is_masked;
    uint debug_mode;
    float metallic_factor;
    float roughness_factor;
} pc;

void main() {
    vec4 worldPos = pc.transform * vec4(inPos, 1.0);
    gl_Position = ubo.proj * ubo.view * worldPos;

    fragColor = inColor;
    fragUV = inUV;

    fragNormal = mat3(pc.transform) * inNormal;
    outTangent = vec4(mat3(pc.transform) * inTangent.xyz, inTangent.w);

    fragPos = worldPos.xyz;
}
