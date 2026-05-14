/**
 * Specular-only debug fragment shader.
 */
#version 450

// MARK: Varyings

// Inputs
layout(location = 0) in vec4 frag_vertex_color;
layout(location = 1) in vec2 frag_texture_coordinates;
layout(location = 2) in vec3 frag_vertex_normal;
layout(location = 3) in vec4 frag_vertex_tangent;
layout(location = 4) in vec3 frag_world_position;
// Outputs
layout(location = 0) out vec4 out_color;

// MARK: Uniforms
layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
} camera;
layout(set = 1, binding = 0) uniform sampler2D albedo_sampler;
layout(set = 1, binding = 1) uniform sampler2D normal_map_sampler;
layout(set = 1, binding = 2) uniform sampler2D ao_roughness_metallic_sampler; // AO = red, Roughness = green, Metallic = blue

// MARK: Push Constants
layout(push_constant) uniform PushConstants {
    mat4 transform;
    uint is_alpha_masked;
    uint debug_mode;
} pc;

// MARK: Functions

/**
 * Calculates the final world-space surface normal by applying a tangent-space normal map.
 *
 * @param base_normal          The interpolated 3D vertex normal in world space.
 * @param base_tangent         The interpolated 3D vertex tangent (xyz) and mirroring sign (w).
 * @param texture_coordinates  The 2D UV coordinates used to sample the normal map.
 *
 * @return vec3 The final, perturbed, normalized surface normal in world space.
 */
vec3 calculate_world_space_normal(vec3 base_normal, vec4 base_tangent, vec2 texture_coordinates) {
    vec3 surface_normal = normalize(base_normal);
    if (length(base_tangent.xyz) < 0.001) {
        return surface_normal; // If the mesh does not have tangent data, fall back to the base normal
    }
    vec3 tangent_vector = normalize(base_tangent.xyz);

    // Ensure the Tangent is perfectly 90-degrees to the normal (Gram-Schmidt)
    vec3 orthogonal_tangent = tangent_vector - dot(tangent_vector, surface_normal) * surface_normal;
    if (length(orthogonal_tangent) < 0.001) {
        return surface_normal; // If the vectors were perfectly parallel, fall back to the base normal
    }
    tangent_vector = normalize(orthogonal_tangent);

    vec3 bitangent_vector = cross(surface_normal, tangent_vector) * base_tangent.w * -1.0; // Reconstruct the Bitangent using the cross product. Multiply by the 'w' component (-1.0 or 1.0) to handle mirrored textures.
    mat3 tangent_basis_matrix = mat3(tangent_vector, bitangent_vector, surface_normal); // Construct the Tangent-Bitangent-Normal matrix to transform from flat texture space to 3D world space
    vec3 sampled_normal_color = texture(normal_map_sampler, texture_coordinates).rgb; // Sample the normal map image
    vec3 local_surface_normal = sampled_normal_color * 2.0 - 1.0; // Convert from the image color range [0.0 to 1.0] to physical direction vectors [-1.0 to 1.0]
    return normalize(tangent_basis_matrix * local_surface_normal); // Rotate the flat map normal into the 3D world
}

// MARK: Main
void main() {
    vec3 resolved_world_normal = calculate_world_space_normal(
        frag_vertex_normal,
        frag_vertex_tangent,
        frag_texture_coordinates
    );

    switch (pc.debug_mode) {
        case 0: { // DRAW_MODE_FORWARD_LIT
            out_color = vec4(1.0, 0.0, 1.0, 1.0);
            break;
        }
        case 1: { // DRAW_MODE_DEBUG_WIREFRAME
            out_color = vec4(1.0, 0.0, 1.0, 1.0);
            break;
        }
        case 2: { // ALBEDO
            out_color = texture(albedo_sampler, frag_texture_coordinates) * frag_vertex_color;
            break;
        }
        case 3: { // DRAW_MODE_DEBUG_LIGHTING
            vec3 light_dir = normalize(vec3(1.0, 1.0, 1.0));
            float diff = max(dot(frag_vertex_normal, light_dir), 0.15);
            vec3 clay_color = vec3(0.7, 0.7, 0.7);
            out_color = vec4(clay_color * diff, 1.0);
            break;
        }
        case 4: { // DRAW_MODE_DEBUG_GEOMETRY_NORMAL
            vec3 color = frag_vertex_normal * 0.5 + 0.5;
            out_color = vec4(color, 1.0);
            break;
        }
        case 5: { // DRAW_MODE_DEBUG_TEXTURE_NORMAL
            out_color = texture(normal_map_sampler, frag_texture_coordinates);
            break;
        }
        case 6: { // DRAW_MODE_DEBUG_NORMAL
            vec3 normal_tex = texture(normal_map_sampler, frag_texture_coordinates).rgb;
            normal_tex = normal_tex * 2.0 - 1.0;
            vec3 N = normalize(frag_vertex_normal);
            vec3 T = normalize(frag_vertex_tangent.xyz);
            if (abs(dot(T, N)) > 0.9999) {
                T = normalize(cross(N, vec3(0.0, 1.0, 0.0)));
            } else {
                T = normalize(T - dot(T, N) * N);
            }
            vec3 B = cross(N, T) * frag_vertex_tangent.w;
            mat3 TBN = mat3(T, B, N);
            vec3 final_normal = normalize(TBN * normal_tex);
            vec3 normal_debug_color = (final_normal + 1.0) * 0.5;
            out_color = vec4(normal_debug_color, 1.0);
            break;
        }
        case 7: { // DRAW_MODE_DEBUG_TANGENT
            vec3 tangent_debug_color = (normalize(frag_vertex_tangent.xyz) + 1.0) * 0.5;
            out_color = vec4(tangent_debug_color, 1.0);
            break;
        }
        case 8: { // DRAW_MODE_DEBUG_BITANGENT
            vec3 N = normalize(frag_vertex_normal);
            vec3 T = normalize(frag_vertex_tangent.xyz);
            vec3 B = cross(N, T) * frag_vertex_tangent.w;
            vec3 bitangent_debug_color = (normalize(B) + 1.0) * 0.5;
            out_color = vec4(bitangent_debug_color, 1.0);
            break;
        }
        case 9: { // DRAW_MODE_DEBUG_VERTEX_COLOR
            out_color = frag_vertex_color;
            break;
        }
        case 10: { // DRAW_MODE_DEBUG_MIPMAPS
            vec2 lod_info = textureQueryLod(albedo_sampler, frag_texture_coordinates);
            float mip_level = lod_info.x;
            vec3 debug_color;
            if      (mip_level < 1.0) debug_color = vec3(1.0, 0.0, 0.0); // Red (Mip 0 - Closest)
            else if (mip_level < 2.0) debug_color = vec3(0.0, 1.0, 0.0); // Green (Mip 1)
            else if (mip_level < 3.0) debug_color = vec3(0.0, 0.0, 1.0); // Blue (Mip 2)
            else if (mip_level < 4.0) debug_color = vec3(1.0, 1.0, 0.0); // Yellow (Mip 3)
            else if (mip_level < 5.0) debug_color = vec3(1.0, 0.0, 1.0); // Magenta (Mip 4)
            else                     debug_color = vec3(0.0, 1.0, 1.0); // Cyan (Mip 5+)
            vec3 light_dir = normalize(vec3(0.5, 1.0, 0.3));
            float n_dot_l = max(dot(normalize(frag_vertex_normal), light_dir), 0.2);
            out_color = vec4(debug_color * n_dot_l, 1.0);
            break;
        }
        case 11: { // DRAW_MODE_DEBUG_SPECULAR
            vec3 resolved_world_normal = calculate_world_space_normal(
                frag_vertex_normal,
                frag_vertex_tangent,
                frag_texture_coordinates
            );

            // Affine Transformations & Matrix Inverses
            vec3 key_light_direction = normalize(vec3(1.0, 1.5, 1.0)); // hard coded key light
            vec3 camera_pos = vec3(inverse(camera.view)[3]);
            vec3 view_dir = camera_pos - frag_world_position;

            vec3 view = length(view_dir) > 0.001 ? normalize(view_dir) : vec3(0.0, 0.0, 1.0);

            // Blinn-Phong speculars/highlights
            vec3 halfway_dir = normalize(key_light_direction + view);
            float spec = pow(max(dot(resolved_world_normal, halfway_dir), 0.0), 32.0);

            out_color = vec4(vec3(spec), 1.0);
            break;
        }
        case 12: { // DRAW_MODE_DEBUG_AO
            float ao = texture(ao_roughness_metallic_sampler, frag_texture_coordinates).r;
            out_color = vec4(vec3(ao), 1.0);
            break;
        }
        case 13: { // DRAW_MODE_DEBUG_ROUGHNESS
            float roughness = texture(ao_roughness_metallic_sampler, frag_texture_coordinates).g;
            out_color = vec4(vec3(roughness), 1.0);
            break;
        }
        case 14: {// DRAW_MODE_DEBUG_METALLIC
            float metallic = texture(ao_roughness_metallic_sampler, frag_texture_coordinates).b;
            out_color = vec4(vec3(metallic), 1.0);
            break;
        }
        default: {
            out_color = vec4(1.0, 0.0, 1.0, 1.0); // bad draw mode value! turn everything pink!
            break;
        }
    }
}
