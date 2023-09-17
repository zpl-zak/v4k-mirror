#include "v4k.h"

#define WS 32

enum {
    WALL=2, DOOR=4
};

typedef struct {
    int16_t id:8;
    int16_t flags:8;
} cell_t;

#define cell_t(...) C_CAST(cell_t, __VA_ARGS__)

typedef struct {
    cell_t grid[WS*WS];
    mesh_t level;
} world_t;

static world_t world;

void world_init() {
    int i, j;
    for (i = 0; i < WS; ++i) {
        for (j = 0; j < WS; ++j) {
            if (i == 0 || i == WS-1 || j == 0 || j == WS-1) {
                // Border cells
                world.grid[i*WS + j] = cell_t(0, WALL);
            } else {
                // Interior cells
                world.grid[i*WS + j] = cell_t(rand()%3, 0);
            }
        }
    }

    world.level = mesh();


    const int cube_vertices = 24;
    const int cube_triangles = 12;
    const int vertex_count = WS * WS * cube_vertices;
    const int index_count = WS * WS * cube_triangles * 3;
   
    struct vert {
        vec3 pos;
        vec3 normal;
    };

    struct vert verts[vertex_count];

    unsigned index_data[index_count];

    int vertex_index = 0;
    int index_index = 0;

    static vec3 normals[6] = {
        {1, 0, 0}, {-1, 0, 0},
        {0, 1, 0}, {0, -1, 0},
        {0, 0, 1}, {0, 0, -1}
    };

    for(int z = 0; z < WS; ++z) {
        for(int x = 0; x < WS; ++x) {
            if(world.grid[z*WS + x].id >= 0) {
                for(int face = 0; face < 6; ++face) {
                    for(int i = 0; i < 4; ++i) {
                        
                        vertex_index++;
                    }
                }
                
                // Create 12 triangles for the cube
                static unsigned indices[12][3] = {
                    {0, 1, 2}, {2, 1, 3}, {4, 5, 6}, {6, 5, 7},
                    {0, 4, 1}, {1, 4, 5}, {2, 6, 3}, {3, 6, 7},
                    {0, 2, 4}, {4, 2, 6}, {1, 3, 5}, {5, 3, 7}
                };
                for(int i = 0; i < 12; ++i) {
                    for(int j = 0; j < 3; ++j) {
                        index_data[index_index++] = vertex_index - 24 + indices[i][j];
                    }
                }
            }
        }
    }

    mesh_update(&world.level, "p3 n3", sizeof(struct vert), vertex_count, verts, index_count, index_data, 0);
}

void draw_world() {
    static mat44 M;  do_once id44(M);
    static mat44 VP; multiply44x2(VP, camera_get_active()->proj, camera_get_active()->view);

    static const char *vs =
    "#version 130\n"
    "//" FILELINE "\n"
    "uniform mat4 M,VP;\n"
    "in vec3 att_position;\n"
    "in vec3 att_normal;\n"
    "out vec3 v_normal;\n"
    "void main() {\n"
    "   v_normal = normalize(att_position);\n"
    "   gl_Position = M * VP * vec4( att_position, 1.0 );\n"
    "}\n";
    static const char *fs =
    "#version 130\n"
    "//" FILELINE "\n"
    "in vec3 v_normal;\n"
    "out vec4 fragcolor;\n"
    "void main() {\n"
        "fragcolor = vec4(v_normal, 1.0);\n" // diffuse
    "}";

    static unsigned program; do_once program = shader(vs, fs, "att_position,att_normal", "fragcolor");
    shader_bind(program);
    shader_mat44("VP", VP);
    shader_mat44("M", M);

    mesh_render(&world.level);
}


int main() {
    window_create(80, WINDOW_MSAA8);
    window_title(__FILE__);

    // init world
    world_init();

    // load all fx files
    fx_load("fx**.fs");

    // load skybox
    skybox_t sky = skybox(flag("--mie") ? 0 : "cubemaps/stardust", 0); // --mie for rayleigh/mie scattering

    // camera
    camera_t cam = camera();
    cam.speed = 0.2f;

    // audio (both clips & streams)
    // audio_t SFX1 = audio_clip( "coin.wav" );
    // audio_t SFX2 = audio_clip( "pew.sfxr" );
    // audio_t BGM1 = audio_stream( "waterworld-map.fur"); // wrath_of_the_djinn.xm" );
    // audio_t BGM2 = audio_stream( "larry.mid" );
    // audio_t BGM3 = audio_stream( "monkey1.mid" ), BGM = BGM1;
    // audio_play(SFX1, 0);
    // audio_play(BGM1, 0);

    while (window_swap()) {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) window_reload();
        if( input_down(KEY_W) && input_held(KEY_LCTRL) ) break;
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );
        if( input_down(KEY_X) ) window_screenshot(__FILE__ ".png");
        if( input_down(KEY_Z) ) window_record(__FILE__ ".mp4");

        // fps camera
        bool active = ui_active() || ui_hover() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdecq.x,wasdecq.y,wasdecq.z);
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

        fx_begin();
            skybox_render(&sky, cam.proj, cam.view);

            draw_world();
        fx_end(0);
    }

    return 0;
}
