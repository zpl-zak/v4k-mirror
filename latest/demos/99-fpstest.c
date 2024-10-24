#include "v4k.h"

vec3 campos1;
vec3 campos2;
tween_t anim;
bool cam_tween_reset=1;
camera_t cam;
#define NUM_SAMPLES 64

int tick_mode = 0;

static double smooth_delta() {
    static double time_samples[NUM_SAMPLES] = {0};
    static int curr_index = 0;
    double time = window_delta();

    time_samples[curr_index] = time;
    if (++curr_index == NUM_SAMPLES)
        curr_index = 0;

    double average = 0;
    for (int i = NUM_SAMPLES; i--; )
        average += time_samples[i];
    average /= NUM_SAMPLES;

    time = minf(time, average);
    return time;
}

bool intro_camera() {
    const float START_MOVING_ONTO_PLANET = 1.5f;
    const float STAY_PUT_AT_PLANET = 4.0f;
    const float ZOOM_OUT_TO_BOARD = 5.0f;
    const float STAY_STILL = 6.0f;
    const float LIFT_OFF = 7.0f;
    if (cam_tween_reset) {
        anim = tween();
        tween_setkey(&anim, 0.0f, campos1, EASE_QUAD);
        tween_setkey(&anim, START_MOVING_ONTO_PLANET, campos1, EASE_QUAD);
        tween_setkey(&anim, STAY_PUT_AT_PLANET, campos2, EASE_QUAD);
        tween_setkey(&anim, ZOOM_OUT_TO_BOARD, campos2, EASE_QUAD);
        tween_setkey(&anim, LIFT_OFF, campos1, EASE_QUAD);
        cam_tween_reset=0;
    }

    float val = !tick_mode ? window_delta() : tick_mode == 1 ? smooth_delta() : 0.016f;
    printf("dt: %.04f\n", val);
    bool done = tween_update(&anim, val) == 1.0f;
    cam.position = anim.result;
    camera_lookat(&cam, vec3(cam.position.x,0,cam.position.z));
    return done;
}

void reset_intro() {
    cam_tween_reset=1;
    tween_destroy(&anim);

    campos1 = vec3(0.0f, 5.0f, 0.0f);
    campos2 = vec3(0.0f, 25.0f, 0.0f);
}

int main() {
    window_create(75.0, WINDOW_FULLSCREEN|WINDOW_SQUARE/*|WINDOW_VSYNC_DISABLED*/);
    // window_fps_unlock();
    // window_fps_lock(60.0);

    cam = camera();
    reset_intro();

    while( window_swap() && !input(KEY_ESC) ) { // game loop
        if (intro_camera())
            reset_intro();

        ddraw_circle(vec3(0,0,0), vec3(0,-1,0), 1.5f);

        if (input_down(KEY_SPACE))
            tick_mode = ++tick_mode % 3;

        ddraw_text2d(vec2(15, 95), va("mode: %s\n", !tick_mode ? "delta time" : tick_mode == 1 ? "avg delta time" : "fixed time increment"));
    }
}
