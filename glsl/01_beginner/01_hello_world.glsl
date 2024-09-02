// Simple test to set each pixel to the exact same color
void _01_set_frag_color(out vec4 fragColor) {
    fragColor += vec4(1.0, 0.0, 1.0, 1.0); // frankly, filthy color
}

// Map the screen space x and y to the red and green color channels to show
// how `fragCoord` is provided a unique value per pixel.
void _02_using_frag_coord(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = fragCoord/iResolution.xy;
    fragColor += vec4(uv.x, uv.y, 0.0, 1.0);
}

// Using the "length" function with fragment coordinates, you can create a map
// defining the distance between pixels from an arbitrary point.
// (0, 0) is the bottom left corner.
void _03_frag_coord_distance(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = fragCoord/iResolution.xy;
    vec2 position_offset = vec2(0.5, 0.5);
    float distance_from_center = length(uv - position_offset);
    fragColor += vec4(distance_from_center, distance_from_center, distance_from_center, 1.0);
}

// Pulse through various rgb values, the magic numbers are to keep the rgb channels
// out of sync so they don't just repeat the same patterns.
void _04_using_time(out vec4 fragColor) {
    float r = abs(sin(iTime * 0.21));
    float g = abs(sin(iTime * 0.53));
    float b = abs(sin(iTime * 0.71));
    fragColor += vec4(r, g, b, 1.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    //_01_set_frag_color(fragColor);
    //_02_using_frag_coord(fragColor, fragCoord);
    //_03_frag_coord_distance(fragColor, fragCoord);
    _04_using_time(fragColor);
}
