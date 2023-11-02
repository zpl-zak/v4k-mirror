#include "v4k.h"

float* billboard(mat44 modelview, bool billboard_x, bool billboard_y, bool billboard_z) {
    if( billboard_x ) { modelview[0*4+0] = 1; modelview[0*4+1] = 0; modelview[0*4+2] = 0; }
    if( billboard_y ) { modelview[1*4+0] = 0; modelview[1*4+1] = 1; modelview[1*4+2] = 0; }
    if( billboard_z ) { modelview[2*4+0] = 0; modelview[2*4+1] = 0; modelview[2*4+2] = 1; }
    return modelview;
}
const char *billboard_name(bool billboard_x, bool billboard_y, bool billboard_z) {
    // Spherical billboarding makes the object to always face the camera no matter the position of the camera on an imaginary sphere.
    // Cylindrical billboarding makes the object to face the camera only when the camera looks at the right or at the left.
    return
    billboard_x && billboard_y && billboard_z ? "(spherical billboard)" : billboard_x && billboard_z ? "(cylindrical billboard)" : "";
}

const char *shader_projected_vs =
"#version 150\n"
"in vec3 att_position;\n"
"in vec2 att_uv;\n"
"out vec2 uv;\n"
"uniform mat4 u_projection;\n"
"uniform mat4 u_modelview;\n"
//"uniform mat4 u_billboard;\n"
"uniform float u_aspect;\n"

"void main() {\n"
"    gl_Position = u_projection * u_modelview * vec4(att_position, 1.0f);\n"
"    uv = vec2(att_uv.x, 1.0 - att_uv.y);\n"
"return;"
"    // billboarding keeping scale (see: https://stackoverflow.com/questions/15937842/glsl-billboard-shader-keep-the-scaling)\n"
"    vec2 scale = vec2( length(u_modelview[0]) / u_aspect, length(u_modelview[1]) );\n"
"    vec4 billboard = u_modelview * vec4(vec3(0.0), 1.0);\n"
"    gl_Position = u_projection * billboard + vec4(scale * att_position.xy, 0.0, 0.0);\n"
""
"}\n";

const char *shader_textured_fs =
"#version 150\n"
"in vec2 uv;\n"
"out vec4 out_color;\n"
"uniform sampler2D u_texture;\n"
"void main() {\n"
"   out_color = texture(u_texture, uv);\n"
"}\n";

int main() {
    // create the window
    window_create( 0.5f, 0 );
    window_color( RGB3(40,40,50) );

    // enable some gl stuff. needed?
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // compile shaders
    GLuint program = // shader(shader_projected_vs, shader_textured_fs);
    shader(shader_projected_vs, shader_textured_fs, "att_position,att_uv", "out_color", NULL );

    // a test cube: 3POS + 2UV
    GLfloat vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    typedef struct MyVertex {
        vec3 pos;
        vec2 texcoord;
    } MyVertex;

    GLuint indices[] = {
        0,2,1,3,5,4,6,7,8,9,10,11,12,13,14,15,16,17,18,20,19,21,23,22,24,25,26,27,28,29,30,32,31,33,35,34
    };

    mesh_t cube = mesh();
    mesh_update(&cube, "p3 t2", 0,36,vertices, 36,indices, 0);

    mat44 cube_transform; id44(cube_transform);

    // create (fps) camera
    camera_t cam = camera();

    // main loop
    while (window_swap()) {
        if (input(KEY_ESC))
            break;

        // setup projection
        mat44 proj;
        perspective44(proj, 45, window_aspect(), 0.1f, 100.0f);

        // fps camera
        bool active = ui_hover() || ui_active() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );
        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, wasdecq);
        camera_fps(&cam, mouse.x,mouse.y);

        // spin cube
        // rotate44(cube->transform, 1, 1,1,1);

        static int billboard_x = 0; if( input_down(KEY_1) ) billboard_x ^= 1;
        static int billboard_y = 0; if( input_down(KEY_2) ) billboard_y ^= 1;
        static int billboard_z = 0; if( input_down(KEY_3) ) billboard_z ^= 1;

        // draw cube
        mat44 modelview; multiply44x2(modelview, cam.view, cube_transform);
        shader_bind(program);
        shader_mat44("u_projection", proj);
        shader_mat44("u_modelview", billboard(modelview, billboard_x,billboard_y,billboard_z) );
        shader_texture("u_texture", texture_checker());

        mesh_render(&cube);

        window_title(va("billboard_x: %s, billboard_y: %s, billboard_z: %s %s", billboard_x ? "on":"off", billboard_y ? "on":"off", billboard_z ? "on":"off", billboard_name(billboard_x,billboard_y,billboard_z)));
    }
}
