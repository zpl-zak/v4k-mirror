#include "v4k.h"

#define LIGHTMAPPER_IMPLEMENTATION
// #define LM_DEBUG_INTERPOLATION
#include "3rd_lightmapper.h"

#define scene_t scene_t2

typedef struct {
    float p[3];
    float t[2];
} vertex_t;

typedef struct
{
    GLuint program;
    GLint u_lightmap;
    GLint u_projection;
    GLint u_view;

    GLuint lightmap;
    int w, h;

    GLuint vao, vbo, ibo;
    vertex_t *vertices;
    unsigned short *indices;
    unsigned int vertexCount, indexCount;
} scene_t;

static int initScene(scene_t *scene);
static void drawScene(scene_t *scene, float *view, float *projection);
static void destroyScene(scene_t *scene);

static int bake(scene_t *scene)
{
    lm_context *ctx = lmCreate(
        64,               // hemisphere resolution (power of two, max=512)
        0.001f, 100.0f,   // zNear, zFar of hemisphere cameras
        1.0f, 1.0f, 1.0f, // background color (white for ambient occlusion)
        2, 0.01f,         // lightmap interpolation threshold (small differences are interpolated rather than sampled)
                          // check debug_interpolation.tga for an overview of sampled (red) vs interpolated (green) pixels.
        0.0f);            // modifier for camera-to-surface distance for hemisphere rendering.
                          // tweak this to trade-off between interpolated normals quality and other artifacts (see declaration).

    if (!ctx)
    {
        fprintf(stderr, "Error: Could not initialize lightmapper.\n");
        return 0;
    }

    int w = scene->w, h = scene->h;
    float *data = CALLOC(w * h * 4, sizeof(float));
    for (int b = 0; b < 1; b++) {
        memset(data, 0, w*h*4);
        lmSetTargetLightmap(ctx, data, w, h, 4);

        lmSetGeometry(ctx, NULL,                                                                 // no transformation in this example
            LM_FLOAT, (unsigned char*)scene->vertices + offsetof(vertex_t, p), sizeof(vertex_t),
            LM_NONE , NULL                                                   , 0               , // no interpolated normals in this example
            LM_FLOAT, (unsigned char*)scene->vertices + offsetof(vertex_t, t), sizeof(vertex_t),
            scene->indexCount, LM_UNSIGNED_SHORT, scene->indices);

        glDisable(GL_BLEND);

        int vp[4];
        float view[16], projection[16];
        double lastUpdateTime = 0.0;
        while (lmBegin(ctx, vp, view, projection))
        {
            // render to lightmapper framebuffer
            glViewport(vp[0], vp[1], vp[2], vp[3]);
            drawScene(scene, view, projection);

            // display progress every second (printf is expensive)
            double time = time_ms() / 1000.0;
            if (time - lastUpdateTime > 1.0)
            {
                lastUpdateTime = time;
                printf("\r%6.2f%%", lmProgress(ctx) * 100.0f);
                fflush(stdout);
            }

            lmEnd(ctx);
    //      window_swap();
        }
        printf("\rFinished baking %d triangles.\n", scene->indexCount / 3);
    }

    lmDestroy(ctx);

    // postprocess texture
    float *temp = CALLOC(w * h * 4, sizeof(float));
    for (int i = 0; i < 16; i++)
    {
        lmImageDilate(data, temp, w, h, 4);
        lmImageDilate(temp, data, w, h, 4);
    }
    lmImageSmooth(data, temp, w, h, 4);
    lmImageDilate(temp, data, w, h, 4);
    lmImagePower(data, w, h, 4, 1.0f / 2.2f, 0x7); // gamma correct color channels
    FREE(temp);

    // save result to a file
    if (lmImageSaveTGAf("result.tga", data, w, h, 4, 1.0f))
        printf("Saved result.tga\n");

    // upload result
    glBindTexture(GL_TEXTURE_2D, scene->lightmap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, data);
    FREE(data);

    return 1;
}

static void fpsCameraViewMatrix(float *view);

static void mainLoop(scene_t *scene)
{
    glViewport(0, 0, window_width(), window_height());

    // camera for glfw window
    float view[16], projection[16];
    fpsCameraViewMatrix(view);
    perspective44(projection, 45.0f, window_aspect(), 0.01f, 100.0f);

    // draw to screen with a blueish sky
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawScene(scene, view, projection);
}

int main()
{
    window_create(0.5, WINDOW_VSYNC_DISABLED);

    scene_t scene = {0};
    if (!initScene(&scene))
    {
        fprintf(stderr, "Could not initialize scene.\n");
        return EXIT_FAILURE;
    }

    window_title("AO Baking demo");

    while (window_swap())
    {
        mainLoop(&scene);

        if( ui_panel("Lightmapper", PANEL_OPEN) ) {
            ui_label2("Freecam", "Mouse + W/A/S/D/E/Q keys");
            ui_label("Warning " ICON_MD_WARNING "@This will take a few seconds and bake a lightmap illuminated by: The mesh itself (initially black) + A white sky (1.0f, 1.0f, 1.0f)");
            if( ui_button("Bake 1 light bounce") ) {
                bake(&scene);
            }
            ui_panel_end();
        }
    }

    destroyScene(&scene);
}

// helpers ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int loadSimpleObjFile(const char *filename, vertex_t **vertices, unsigned int *vertexCount, unsigned short **indices, unsigned int *indexCount);

static int initScene(scene_t *scene)
{
    // load mesh
    if (!loadSimpleObjFile("demos/art/meshes/gazebo.obj", &scene->vertices, &scene->vertexCount, &scene->indices, &scene->indexCount))
    {
        fprintf(stderr, "Error loading obj file\n");
        return 0;
    }

    glGenVertexArrays(1, &scene->vao);
    glBindVertexArray(scene->vao);

    glGenBuffers(1, &scene->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, scene->vbo);
    glBufferData(GL_ARRAY_BUFFER, scene->vertexCount * sizeof(vertex_t), scene->vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &scene->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, scene->indexCount * sizeof(unsigned short), scene->indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, p));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, t));

    // create lightmap texture
    scene->w = 654;
    scene->h = 654;
    glGenTextures(1, &scene->lightmap);
    glBindTexture(GL_TEXTURE_2D, scene->lightmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    unsigned char emissive[] = { 0, 0, 0, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, emissive);

    // load shader
    const char *vp =
        "in vec3 a_position;\n"
        "in vec2 a_texcoord;\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_projection;\n"
        "out vec2 v_texcoord;\n"

        "void main()\n"
        "{\n"
        "gl_Position = u_projection * (u_view * vec4(a_position, 1.0));\n"
        "v_texcoord = a_texcoord;\n"
        "}\n";

    const char *fp =
        "in vec2 v_texcoord;\n"
        "uniform sampler2D u_lightmap;\n"
        "out vec4 o_color;\n"

        "void main()\n"
        "{\n"
        "o_color = vec4(texture(u_lightmap, v_texcoord).rgb, gl_FrontFacing ? 1.0 : 0.0);\n"
        "}\n";

    scene->program = shader(vp, fp, "a_position,a_texcoord", "o_color", NULL);
    if (!scene->program)
    {
        fprintf(stderr, "Error loading shader\n");
        return 0;
    }
    scene->u_view = glGetUniformLocation(scene->program, "u_view");
    scene->u_projection = glGetUniformLocation(scene->program, "u_projection");
    scene->u_lightmap = glGetUniformLocation(scene->program, "u_lightmap");

    return 1;
}

static void drawScene(scene_t *scene, float *view, float *projection)
{
    glEnable(GL_DEPTH_TEST);

    glUseProgram(scene->program);
    glUniform1i(scene->u_lightmap, 0);
    glUniformMatrix4fv(scene->u_projection, 1, GL_FALSE, projection);
    glUniformMatrix4fv(scene->u_view, 1, GL_FALSE, view);

    glBindTexture(GL_TEXTURE_2D, scene->lightmap);

    glBindVertexArray(scene->vao);
    glDrawElements(GL_TRIANGLES, scene->indexCount, GL_UNSIGNED_SHORT, 0);
}

static void destroyScene(scene_t *scene)
{
    FREE(scene->vertices);
    FREE(scene->indices);
    glDeleteVertexArrays(1, &scene->vao);
    glDeleteBuffers(1, &scene->vbo);
    glDeleteBuffers(1, &scene->ibo);
    glDeleteTextures(1, &scene->lightmap);
    glDeleteProgram(scene->program);
}

static int loadSimpleObjFile(const char *filename, vertex_t **vertices, unsigned int *vertexCount, unsigned short **indices, unsigned int *indexCount)
{
    FILE *file = fopen(filename, "rt");
    if (!file)
        return 0;
    char line[1024];

    // first pass
    unsigned int np = 0, nn = 0, nt = 0, nf = 0;
    while (!feof(file))
    {
        fgets(line, 1024, file);
        if (line[0] == '#') continue;
        if (line[0] == 'v')
        {
            if (line[1] == ' ') { np++; continue; }
            if (line[1] == 'n') { nn++; continue; }
            if (line[1] == 't') { nt++; continue; }
            assert(!"unknown vertex attribute");
        }
        if (line[0] == 'f') { nf++; continue; }
        assert(!"unknown identifier");
    }
    assert(np && np == nn && np == nt && nf); // only supports obj files without separately indexed vertex attributes

    // allocate memory
    *vertexCount = np;
    *vertices = CALLOC(np, sizeof(vertex_t));
    *indexCount = nf * 3;
    *indices = CALLOC(nf * 3, sizeof(unsigned short));

    // second pass
    fseek(file, 0, SEEK_SET);
    unsigned int cp = 0, cn = 0, ct = 0, cf = 0;
    while (!feof(file))
    {
        fgets(line, 1024, file);
        if (line[0] == '#') continue;
        if (line[0] == 'v')
        {
            if (line[1] == ' ') { float *p = (*vertices)[cp++].p; char *e1, *e2; p[0] = (float)strtod(line + 2, &e1); p[1] = (float)strtod(e1, &e2); p[2] = (float)strtod(e2, 0); continue; }
            if (line[1] == 'n') { /*float *n = (*vertices)[cn++].n; char *e1, *e2; n[0] = (float)strtod(line + 3, &e1); n[1] = (float)strtod(e1, &e2); n[2] = (float)strtod(e2, 0);*/ continue; } // no normals needed
            if (line[1] == 't') { float *t = (*vertices)[ct++].t; char *e1;      t[0] = (float)strtod(line + 3, &e1); t[1] = (float)strtod(e1, 0);                                continue; }
            assert(!"unknown vertex attribute");
        }
        if (line[0] == 'f')
        {
            unsigned short *tri = (*indices) + cf;
            cf += 3;
            char *e1, *e2, *e3 = line + 1;
            for (int i = 0; i < 3; i++)
            {
                unsigned long pi = strtoul(e3 + 1, &e1, 10);
                assert(e1[0] == '/');
                unsigned long ti = strtoul(e1 + 1, &e2, 10);
                assert(e2[0] == '/');
                unsigned long ni = strtoul(e2 + 1, &e3, 10);
                assert(pi == ti && pi == ni);
                tri[i] = (unsigned short)(pi - 1);
            }
            continue;
        }
        assert(!"unknown identifier");
    }

    fclose(file);
    return 1;
}



static void fpsCameraViewMatrix(float *view)
{
    // initial camera config
    static float position[] = { 0.0f, 0.3f, 1.5f };
    static float rotation[] = { 0.0f, 0.0f };

    // mouse look
    static double lastMouse[] = { 0.0, 0.0 };
    double mouse_coord[2];
    mouse_coord[0] = input(MOUSE_X);
    mouse_coord[1] = input(MOUSE_Y);
    if (input(MOUSE_L))
    {
        rotation[0] += (float)(mouse_coord[1] - lastMouse[1]) * -0.2f;
        rotation[1] += (float)(mouse_coord[0] - lastMouse[0]) * -0.2f;
    }
    lastMouse[0] = mouse_coord[0];
    lastMouse[1] = mouse_coord[1];

    float rotationY[16], rotationX[16], rotationYX[16];
    rotation44(rotationX, rotation[0], 1.0f, 0.0f, 0.0f);
    rotation44(rotationY, rotation[1], 0.0f, 1.0f, 0.0f);
    multiply44x2(rotationYX, rotationY, rotationX);

    // keyboard movement (WSADEQ)
    float speed = input(KEY_SHIFT) ? 0.1f : 0.01f;
    vec3 movement = {0};
    if (input(KEY_W)) movement.z -= speed;
    if (input(KEY_S)) movement.z += speed;
    if (input(KEY_A)) movement.x -= speed;
    if (input(KEY_D)) movement.x += speed;
    if (input(KEY_E)) movement.y -= speed;
    if (input(KEY_Q)) movement.y += speed;

    vec3 worldMovement = transform344(rotationYX, movement);
    position[0] += worldMovement.x;
    position[1] += worldMovement.y;
    position[2] += worldMovement.z;

    // construct view matrix
    float inverseRotation[16], inverseTranslation[16];
    transpose44(inverseRotation, rotationYX);
    translation44(inverseTranslation, -position[0], -position[1], -position[2]);
    multiply44x2(view, inverseRotation, inverseTranslation); // = inverse(translation(position) * rotationYX);
}

#if 0

#######################################################################
     76702  17.93% rendered hemicubes integrated to lightmap texels.
    179388  41.94% interpolated lightmap texels.
    171626  40.13% wasted lightmap texels.

            29.95% of used texels were rendered.
#######################################################################
Finished baking 731 triangles.
Saved result.tga

vs

#######################################################################
       124   0.05% rendered hemicubes integrated to lightmap texels.
       201   0.08% interpolated lightmap texels.
    261947  99.88% wasted lightmap texels.

            38.15% of used texels were rendered.
#######################################################################
Finished baking 731 triangles.
Saved result.tga

#endif

