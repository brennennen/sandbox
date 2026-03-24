#version 450


layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(norm, lightDir), 0.0);
    float ambient = 0.15;
    float light_intensity = ambient + diff;
    vec4 baseColor = fragColor * texture(texSampler, fragUV);
    outColor = vec4(baseColor.rgb * light_intensity, baseColor.a);
}
