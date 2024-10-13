uniform sampler2D u_normal_texture;
uniform sampler2D u_matprops_texture;
uniform samplerCube u_cubemap_texture;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_inv_projection;

uniform float u_max_distance; /// set:100.0
uniform float u_reflection_strength; /// set:0.5
uniform float u_metallic_threshold; /// set:0.001
uniform bool u_sample_skybox; /// set:0
const float EPSILON = 0.0001;

bool ray_out_of_bounds(vec3 screen_pos) {
    return screen_pos.x < 0.0 || screen_pos.x > 1.0 || screen_pos.y < 0.0 || screen_pos.y > 1.0;
}

vec3 trace_ray(vec3 ray_pos, vec3 ray_dir, int steps, out vec3 last_pos) {
    float sample_depth;
    vec3 hit_color = vec3(0.0);

    for (int i = 0; i < steps; ++i) {
        last_pos = ray_pos;
        ray_pos += ray_dir;
        if (ray_out_of_bounds(ray_pos)) {
            break;
        }

        sample_depth = texture(iChannel1, ray_pos.xy).r;
        float depth_delta = ray_pos.z - sample_depth;
        if (depth_delta >= 0.0 && depth_delta < EPSILON) {
            hit_color = texture(iChannel0, ray_pos.xy).rgb;
            break;
        }
    }
    return hit_color;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 uv = vec2(fragCoord.x / iChannelRes0x, fragCoord.y / iChannelRes0y);
    vec4 color_pixel = texture(iChannel0, uv);

    vec3 matprops = texture(u_matprops_texture, uv).rgb;
    if (matprops.r * color_pixel.a < u_metallic_threshold) {
        fragColor = vec4(0);
        return;
    }

    if (matprops.r == 0.0) {
        matprops.r = 1.0;
    }

    vec3 pos;
    pos.xy = uv;

    vec3 normal_view = (u_view * vec4(texture(u_normal_texture, pos.xy).xyz, 0.0)).xyz;
    float pixel_depth = texture(iChannel1, pos.xy).r;
    pos.z = pixel_depth;
    vec4 pos_view = u_inv_projection * vec4(pos*2 - vec3(1), 1.0);
    pos_view.xyz /= pos_view.w;

    vec3 refl_view = normalize(reflect(pos_view.xyz, normal_view));
    if (refl_view.z > 0.0) {
        if (u_sample_skybox) {
            vec3 world_refl = (inverse(u_view) * vec4(refl_view, 0.0)).xyz;
            vec3 refl_color = texture(u_cubemap_texture, world_refl).rgb;
            vec3 result = mix(vec3(0.0), refl_color, u_reflection_strength*matprops.r);
            bool is_black = max(result.r, max(result.g, result.b)) < 0.01;
            fragColor = vec4(result, is_black ? 0.0 : 1.0);
        } else {
            fragColor = vec4(0);
        }
        return;
    }
    vec3 ray_end_view = pos_view.xyz + refl_view * u_max_distance;
    vec4 ray_end_clip = u_projection * vec4(ray_end_view, 1.0);
    ray_end_clip.xyz /= ray_end_clip.w;
    vec3 ray_end_pos = ray_end_clip.xyz * 0.5 + 0.5;
    vec3 ray_dir = ray_end_pos - pos;

    ivec2 ss_start = ivec2(pos.x * iChannelRes0x, pos.y * iChannelRes0y);
    ivec2 ss_end = ivec2(ray_end_pos.x * iChannelRes0x, ray_end_pos.y * iChannelRes0y);
    ivec2 ss_distance = ss_end - ss_start;
    int ss_max_distance = max(abs(ss_distance.x), abs(ss_distance.y)) / 2;
    ray_dir /= max(ss_max_distance, 0.001);

    vec3 last_pos;
    vec3 refl_color = trace_ray(pos, ray_dir, ss_max_distance, last_pos);

    if (refl_color == vec3(0.0)) {
        if (u_sample_skybox) {
            vec3 world_refl = (inverse(u_view) * vec4(refl_view, 0.0)).xyz;
            refl_color = texture(u_cubemap_texture, world_refl).rgb;
            vec3 result = mix(vec3(0.0), refl_color, u_reflection_strength*matprops.r);
            bool is_black = max(result.r, max(result.g, result.b)) < 0.01;
            fragColor = vec4(result, is_black ? 0.0 : 1.0);
        } else {
            fragColor = vec4(0);
        }
        return;
    }

    vec3 result = mix(vec3(0.0), refl_color, u_reflection_strength*matprops.r);
    bool is_black = max(result.r, max(result.g, result.b)) < 0.1;
    fragColor = vec4(result, is_black ? 0.0 : 1.0);
}