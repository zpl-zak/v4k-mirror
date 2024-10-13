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
    fx_enable_ordered(fx_find("fx/fxSSAO.fs"));
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

    int mips_count = 6;
    float filter_radius = 0.005f;
    float strength = 0.40f;
    float threshold = 0.10f;
    float soft_threshold = 0.50f;
    bool suppress_fireflies = true;

    fbo_t main_fb = fbo(window_width(), window_height(), 0, TEXTURE_FLOAT);

    scene_t *scene = scene_get_active();

    
        reflect_params_t reflect_params = {
            .ray_step = 0.1f,
            .iteration_count = 100,
            .distance_bias = 0.05f,
            .sample_count = 0,
            .adaptive_step = true,
            .binary_search = true,
            .sampling_coefficient = 0.01f,
            .metallic_threshold = 0.0f,
            .debug = false
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
        
        fbo_bind(main_fb.id);
            viewport_clear(true, true);
            viewport_clip(vec2(0,0), vec2(window_width(), window_height()));
            scene_render(SCENE_BACKGROUND|SCENE_FOREGROUND|SCENE_SHADOWS|SCENE_DRAWMAT);
        fbo_unbind();

        bloom_params_t bloom_params = {
            .mips_count = mips_count,
            .filter_radius = filter_radius,
            .strength = strength,
            .threshold = threshold,
            .soft_threshold = soft_threshold,
            .suppress_fireflies = suppress_fireflies,
        };

        texture_t bloom_fb = fxt_bloom(main_fb.texture_color, bloom_params);
        texture_t reflect_fb = fxt_reflect(main_fb.texture_color, main_fb.texture_depth, scene->drawmat.normals, scene->drawmat.matprops, cam.proj, cam.view, reflect_params);

        // fx_apply(main_fb.texture_depth, main_fb.texture_depth);
        fullscreen_quad_rgb_flipped(reflect_fb);
        // fbo_blit(main_fb.id, bloom_fb, 1);
        // fbo_blit(main_fb.id, reflect_fb, 0);
        // fx_apply(main_fb.texture_color, main_fb.texture_depth);

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
            ui_float("Ray Step", &reflect_params.ray_step);
            ui_int("Iteration Count", &reflect_params.iteration_count);
            ui_float("Distance Bias", &reflect_params.distance_bias);
            ui_int("Sample Count", &reflect_params.sample_count);
            ui_bool("Adaptive Step", &reflect_params.adaptive_step);
            ui_bool("Binary Search", &reflect_params.binary_search);
            ui_float("Sampling Coefficient", &reflect_params.sampling_coefficient);
            ui_section("sunlight");
            ui_light(sun);
            ui_panel_end();
        }
    }
}
