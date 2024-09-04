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
    "meshes/CornellBox-Original.obj",
    "meshes/sphere.obj",
    "meshes/suzanne.obj",
    "meshes/gazebo.obj",
};

int main(int argc, char** argv) {
    window_create(85, WINDOW_MSAA8);
    window_fps_unlock();

    camera_t cam = camera(); {
        cam.position = vec3(0, 7.5, 15);
        cam.pitch = -15;
        cam.yaw = -90;
        camera_fps(&cam, 0, 0);
    }
    skybox_t sky = {0};
    skybox_t env_probe = {0};
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
            env_probe = skybox(0, 0);
            mdl = model(OBJ_MDLS[OBJ_MDL], 0);
            shader_bind(mdl.program);
            vec3 clr[9]={0};
            shader_vec3v("u_coefficients_sh", 9, clr);
            // rotation44(mdl.pivot, 0, 1,0,0); // @fixme: -90,1,0,0 -> should we rotate SHMs as well? compensate rotation in shader?
        }

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, wasdec);
        camera_fps(&cam, mouse.x,mouse.y);

        static bool first_time = true;
        static bool animate_probe_pos = true;
        static bool follow_cam = false;
        static bool show_env_probe = false;
        static vec3 probe_pos;
        if (input_down(KEY_T)) {
            animate_probe_pos = !animate_probe_pos;
        }
        if (input_down(KEY_F)) {
            follow_cam = !follow_cam;
        }
        if (input_down(KEY_P)) {
            show_env_probe = !show_env_probe;
        }
        if (animate_probe_pos) {
            probe_pos = vec3(0, 5.5, 1);
            probe_pos.x = sinf(window_time()*2)*3.9f;
            probe_pos.y += cosf(window_time()*2)*1.0;
        }
        if (input_down(KEY_SPACE) || first_time || animate_probe_pos || follow_cam) {
            first_time = false;
            mat44 probe_proj, probe_view;
            if (!animate_probe_pos) {
                probe_pos = cam.position;
            }
            if (follow_cam) {
                probe_pos = cam.position;
            }
            unsigned tex_size = 64;
            cubemap_bake_begin(&env_probe.cubemap, probe_pos, tex_size, tex_size);
            while (cubemap_bake_step(&env_probe.cubemap, probe_proj, probe_view)) {
                skybox_render(&sky, probe_proj, probe_view);

                shader_bind(mdl.program);
                cubemap_sh_shader(0);
                model_render(mdl, probe_proj, probe_view, mdl.pivot, 0);
            }
            cubemap_bake_end(&env_probe.cubemap, 4, 1.0f);
        }

        ddraw_sphere(probe_pos, 0.1f);

        // render
        mat44 mvp; multiply44x2(mvp, cam.proj, cam.view);
        {
            if (show_env_probe) {
                skybox_render(&env_probe, cam.proj, cam.view);
            } else {
                skybox_render(&sky, cam.proj, cam.view);
                shader_bind(mdl.program);
                cubemap_sh_blend(vec3(0,0,0), 10.0f, 1, &env_probe.cubemap);
                shader_int("u_textured", false);

                model_render(mdl, cam.proj, cam.view, mdl.pivot, 0);
            }
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
                ui_color3f(va("SH Coefficient [%d]", i), &env_probe.cubemap.sh[i].x);
            }
            ui_panel_end();
        }
    }
}
