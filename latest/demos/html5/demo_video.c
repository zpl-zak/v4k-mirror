#include "v4k.h"

void render(void *arg) {
    video_t *v = arg;

    // decode video frame and get associated textures (audio is automatically sent to audiomixer)
    texture_t *textures;
    profile( "Video decoder" ) {
        textures = video_decode( v );
    }

    // present decoded textures as a fullscreen composed quad
    profile( "Video quad" ) {
        fullscreen_quad_rgb( textures[0] );
    }

    // ui video
    if( ui_panel("Video", 0) ) {
        if( ui_button("Rewind") )  video_seek(v, video_position(v) - 3);
        if( ui_button("Pause") )   video_pause(v, video_is_paused(v) ^ 1);
        if( ui_button("Forward") ) video_seek(v, video_position(v) + 3);
        ui_panel_end();
    }
    // audio
    if( ui_panel("Audio", 0)) {
        static float master = 1;
        if( ui_slider2("Master", &master, va("%.2f", master))) audio_volume_master(master);
        ui_panel_end();
    }
}

int main() {
    // 75% window, msaa x2
    window_create(0.75f, 0);
    window_aspect_lock(910, 540);
    video_t *v = video( "videos/pexels-pachon-in-motion-17486489.mp4", VIDEO_RGB );
    window_loop(render, v);
}
