#include "v4k.h"


int SKY_DIR = 0;
const char *SKY_DIRS[] = {
    "cubemaps/bridge3/",
    "cubemaps/colors/",
    "cubemaps/colors2/",
    "hdr/Tokyo_BigSight_1k.hdr",
};

int main(int argc, char** argv) {
    window_create(85, 0);
    window_fps_unlock();

    camera_t cam = camera(); {
        cam.position = vec3(0, 500, 0);
        cam.pitch = -90;
        cam.yaw = 0;
        camera_fps(&cam, 0, 0);
    }
    skybox_t sky = {0};
    model_t mdl = {0};

    array(light_t) lights = 0;

    for (int y = -5; y < 5; y++) {
        for (int x = -5; x < 4; x++) {
            light_t lit = light();
            light_type(&lit, LIGHT_POINT);
            light_falloff(&lit, 0.1, 0.01, 0.001);
            light_diffuse(&lit, vec3(randf(), randf(), randf()));
            light_pos(&lit, vec3(x * 25, 1, y * 25));
            array_push(lights, lit);
        }
    }

    bool initialized = 0;
    bool must_reload = 0;

    while( window_swap()) {
        if (input_down(KEY_ESC)) break;
        // reloading
        if( must_reload ) {
            must_reload = 0;
            skybox_destroy(&sky);
            initialized = 0;
        }
        if( !initialized ) {
            initialized = 1;
            sky = skybox(flag("--mie") ? 0 : SKY_DIRS[SKY_DIR], 0);
            mdl = model("plane.obj", 0);
            scale44(mdl.pivot, 150, 1, 150);
            model_bind_shader(mdl);
            cubemap_sh_shader(&sky.cubemap);
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
            skybox_render(&sky, cam.proj, cam.view);

            model_light(&mdl, array_count(lights), lights);
            model_render(mdl, cam.proj, cam.view, mdl.pivot);
        }

        if( ui_panel("Scene", 0)) {
            if( ui_list("Skybox", SKY_DIRS, countof(SKY_DIRS), &SKY_DIR) ) {
                must_reload = 1;
            }
            ui_separator();
            ui_lights(array_count(lights), lights);
            ui_panel_end();
        }
    }
}
