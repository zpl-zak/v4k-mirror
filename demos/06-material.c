// material demo
// - rlyeh, public domain
//
// @todo: object_print(obj, "");

#include "v4k.h"


const char *skyboxes[][2] = { // reflection, env, metadata
    {"hdr/Tokyo_BigSight_1k.hdr","hdr/Tokyo_BigSight_Env.hdr"},
    {"hdr/GCanyon_C_YumaPoint_1k.hdr","hdr/GCanyon_C_YumaPoint_Env.hdr"},
    {"hdr/Factory_Catwalk_1k.hdr","hdr/Factory_Catwalk_Env.hdr"},
    {"hdr/MonValley_G_DirtRoad_1k.hdr","hdr/MonValley_G_DirtRoad_Env.hdr"},
    {"hdr/Shiodome_Stairs_1k.hdr","hdr/Shiodome_Stairs_Env.hdr"},
};

int main() {
    // create the window
    window_create( 0.75f, WINDOW_MSAA8 );
    window_color( GRAY );

    // create camera
    camera_t cam = camera();

    // fx: load all post fx files in all subdirs.
    fx_load("fx**.fs");

    // load video, RGB texture, no audio
    video_t *v = video( "pexels-pachon-in-motion-17486489.mp4", VIDEO_RGB | VIDEO_NO_AUDIO | VIDEO_LOOP ); video_seek(v, 30);
    // load texture
    texture_t t1 = texture("kgirl/g01_texture.png", TEXTURE_RGB);
    texture_t t2 = texture("matcaps/material3", 0);
    // load model
    model_t m1 = model("suzanne.obj", MODEL_NO_ANIMATIONS);
    model_t m2 = model("suzanne.obj", MODEL_NO_ANIMATIONS|MODEL_MATCAPS);
    // model_t m3 = model("damagedhelmet.gltf", MODEL_NO_ANIMATIONS|MODEL_PBR);
    model_t m3 = model("Scutum_low.fbx", MODEL_NO_ANIMATIONS|MODEL_PBR);
    // model_t m3 = model("Cerberus_LP.FBX", MODEL_NO_ANIMATIONS|MODEL_PBR);

    // spawn object1 (diffuse)
    object_t* obj1 = scene_spawn();
    object_model(obj1, m1);
    object_diffuse(obj1, t1);
    object_scale(obj1, vec3(3,3,3));
    object_move(obj1, vec3(-10+5*0,0,-10));
    object_pivot(obj1, vec3(0,90,0));

    // spawn object2 (matcap)
    object_t* obj2 = scene_spawn();
    object_model(obj2, m2);
    object_diffuse(obj2, t2);
    object_scale(obj2, vec3(3,3,3));
    object_move(obj2, vec3(-10+5*2,0,-10));
    object_pivot(obj2, vec3(0,90,0));

    // spawn object3 (video)
    object_t* obj3 = scene_spawn();
    object_model(obj3, m1);
    object_diffuse(obj3, video_textures(v)[0]);
    object_scale(obj3, vec3(3,3,3));
    object_move(obj3, vec3(-10+5*1,0,-10));
    object_pivot(obj3, vec3(0,90,0));

    // spawn object4 (pbr)
    object_t* obj4 = scene_spawn();
    object_model(obj4, m3);
    object_scale(obj4, vec3(3,3,3));
    object_move(obj4, vec3(-10+6*3,0,-10));
    object_pivot(obj4, vec3(0,0,90));

    // create point light
    light_t* l = scene_spawn_light();
    light_type(l, LIGHT_POINT);

    // load skybox
    scene_get_active()->skybox = skybox_pbr("hdr/Tokyo_BigSight_1k.hdr","hdr/Tokyo_BigSight_Env.hdr");


    while(window_swap() && !input(KEY_ESC)) {
        // draw environment
        ddraw_grid(0);

        // update video
        video_decode( v );

        // update light position
        light_teleport(l, cam.position);

        // draw scene
        fx_begin();
        scene_render(SCENE_FOREGROUND|SCENE_BACKGROUND|SCENE_UPDATE_SH_COEF);
        fx_end();

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouselook = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, wasdec);
        camera_fps(&cam, mouselook.x,mouselook.y);
        window_cursor( !active );

        if (ui_panel("FXs", 0)) {
            ui_fxs();
            ui_panel_end();
        }

        if( ui_panel( "Viewer", 0 ) ) {
            for( int i = 0; i < countof(skyboxes); i++ ) {
                const char *filename = skyboxes[i][0];
                // bool selected = !strcmp(g_skybox.reflection->filename, file_name(filename));
                bool selected = false;
                if( ui_bool( filename, &selected ) ) {
                    scene_get_active()->skybox = skybox_pbr(skyboxes[i][0], skyboxes[i][1]);
                }
            }
            ui_panel_end();
        }
    }
}
