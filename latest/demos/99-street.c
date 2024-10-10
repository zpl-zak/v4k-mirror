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
    window_create(80, 0);
    window_title(__FILE__);
    window_fps_unlock();

    // load all fx files
    fx_load("fx**.fs");
    fx_enable(fx_find("fx/fxFXAA3.fs"), 1);

    // load skybox
    // skybox_t sky = skybox(flag("--mie") ? 0 : "hdr/Tokyo_BigSight_1k.hdr", 0); // --mie for rayleigh/mie scattering
    skybox_t sky = skybox_pbr(skyboxes[0][0], skyboxes[0][1], skyboxes[0][2]);

    // load static scene
    model_t street, street_shadow;
    street = model(option("--model","Highway_Interchange.obj"), 0); // MODEL_NO_TEXTURES);
    // street_shadow = model(option("--model","Highway_Interchange_shadow.obj"), 0); // MODEL_NO_TEXTURES);
    // translation44(street.pivot, 0,-1,0);
    // rotate44(street.pivot, -90,1,0,0);
    scale44(street.pivot, 0.1,0.1,0.1);

    // camera
    camera_t cam = camera();

    object_t *obj = scene_spawn();
    object_model(obj, street);
    // object_model_shadow(obj, street_shadow);
    // object_scale(obj, vec3(0.1,0.1,0.1));

    double sky_update_until = 0.0;

    light_t *sun = scene_spawn_light();

    scene_skybox(sky);

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

        scene_render(SCENE_BACKGROUND|SCENE_FOREGROUND|SCENE_SHADOWS|SCENE_POSTFX);

        if( ui_panel( "Viewer", 0 ) ) {
            for( int i = 0; i < countof(skyboxes); i++ ) {
                const char *filename = skyboxes[i][0];
                // bool selected = !strcmp(g_skybox.reflection->filename, file_name(filename));
                bool selected = false; 
                if( ui_bool( filename, &selected ) ) {
                    scene_skybox(skybox_pbr(skyboxes[i][0], skyboxes[i][1], skyboxes[i][2]));
                }
            }
            ui_light(sun);
            ui_panel_end();
        }
    }
}
