#version 450

layout(set = 0, binding = 0) uniform sampler2D screenTexture;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

// this isn't math, this is wizardry
vec3 tonemap_aces_film(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 apply_vignette(vec3 color, vec2 uv) {
    // TODO
    return color;
}

vec3 apply_grayscale(vec3 color, vec2 uv) {
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    return vec3(gray, gray, gray);
}

void main() {
    vec2 corrected_uv = vec2(inUV.x, 1.0 - inUV.y);
    vec3 scene_color = texture(screenTexture, corrected_uv).rgb;
    scene_color = tonemap_aces_film(scene_color);
    outColor = vec4(scene_color, 1.0);
}
