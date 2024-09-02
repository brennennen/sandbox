// Only distance based, has aliasing issues.
float create_circle_mask(vec2 uv, vec2 position, float radius) {
    float pixel_distance = length(uv - position);
    float pixel_mask_color = 0.0;
    if (pixel_distance < radius) {
        pixel_mask_color = 1.0;
    }
    return pixel_mask_color;
}

// Using "smoothstep" to ease in and ease out between pixels.
float create_circle_with_easing_mask(vec2 uv, vec2 position, float radius, float easing) {
    float pixel_distance = length(uv - position);
    float pixel_mask_color = smoothstep(radius, radius - easing, pixel_distance);
    return pixel_mask_color;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    uv.x = uv.x * iResolution.x/iResolution.y; // Match UV space to the aspect ratio

    // Circle with hard edges/harsh aliasing
    float circl1_mask = create_circle_mask(uv, vec2(0.25, 0.6666), 0.1);
    fragColor = vec4(vec3(circl1_mask), 1.0);

    // Circle with soft edges/easing
    float circle2_mask = create_circle_with_easing_mask(uv, vec2(0.5, 0.6666), 0.1, 0.015);
    fragColor += vec4(vec3(circle2_mask), 1.0);

    // Donut! Subtraction
    float circle3_outer_mask = create_circle_with_easing_mask(uv, vec2(0.75, 0.6666), 0.1, 0.015);
    float circle3_inner_mask = create_circle_with_easing_mask(uv, vec2(0.75, 0.6666), 0.05, 0.01);
    float circle3_mask = circle3_outer_mask - circle3_inner_mask; // remove the inner circle from the outer circle
    fragColor += vec4(vec3(circle3_mask), 1.0);

    // Pink Circle
    float circle4_mask = create_circle_with_easing_mask(uv, vec2(0.25, 0.3333), 0.1, 0.01);
    vec3 circle4_color = circle4_mask * vec3(1.0, 0.0, 1.0);
    fragColor += vec4(circle4_color, 1.0);

    // Overlap - Addition
    float circle5_left_mask = create_circle_with_easing_mask(uv, vec2(0.5, 0.3333), 0.1, 0.01);
    vec3 circle5_left_color = circle5_left_mask * vec3(1.0, 0.0, 0.0);
    float circle5_middle_mask = create_circle_with_easing_mask(uv, vec2(0.566, 0.4333), 0.1, 0.01);
    vec3 circle5_middle_color = circle5_middle_mask * vec3(0.0, 1.0, 0.0);
    float circle5_right_mask = create_circle_with_easing_mask(uv, vec2(0.625, 0.3333), 0.1, 0.01);
    vec3 circle5_right_color = circle5_right_mask * vec3(0.0, 0.0, 1.0);
    vec3 circle5_color = circle5_left_color + circle5_middle_color + circle5_right_color;
    fragColor += vec4(circle5_color, 1.0);
}
