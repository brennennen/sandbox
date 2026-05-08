#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main() {
    vec2 lodInfo = textureQueryLod(texSampler, fragUV);
    float mipLevel = lodInfo.x;
    vec3 debugColor;
    if      (mipLevel < 1.0) debugColor = vec3(1.0, 0.0, 0.0); // Red (Mip 0 - Closest)
    else if (mipLevel < 2.0) debugColor = vec3(0.0, 1.0, 0.0); // Green (Mip 1)
    else if (mipLevel < 3.0) debugColor = vec3(0.0, 0.0, 1.0); // Blue (Mip 2)
    else if (mipLevel < 4.0) debugColor = vec3(1.0, 1.0, 0.0); // Yellow (Mip 3)
    else if (mipLevel < 5.0) debugColor = vec3(1.0, 0.0, 1.0); // Magenta (Mip 4)
    else                     debugColor = vec3(0.0, 1.0, 1.0); // Cyan (Mip 5+)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float n_dot_l = max(dot(normalize(fragNormal), lightDir), 0.2);
    outColor = vec4(debugColor * n_dot_l, 1.0);
}