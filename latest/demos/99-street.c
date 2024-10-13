// render street

#include "v4k.h"

const char *skyboxes[][3] = { // reflection, rad, env
    {"hdr/GCanyon_C_YumaPoint_1k.hdr","hdr/GCanyon_C_YumaPoint_1k.hdr","hdr/GCanyon_C_YumaPoint_Env.hdr"},
    {"hdr/Factory_Catwalk_1k.hdr","hdr/Factory_Catwalk_Rad.hdr","hdr/Factory_Catwalk_Env.hdr"},
    {"hdr/graffiti_shelter_4k.hdr","hdr/graffiti_shelter_Rad.hdr","hdr/graffiti_shelter_Env.hdr"},
    {"hdr/Tokyo_BigSight_1k.hdr","hdr/Tokyo_BigSight_1k.hdr","hdr/Tokyo_BigSight_Env.hdr"},
    {"hdr/music_hall_01_4k.hdr","hdr/music_hall_01_4k.hdr","hdr/music_hall_01_Env.hdr"},
    {"hdr/the_sky_is_on_fire_2k.hdr","hdr/the_sky_is_on_fire_2k.hdr","hdr/the_sky_is_on_fire_Env.hdr"},
    {"hdr/MonValley_G_DirtRoad_1k.hdr","hdr/MonValley_G_DirtRoad_1k.hdr","hdr/MonValley_G_DirtRoad_Env.hdr"},
    {"hdr/Shiodome_Stairs_1k.hdr","hdr/Shiodome_Stairs_1k.hdr","hdr/Shiodome_Stairs_Env.hdr"},
    {"hdr/mesto.hdr","hdr/mesto.hdr","hdr/mesto_Env.hdr"},
};

int main() {
    window_create(80, WINDOW_VSYNC);
    window_title(__FILE__);
    // window_fps_unlock();

    // load all fx files
    fx_load("fx**.fs");
    fx_enable_ordered(fx_find("fx/fxTonemapACES.fs"));
    fx_enable_ordered(fx_find("fx/fxFXAA3.fs"));

    // load skybox
    skybox_t sky = skybox_pbr(skyboxes[0][0], skyboxes[0][1], skyboxes[0][2]);

    // load static scene
    model_t street, street_shadow;
    street = model(option("--model","Highway_Interchange.obj"), 0);

    // camera
    camera_t cam = camera();

    object_t *obj = scene_spawn();
    object_model(obj, street);

    double sky_update_until = 0.0;

    light_t *sun = scene_spawn_light();

    scene_skybox(sky);

    
    fbo_t main_fb = fbo(window_width(), window_height(), 0, TEXTURE_FLOAT);

    scene_t *scene = scene_get_active();

    reflect_params_t reflect_params = {
        .max_distance = 100.0f,
        .reflection_strength = 0.8f,
        .metallic_threshold = 0.001f,
        .downsample = 0,
        .cubemap = &sky.cubemap,
    };

    bloom_params_t bloom_params = {
        .mips_count = 4,
        .filter_radius = 0.005f,
        .strength = 0.80f,
        .threshold = 0.10f,
        .soft_threshold = 0.50f,
        .suppress_fireflies = true
    };

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) window_reload();
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );
        if( input_down(KEY_X) ) window_screenshot(__FILE__ ".png");
        if( input_down(KEY_Z) ) window_record(__FILE__ ".mp4");

        // fps camera
        camera_freefly(&cam);
        
        fbo_resize(&main_fb, window_width(), window_height());
        fbo_bind(main_fb.id);
            viewport_clear(false, true);
            viewport_clip(vec2(0,0), vec2(window_width(), window_height()));
            scene_render(SCENE_BACKGROUND|SCENE_FOREGROUND|SCENE_SHADOWS|SCENE_DRAWMAT);
            fx_drawpass(fx_find("fx/fxSSAO.fs"), main_fb.texture_color, main_fb.texture_depth);
        fbo_unbind();

        reflect_params.cubemap = &sky.cubemap;

        {
            texture_t reflect_fb = fxt_reflect(main_fb.texture_color, main_fb.texture_depth, scene->drawmat.normals, scene->drawmat.matprops, cam.proj, cam.view, reflect_params);
            fbo_blit(main_fb.id, reflect_fb, 1);
        
            texture_t bloom_fb = fxt_bloom(main_fb.texture_color, bloom_params);
            fbo_blit(main_fb.id, bloom_fb, 1);
            // fullscreen_quad_rgb_flipped(reflect_fb);
        }

        fx_apply(main_fb.texture_color, main_fb.texture_depth);

        if( ui_panel( "Viewer", 0 ) ) {
            for( int i = 0; i < countof(skyboxes); i++ ) {
                const char *filename = skyboxes[i][0];
                bool selected = false; 
                if( ui_bool( filename, &selected ) ) {
                    scene_skybox(skybox_pbr(skyboxes[i][0], skyboxes[i][1], skyboxes[i][2]));
                }
            }
            ui_float("Blend Region", &scene_get_active()->shadowmap.blend_region);
            ui_section("reflect");
            ui_float("Max Distance", &reflect_params.max_distance);
            ui_float("Reflection Strength", &reflect_params.reflection_strength);
            ui_float("Metallic Threshold", &reflect_params.metallic_threshold);
            ui_int("Downsample", &reflect_params.downsample);
            ui_section("bloom");
            ui_int("Mips Count", &bloom_params.mips_count);
            ui_float("Filter Radius", &bloom_params.filter_radius);
            ui_float("Strength", &bloom_params.strength);
            ui_float("Threshold", &bloom_params.threshold);
            ui_float("Soft Threshold", &bloom_params.soft_threshold);
            ui_bool("Suppress Fireflies", &bloom_params.suppress_fireflies);
            ui_section("sunlight");
            ui_light(sun);
            ui_panel_end();
        }
    }
}
