#ifndef LIGHTMAP_GLSL
#define LIGHTMAP_GLSL

bool do_lightmap() {
#ifdef LIGHTMAP_BAKING
    vec3 n = normalize(v_normal_ws);
    vec4 diffuse;

    if(u_textured) {
        diffuse = texture(u_texture2d, v_texcoord);
    } else {
        diffuse = u_diffuse; // * v_color;
    }

    if (u_texlit) {
        vec4 litsample = texture(u_lightmap, v_texcoord);
        diffuse *= litsample;
    }

    fragcolor = vec4(diffuse.rgb*u_litboost, 1.0);
    return true;
#else
    return false;
#endif
}

#endif