#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec4 fragTangent;
layout(location = 4) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    vec4 camera_pos;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalMapSampler;
layout(set = 1, binding = 2) uniform sampler2D ao_roughness_metallic_sampler; // AO = red, Roughness = green, Metallic = blue

layout(push_constant) uniform PushConstants {
    mat4 transform;
    uint is_masked;
    uint debug_mode;
    float metallic_factor;
    float roughness_factor;
} pc;

const float PI = 3.14159265359;

void main() {
    vec3 finalNormal;
    vec4 texColor = texture(texSampler, fragUV) * fragColor;

    if (pc.is_masked == 1 && texColor.a < 0.5) {
        discard;
    }

    vec4 ao_roughness_metallic_sample = texture(ao_roughness_metallic_sampler, fragUV);
    float ao = ao_roughness_metallic_sample.r;
    float roughness = max(ao_roughness_metallic_sample.g * pc.roughness_factor, 0.04);
    float metallic = ao_roughness_metallic_sample.b * pc.metallic_factor;

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
    vec3 keyDir = normalize(vec3(1.0, -1.5, 1.0));
    vec3 keyColor = vec3(1.0, 0.95, 0.9);
    float keyDiff = max(dot(finalNormal, keyDir), 0.0);

    vec3 fillDir = normalize(vec3(-1.0, 0.5, -1.0));
    vec3 fillColor = vec3(0.8, 0.85, 1.0);
    float fillDiff = max(dot(finalNormal, fillDir), 0.0) * 0.4;

    vec3 camera_pos = ubo.camera_pos.xyz;
    vec3 view_dir = normalize(camera_pos - fragPos); // direction from the pixel to the camera lens
    vec3 halfwayDir = normalize(keyDir + view_dir); // halfway vector between light and camera

    float shininess = exp2(10.0 * (1.0 - roughness) + 1.0); // map roughness to shininess
    float specular = pow(max(dot(finalNormal, halfwayDir), 0.0), shininess); // how much light bounces off normal surface into camera
    float energy_conservation = (shininess + 8.0) / (8.0 * PI); // smaller highlights become brighter

    vec3 raw_diffuse_light = ambient + (keyColor * keyDiff) + (fillColor * fillDiff);
    vec3 final_diffuse = texColor.rgb * raw_diffuse_light * (1.0 - metallic);
    vec3 specular_tint = mix(vec3(0.04), texColor.rgb, metallic);
    vec3 final_specular = keyColor * specular * specular_tint * energy_conservation;
    vec3 ambient_specular = ambient * specular_tint; // fake environment reflection so metals aren't black

    vec3 ambientLight = vec3(0.03) * texColor.rgb * ao;

    outColor = vec4(final_diffuse + final_specular + ambient_specular + ambientLight, texColor.a);

    // debug traps
    if (isnan(fragPos.x) || isnan(fragPos.y) || isnan(fragPos.z)) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0); // red - world pos invalid
    }
    else if (isnan(camera_pos.x) || isnan(camera_pos.y) || isnan(camera_pos.z)) {
        outColor = vec4(0.0, 1.0, 0.0, 1.0); // green - camera pos invalid
    }
    else if (length(camera_pos - fragPos) < 0.00001) {
        outColor = vec4(0.0, 0.0, 1.0, 1.0); // blue - under geo
    }
    else if (isnan(view_dir.x) || isnan(view_dir.y) || isnan(view_dir.z)) {
        outColor = vec4(1.0, 1.0, 0.0, 1.0); // yellow - misc
    }
}
