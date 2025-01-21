shader_type spatial;
//render_mode specular_disabled, unshaded;

uniform float shell_offset_strength = 0.1;

uniform vec3 shell_top_color : source_color = vec3(0.3, 0.1, 0.1);
uniform vec3 shell_bottom_color : source_color = vec3(0.7, 0.5, 0.5);

uniform vec3 shell_base_direction = vec3(0.0, 0.0, 0.0);
uniform vec3 shell_wind_direction = vec3(0.0, 0.0, 0.0);
uniform float shell_strand_constant = 3.0;
uniform float shell_strand_strength = 1.0;
uniform float shell_strand_add_strength = 1.0;
uniform float shell_strand_minus_strength = 0.0;
uniform float shell_strand_divide_constant = 1.0;

uniform bool shell_movement = true;
uniform float shell_movement_speed = 3.0;

uniform float uv_scale = 1.0;
uniform float worley_columns = 5.0;
uniform float worley_rows = 5.0;

vec2 random(vec2 uv) {
	return vec2(fract(sin(dot(uv.xy,
		vec2(12.9898,78.233))) * 43758.5453123));
}

/*
worley noise function with math inspired by https://thebookofshaders.com/12/
*/
float worley(vec2 uv, float columns, float rows, float normalized_shell_id) {
	vec2 index_uv = floor(vec2(uv.x * columns, uv.y * rows));
	vec2 fract_uv = fract(vec2(uv.x * columns, uv.y * rows));

	float minimum_dist = 1.0;

	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			vec2 neighbor = vec2(float(x), float(y));
			vec2 point = random(index_uv + neighbor);
			if (shell_movement) {
				point = vec2( cos(TIME * point.x * shell_movement_speed), sin(TIME * point.y * shell_movement_speed) ) * 0.5 + 0.5;
			}
			vec2 diff = (neighbor) + (point * 1.0) - fract_uv;
			float dist = length(diff) * shell_strand_constant;
			dist = dist * (normalized_shell_id * shell_strand_strength);
			dist = dist + (normalized_shell_id * shell_strand_add_strength);
			dist = dist - (normalized_shell_id * shell_strand_minus_strength);
			dist = dist / shell_strand_divide_constant;
			minimum_dist = min(minimum_dist, dist);
		}
	}
	// DEBUG: draw grid
    //minimum_dist += step(0.98, fract_uv.x) + step(0.98, fract_uv.y);
	// DEBUG: draw isolines
	//minimum_dist -= step(0.7, abs(sin(27.0 * minimum_dist))) * 0.5;
	return minimum_dist;
}

void vertex() {
	// TODO: read vertex color to control length a bit. 3d asset authors can maybe us a channel to paint where fur should exist or not?
	float shell_offset = (INSTANCE_CUSTOM.a * shell_offset_strength);

	vec3 normal = NORMAL;
	// TODO: do 3d noise to create swirls as the "shell_base_direction" to create swirly tufts.
	normal = normal + shell_wind_direction;
	normal = normalize(normal);
	vec3 shell_position = (normal * shell_offset) + vec3(VERTEX.x, VERTEX.y, VERTEX.z);
	vec3 shell_color = mix(shell_top_color, shell_bottom_color, INSTANCE_CUSTOM.a);
	COLOR = vec4(shell_color, INSTANCE_CUSTOM.a);
    VERTEX = shell_position;
}

void fragment() {
	ALBEDO = COLOR.rgb;
	float worley = worley(UV * uv_scale, worley_columns, worley_rows, COLOR.a);
	worley = 1.0 - worley;
	ALPHA = worley;
}
