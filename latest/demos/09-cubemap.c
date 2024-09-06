#include "v4k.h"

int SKY_DIR = 0;
const char *SKY_DIRS[] = {
    "cubemaps/bridge3/",
    "cubemaps/colors/",
    "cubemaps/colors2/",
    "hdr/Tokyo_BigSight_1k.hdr",
};

int OBJ_MDL = 0;
const char *OBJ_MDLS[] = {
    "meshes/sphere.obj",
    "meshes/suzanne.obj",
    "meshes/gazebo.obj",
};

int main(int argc, char** argv) {
    window_create(85, WINDOW_MSAA8);

    camera_t cam = camera();
    skybox_t sky = {0};
    model_t mdl = {0};

    bool initialized = 0;
    bool must_reload = 0;

    while( window_swap()) {
        if (input_down(KEY_ESC)) break;
        // reloading
        if( must_reload ) {
            must_reload = 0;
            skybox_destroy(&sky);
            model_destroy(mdl);
            initialized = 0;
        }
        if( !initialized ) {
            initialized = 1;
            sky = skybox(flag("--mie") ? 0 : SKY_DIRS[SKY_DIR], 0);
            mdl = model(OBJ_MDLS[OBJ_MDL], 0);
            // sky.rayleigh_immediate = 1;
            rotation44(mdl.pivot, 0, 1,0,0); // @fixme: -90,1,0,0 -> should we rotate SHMs as well? compensate rotation in shader?
        }

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, scale3(wasdec, window_delta() * 60));
        camera_fps(&cam, mouse.x,mouse.y);

        // render
        mat44 mvp; multiply44x2(mvp, cam.proj, cam.view);
        {
            do_once {
                if (flag("--mie")) {
                    // skybox_sh_reset(&sky);
                    skybox_mie_calc_sh(&sky, 4.0f);
                    // float x = cosf((float)window_time())*4;
                    // skybox_sh_add_light(&sky, vec3(0.3,0.3,0.3), vec3(0,1,0), 16*absf(cosf((float)window_time()*2))+2);
                    // skybox_sh_add_light(&sky, vec3(0.6,0,0), vec3(x,1,0), 2);
                }
            }

            skybox_render(&sky, cam.proj, cam.view);

            model_bind_shader(mdl);
            shader_vec3v("u_coefficients_sh", 9, sky.cubemap.sh);
            shader_int("u_textured", false);

            model_render(mdl, cam.proj, cam.view, mdl.pivot);

        }

        if( ui_panel("Scene", 0)) {
            if( ui_list("Skybox", SKY_DIRS, countof(SKY_DIRS), &SKY_DIR) ) {
                must_reload = 1;
            }
            if( ui_list("Model", OBJ_MDLS, countof(OBJ_MDLS), &OBJ_MDL) ) {
                must_reload = 1;
            }
            ui_separator();
            for (int i = 0; i < 9; i++) {
                ui_color3f(va("SH Coefficient [%d]", i), &sky.cubemap.sh[i].x);
            }
            ui_panel_end();
        }
    }
}
