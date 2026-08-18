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
layout(set = 0, binding = 2) uniform samplerCube irradiance_map;
layout(set = 0, binding = 3) uniform samplerCube prefiltered_map;
// layout(set = 0, binding = 3) uniform sampler2D brdfLUT;

layout(push_constant) uniform PushConstants {
    mat4 transform;
    uint is_masked;
    uint debug_mode;
    float metallic_factor;
    float roughness_factor;
} pc;

const float roughness_strength = 1.0;

const float PI = 3.14159265359;

// ratio of reflected light vs refracted light
vec3 fresnel_schlick_roughness(float cos_theta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 finalNormal;
    vec4 texColor = texture(texSampler, fragUV) * fragColor;

    if (pc.is_masked == 1 && texColor.a < 0.5) {
        discard;
    }

    vec4 ao_roughness_metallic_sample = texture(ao_roughness_metallic_sampler, fragUV);
    float ao = ao_roughness_metallic_sample.r;
    //float ao = 1.0;
    float roughness = max(ao_roughness_metallic_sample.g * pc.roughness_factor, 0.04) * roughness_strength;
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

    vec3 camera_pos = ubo.camera_pos.xyz;
    vec3 V = normalize(camera_pos - fragPos); // View vector
    vec3 N = finalNormal;                     // Normal vector
    vec3 R = reflect(-V, N);                  // Reflection vector

    // base reflectivity
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, texColor.rgb, metallic);
    float NdotV = max(dot(N, V), 0.0001); // Prevent division by zero later

    // irradiance
    vec3 irradiance = texture(irradiance_map, N).rgb;
    vec3 F = fresnel_schlick_roughness(NdotV, F0, roughness);

    // refraction
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic; // Metals absorb all refracted light
    vec3 diffuse = irradiance * texColor.rgb;

    // specular ibl
    const float MAX_REFLECTION_LOD = 4.0; // needs to match cooked mipmap levels
    vec3 prefilteredColor = textureLod(prefiltered_map, R, roughness * MAX_REFLECTION_LOD).rgb;
    // mock BRDF LUT
    vec2 envBRDF = vec2(
        pow(1.0 - max(roughness, 0.04), 4.0), // Scale
        0.04                                  // Bias
    );
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    vec3 ambient = (kD * diffuse + specular) * ao;
    outColor = vec4(ambient, texColor.a);

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
    // else if (isnan(view_dir.x) || isnan(view_dir.y) || isnan(view_dir.z)) {
    //     outColor = vec4(1.0, 1.0, 0.0, 1.0); // yellow - misc
    // }
}
