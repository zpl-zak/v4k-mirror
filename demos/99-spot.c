// [ref] http://fabiensanglard.net/shadowmappingVSM/index.php
// [ref] http://www.opengl-tutorial.org/es/intermediate-tutorials/tutorial-16-shadow-mapping/
// [ref] https://github.com/cforfang/opengl-shadowmapping
// [ref] https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping

// @todo: spotlight (cone light)
// @todo: pointlight (cubemap light)
// @todo: area light (rect or circle light)
// @todo: directional light (sunlight)

// further reading (in order):
// DPSM  [3] Brabec, Annen: Shadow mapping for hemispherical and omnidirectional light sources (2002)
// DPSM* [4] Osman, Bukowski: Practical implementation of dual paraboloid shadow maps (2006)
// IPSM  [5] Vanek, Herout: High-quality Shadows with Improved Paraboloid Mapping (2011)
// LiSPSMs
// CSMs

// status: CUBE(0)+BLUR(0): ok
// status: CUBE(0)+BLUR(1): ok
// status: CUBE(1)+BLUR(1): ok
// status: CUBE(1)+BLUR(0): ok
// status: CUBE(?)+BLUR(?): no {
// 003.470s|!cannot find uniform 'shadowMap' in shader program 21                         |shader_uniform|v4k_render.c:772
// 001: 00007FF7AF6A3FDA callstack (C:\prj\thread\V4K\v4k_system.c:250)
// 002: 00007FF7AF8E7CBC shader_uniform (C:\prj\thread\V4K\v4k_render.c:772)
// 003: 00007FF7AF691C27 shader_int (C:\prj\thread\V4K\v4k_render.c:777)
// 004: 00007FF7AF8F54EF color_begin (C:\prj\thread\V4K\spot.c:525)
// 005: 00007FF7AF8F5BF7 main (C:\prj\thread\V4K\spot.c:607)
// }

#ifndef VSMCUBE
#define VSMCUBE 0
#endif
#ifndef VSMBLUR
#define VSMBLUR 1
#endif

#include "v4k.h"
#include "split/v4k_shaders.c"

model_t sponza;

typedef struct Mesh
{
    GLuint vao;
    GLuint vbo;
} Mesh;

static float quadVertices[] = {
    // Front-face
    // Pos              // Color          //Tex       // Norm
    -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top-left
     1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top-right
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Bottom-right

     1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Bottom-right
    -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Bottom-left
    -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top-left
};

static Mesh create_mesh(float* verts, int size) {
    Mesh mesh;

    // Create VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    // Create VBO and copy the vertex data to it
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, size, verts, GL_STATIC_DRAW);

    // Enable attribs
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 0);
    // Color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    // Texcoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));

    return mesh;
}

Mesh create_quad() {
    return create_mesh(quadVertices, sizeof(quadVertices));
}

struct shadow {
    // Resources
    GLuint shadowProgram, blurProgram;
    Mesh quadMesh;
    GLuint shadowMapFBO, shadowMapTex, shadowMapTexDepth;
    GLuint blurFBO, blurTex;
    // Options
    bool do_blur;
    bool do_debugshadow;
    float BLUR_SCALE; // Amount of blurring [0..100]
    GLuint SHADOWMAP_WIDTH; // 256,512,1024,2048
    // State
    int shadow_active;
    vec3 lightPos;
    vec3 lightAimPos;

    // VSM cubemap
    GLuint cubeTex, cubeDepthTex, cubeFBOs[6];
    GLuint currentSideTex, currentSideDepthTex;
    GLuint toCurrentSideFBO;
} shadow;

#define shadowProgram shadow.shadowProgram
#define blurProgram shadow.blurProgram
#define quadMesh shadow.quadMesh
#define shadowMapFBO shadow.shadowMapFBO
#define shadowMapTex shadow.shadowMapTex
#define shadowMapTexDepth shadow.shadowMapTexDepth
#define blurFBO shadow.blurFBO
#define blurTex shadow.blurTex
#define do_blur shadow.do_blur
#define do_debugshadow shadow.do_debugshadow
#define BLUR_SCALE shadow.BLUR_SCALE
#define SHADOWMAP_WIDTH shadow.SHADOWMAP_WIDTH
#define shadow_active shadow.shadow_active
#define lightPos shadow.lightPos
#define lightAimPos shadow.lightAimPos
// vsm cubemap
#define cubeTex shadow.cubeTex
#define cubeDepthTex shadow.cubeDepthTex
#define cubeFBOs shadow.cubeFBOs
#define currentSideTex shadow.currentSideTex
#define currentSideDepthTex shadow.currentSideDepthTex
#define toCurrentSideFBO shadow.toCurrentSideFBO



static GLuint cubemap_create(GLsizei size, int flags) {
    GLenum texel = flags & TEXTURE_DEPTH ? GL_DEPTH_COMPONENT : GL_RGB32F;
    GLenum pixel = flags & TEXTURE_DEPTH ? GL_DEPTH_COMPONENT : GL_RGB;
    GLenum storage = flags & TEXTURE_DEPTH ? GL_FLOAT : GL_UNSIGNED_BYTE; // swap?
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, texel, size, size, 0, pixel, storage, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, texel, size, size, 0, pixel, storage, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, texel, size, size, 0, pixel, storage, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, texel, size, size, 0, pixel, storage, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, texel, size, size, 0, pixel, storage, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, texel, size, size, 0, pixel, storage, NULL);
    if( flags & TEXTURE_DEPTH ) {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    } else {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return id;
}
static void framebuffer_cube_create(GLuint cube_fbo[6], GLuint cube_texture, GLuint cube_texture_depth) {
    glGenFramebuffers(6, cube_fbo);
    for (int i = 0; i < 6; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, cube_fbo[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cube_texture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cube_texture_depth, 0);
        GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (GL_FRAMEBUFFER_COMPLETE != result) {
            printf("ERROR: Framebuffer is not complete.\n");
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
static void set_shadow_matrix_uniform(const GLuint program, int dir) {
    if(dir<0) return;
    mat44 P, V, PV;
    perspective44(P, 90.0f, 1.0f, 0.5f, 100.0f);

    /**/ if(dir == 0) lookat44(V, lightPos, add3(lightPos, vec3(+1,  0,  0)), vec3(0, -1,  0)); // +X
    else if(dir == 1) lookat44(V, lightPos, add3(lightPos, vec3(-1,  0,  0)), vec3(0, -1,  0)); // -X
    else if(dir == 2) lookat44(V, lightPos, add3(lightPos, vec3( 0, +1,  0)), vec3(0,  0, +1)); // +Y
    else if(dir == 3) lookat44(V, lightPos, add3(lightPos, vec3( 0, -1,  0)), vec3(0,  0, -1)); // -Y
    else if(dir == 4) lookat44(V, lightPos, add3(lightPos, vec3( 0,  0, +1)), vec3(0, -1,  0)); // +Z
    else /*dir == 5*/ lookat44(V, lightPos, add3(lightPos, vec3( 0,  0, -1)), vec3(0, -1,  0)); // -Z
    multiply44x2(PV, P, V); // -Z

    shader_bind(program); shader_mat44("cameraToShadowView", V);
    shader_bind(program); shader_mat44("cameraToShadowProjector", PV);
}

void shadow_create(int RESOLUTION) {
    do_blur = 1;
    do_debugshadow = 0;
    BLUR_SCALE = 0.5; // Amount of blurring [0..1]
    SHADOWMAP_WIDTH = RESOLUTION; // 256,512,1024,2048
    lightPos = vec3(-2, 2.0, -2);
    lightAimPos = vec3(0.0, 0, -5.0);

    // Create programs
    shadowProgram = shader(vs_shadow_vsm, fs_shadow_vsm, "position", "outColor", "");
    blurProgram = shader(vs_shadow_blur, fs_shadow_blur, "position,,texcoord", "outColor", "");

    // ShadowMap-textures and FBO // @todo: GL_RG32F+GL_RG also GL_CLAMP to remove artifacts
    shadowMapTex = texture_create(SHADOWMAP_WIDTH, SHADOWMAP_WIDTH, 2, NULL, TEXTURE_FLOAT).id;
    shadowMapTexDepth = texture_create(SHADOWMAP_WIDTH, SHADOWMAP_WIDTH, 0, NULL, TEXTURE_DEPTH | TEXTURE_FLOAT).id;
    shadowMapFBO = fbo(shadowMapTex, shadowMapTexDepth, 0);

    // Textures and FBO to perform blurring
    blurTex = texture_create(SHADOWMAP_WIDTH, SHADOWMAP_WIDTH, 2, NULL, TEXTURE_FLOAT).id;
    blurFBO = fbo(blurTex, 0, 0);

#if VSMCUBE
    // Create cubemap
    cubeTex      = cubemap_create(SHADOWMAP_WIDTH, 0);
    cubeDepthTex = cubemap_create(SHADOWMAP_WIDTH, TEXTURE_DEPTH);
    framebuffer_cube_create(cubeFBOs, cubeTex, cubeDepthTex);
    // Temporary storage
    currentSideTex = texture_create(SHADOWMAP_WIDTH, SHADOWMAP_WIDTH, 2, NULL, TEXTURE_FLOAT /*| TEXTURE_EDGE*/ ).id;
    currentSideDepthTex = texture_create(SHADOWMAP_WIDTH, SHADOWMAP_WIDTH, 0, NULL, TEXTURE_DEPTH | TEXTURE_FLOAT).id;
    toCurrentSideFBO = fbo(currentSideTex, currentSideDepthTex, 0);
#endif
}
void shadow_destroy() {
    glDeleteProgram(shadowProgram);
    glDeleteProgram(blurProgram);

    glDeleteTextures(1, &blurTex);
    glDeleteFramebuffers(1, &blurFBO);

    glDeleteTextures(1, &shadowMapTex);
    glDeleteTextures(1, &shadowMapTexDepth);
    glDeleteFramebuffers(1, &shadowMapFBO);
}
bool shadow_is_active() {
    return shadow_active;
}
void shadow_pv_matrix(mat44 PV) {
    mat44 P,V;
//    perspective44(P, 45.0f, 1.0f, 2.0f, 100.0f);
perspective44(P, 45*2, window_width() / ((float)window_height()+!window_height()), 1.f, 100.f);

    lookat44(V, lightPos, lightAimPos, vec3(0, 1, 0)); // Point toward object regardless of position
    multiply44x2(PV, P, V);
}
void shadow_position(vec3 p) {
    lightPos = p;
}
void shadow_target(vec3 p) {
    lightAimPos = p;
}
void shadow_begin() {
    shadow_active = 1;

    // shadow_state() {
        glEnable(GL_DEPTH_TEST);  glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);   glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        glBlendFunc(1, 0);
#if VSMCUBE
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
    // }

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

    glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_WIDTH);
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat44 PV; shadow_pv_matrix(PV);
    shader_bind(shadowProgram);shader_mat44("cameraToShadowProjector", PV);
}
static void draw_fullscreen_quad() {
    //delete_mesh(quadMesh);
    if(!quadMesh.vao) quadMesh = create_quad();

    glBindVertexArray(quadMesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
static void shadow_blur() {
    // remap 0(min)..1(max) -> 2(min)..epsilon(max)
    float blur_scale = 1.999 * (1 - BLUR_SCALE) + 0.001;

    glDisable(GL_DEPTH_TEST);

    // Blur shadowMapTex (horizontally) to blurTex
    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
    glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_WIDTH);
        glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, shadowMapTex); //Input-texture
    shader_bind(blurProgram); shader_vec2("ScaleU", vec2(1.0 / (SHADOWMAP_WIDTH*blur_scale), 0));
    shader_int("textureSource",0);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_fullscreen_quad();

    // Blur blurTex vertically and write to shadowMapTex
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_WIDTH);
        glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurTex);
    shader_bind(blurProgram); shader_vec2("ScaleU", vec2(0, 1.0 / (SHADOWMAP_WIDTH*blur_scale)));
    shader_int("textureSource",0);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_fullscreen_quad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
}
void shadow_end() {
    // Reset
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if(do_blur) shadow_blur();
    shadow_active = 0;

    glViewport(0, 0, window_width(), window_height());
}
void shadow_ui() {
    // UI
    if( ui_panel("Shadow", 0) ) {
        if(ui_toggle("Debug", &do_debugshadow)) {}
        if(ui_toggle("Blur shadow", &do_blur)) {}
        if(ui_slider("Blur amount", &BLUR_SCALE)) {}
        ui_panel_end();
    }

    if(do_debugshadow) {
    // Blur and draw to screen
    shader_bind(blurProgram); shader_vec2("ScaleU", vec2(0, 0)); // ... but make sure we don't actually blur
    glBindTexture(GL_TEXTURE_2D, shadowMapTex);
    draw_fullscreen_quad();
    glBindTexture(GL_TEXTURE_2D, 0);
    }
}

#define USER_DRAWCALL(shader) do { \
    model_render(sponza, camera_get_active()->proj, camera_get_active()->view, sponza.pivot, shader); \
} while(0)


static void vsm_cube_draw()
{
    glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_WIDTH);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    shader_bind(shadowProgram);

    // For each side of cubemap
    for (int i = 0; i < 6; ++i) {
#if VSMBLUR
        // Draw to temp. storage
        shader_bind(shadowProgram);
        glBindFramebuffer(GL_FRAMEBUFFER, toCurrentSideFBO);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        set_shadow_matrix_uniform(shadowProgram, i);
        USER_DRAWCALL(shadowProgram);

        // Blur horizontally to blurTex
        glDisable(GL_DEPTH_TEST);

        shader_bind(blurProgram);
        shader_vec2("ScaleU", vec2(1.0 / SHADOWMAP_WIDTH, 0));

        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
        glBindTexture(GL_TEXTURE_2D, currentSideTex);
        glClear(GL_COLOR_BUFFER_BIT);
        draw_fullscreen_quad();

        // Blur vertically to actual cubemap
        glBindFramebuffer(GL_FRAMEBUFFER, cubeFBOs[i]);

        glBindTexture(GL_TEXTURE_2D, blurTex);
        shader_bind(blurProgram);
        shader_vec2("ScaleU", vec2(0, 1.0 / SHADOWMAP_WIDTH));

        draw_fullscreen_quad();

        glEnable(GL_DEPTH_TEST);
#else
        // Draw directly to cubemap
        glBindFramebuffer(GL_FRAMEBUFFER, cubeFBOs[i]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        set_shadow_matrix_uniform(shadowProgram, i);
        USER_DRAWCALL(shadowProgram);
#endif
    }

    // Reset state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

#undef shadowProgram
#undef blurProgram
#undef quadMesh
#undef shadowMapFBO
#undef shadowMapTex
#undef shadowMapTexDepth
#undef blurFBO
#undef blurTex
#undef do_blur
#undef do_debugshadow
#undef BLUR_SCALE
#undef SHADOWMAP_WIDTH
#undef shadow_active
#undef lightPos
#undef lightAimPos





// Geometry (world coordinates)
static bool do_animate = 1;

static void color_begin(const GLuint program) {
    glCullFace(GL_BACK);

#if 1 // VSMCUBE
glViewport(0, 0, window_width(), window_height());
glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

    // Upload uniforms
    shader_bind(program);
    shader_mat44("view", camera_get_active()->view);
//  shader_mat44("proj", camera_get_active()->proj);
    shader_vec3("lightPos", shadow.lightPos);

#if VSMCUBE
    set_shadow_matrix_uniform(program, -1);
#else
    mat44 PV; shadow_pv_matrix(PV);
    shader_mat44("cameraToShadowProjector", PV);
#endif

    glActiveTexture(GL_TEXTURE0+1);
#if VSMCUBE
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
#else
    glBindTexture(GL_TEXTURE_2D, shadow.shadowMapTex); //Input-texture
#endif
    shader_int("shadowMap",1);
}
static void color_end() {
#if VSMCUBE
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#else
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
}


int main(int argc, char **argv)
{
    window_create(0.75f, 0);

    camera_t cam = camera();

    // init shadowing 384x384 // 512x512
    shadow_create(argc > 1 ? atoi(argv[1]) : 384);

    sponza = model("sponza.obj", 0);
    translation44(sponza.pivot, 0,-1,0);
    rotate44(sponza.pivot, -90,1,0,0);
    scale44(sponza.pivot, 10,10,10);

    model_t suzanne = model("suzanne.obj", 0);

    // create geometry
    GLuint vsm_program = sponza.program;
    #if 1
    const char *tpl[] = {
        "{{include-shadowmap}}", fs_0_0_shadowmap_lit,
    };
    vsm_program = shader(strlerp(1,tpl,vs_323444143_16_332_model), strlerp(1,tpl,fs_32_4_model), "att_position,att_texcoord,att_normal,att_tangent,att_instanced_matrix,,,,att_indexes,att_weights,att_vertexindex,att_color,att_bitangent", "fragColor", "");
    #endif

    while (window_swap())
    {
        // input
        if (input(KEY_ESC))
            break;

        // fps camera
        bool active = ui_active() ? 0 : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        vec3 wasdec = scale3( vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), 0.2f);
        vec2 mouse = scale2( vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        camera_move(&cam, wasdec.x,wasdec.y,wasdec.z);
        camera_fps(&cam, mouse.x,mouse.y);
        window_cursor( !active );

#if 1
        // animate light
        if( do_animate ) {
            static vec3 lightPos;
            do_once {
                lightPos = cam.position;
            };
            vec3 offs = vec3(sin(window_time()) * 15, 0, cos(window_time()) * 15);
            shadow_position(add3(lightPos, offs));
//            shadow_target(vec3(0,0,0)); // good for pointlight
            shadow_target(add3(add3(lightPos, offs), vec3(0,0,-1)));
        }
#else
            shadow_position(cam.position);
            shadow_target(add3(cam.position, cam.look));
#endif

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render

#if VSMCUBE
        shadow_begin();
            vsm_cube_draw();
        shadow_end();
#else
        shadow_begin();
            model_render(sponza, camera_get_active()->proj, camera_get_active()->view, sponza.pivot, shadow.shadowProgram); //< typo!, but works!
        shadow_end();
#endif

        color_begin(vsm_program);
            model_render(sponza, camera_get_active()->proj, camera_get_active()->view, sponza.pivot, vsm_program); // does not work without program
        color_end();


//         // light bulb (suzanne)
//         {
//         mat44 M; scaling44(M, 10,10,10); relocate44(M, shadow.lightPos.x, shadow.lightPos.y, shadow.lightPos.z );
//         model_render(suzanne, camera_get_active()->proj, camera_get_active()->view, M, 0);
//         }


//         if( ui_panel("App", 0) ) {
//             if(ui_toggle("Animate light", &do_animate)) {}
//             ui_panel_end();
//         }

//         shadow_ui();
    }
}
