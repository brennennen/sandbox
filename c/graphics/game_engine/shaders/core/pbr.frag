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
    mat4 light_space_matrix;
    vec4 camera_pos;
    vec4 sun_direction;
    vec4 sun_color;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D normalMapSampler;
layout(set = 1, binding = 2) uniform sampler2D ao_roughness_metallic_sampler; // AO = red, Roughness = green, Metallic = blue
layout(set = 0, binding = 2) uniform samplerCube irradiance_map;
layout(set = 0, binding = 3) uniform samplerCube prefiltered_map;
layout(set = 0, binding = 4) uniform sampler2D shadow_map;
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

float calculate_shadow(vec3 fragPosWorldSpace) {
    vec4 fragPosLightSpace = ubo.light_space_matrix * vec4(fragPosWorldSpace, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Convert NDC to UVs [0, 1]
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    projCoords.y = 1.0 - projCoords.y;

    // If outside the spotlight's cone, it is in shadow
    if(projCoords.z > 1.0 || projCoords.z < 0.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }

    float closestDepth = texture(shadow_map, projCoords.xy).r;

    // Basic static bias
    float shadow = (projCoords.z - 0.005) > closestDepth ? 1.0 : 0.0;
    return shadow;
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

    // Direct sun lighting
    vec3 L = normalize(-ubo.sun_direction.xyz);
    float NdotL = max(dot(N, L), 0.0);

    // Basic Lambertian diffuse for the sun
    vec3 direct_diffuse = (kD * texColor.rgb / PI) * ubo.sun_color.xyz * NdotL;

    // Add a simple Specular highlight for the sun so metallic objects gleam
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    float spec_power = pow(NdotH, max(1.0 - roughness, 0.001) * 128.0);
    vec3 direct_specular = F0 * spec_power * ubo.sun_color.xyz * NdotL;

    vec3 direct_light = (direct_diffuse + direct_specular) * 5.0; // Multiplied by 5.0 to boost sun intensity

    float shadow = calculate_shadow(fragPos);

    float ambient_intensity = 0.2;
    vec3 ambient = (kD * diffuse + specular) * ao * ambient_intensity;
    vec3 final_color = ambient + (direct_light * (1.0 - shadow));
    outColor = vec4(final_color, texColor.a);

    vec2 screen_uv = gl_FragCoord.xy / vec2(1920.0, 1080.0);
    float raw_shadow_depth = texture(shadow_map, screen_uv).r;
    float visual_shadow_depth = (1.0 - raw_shadow_depth) * 100.0;

    //outColor = vec4(vec3(visual_shadow_depth), 1.0);
    //outColor = vec4(vec3(1.0 - shadow), 1.0);


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
