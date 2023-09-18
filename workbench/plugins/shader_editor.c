#include "pluginapi.h"

enum { WIDTH = 1024, HEIGHT = 1024 };
enum { UI_WIDTH = 512, UI_HEIGHT = 512 };
enum { VS, FS, FX, SHADERTOY };

typedef struct {
	char kind;
	bool reload;

	// preview
	handle fb, flipFB;
	texture_t tex, flipTex;
	texture_t texDepth;

	union {
		struct {
			shadertoy_t shadertoy;
			bool flip_y;
			bool allow_mouse;
		};

		struct {
			int model;
			int fx_slot;
		};
	};
} shader_asset_t;

array(shader_asset_t) shaders = 0;

__declspec(dllexport) char *plug_ext() {
	return "fx,glsl,vs,fs";
}

static inline
void reload_shader(asset_t *f, shader_asset_t *s) {
	if (s->kind == SHADERTOY) {
		s->shadertoy = shadertoy(f->name, (s->flip_y?SHADERTOY_FLIP_Y:0)|(s->allow_mouse?0:SHADERTOY_IGNORE_MOUSE)|SHADERTOY_IGNORE_FBO);
		s->shadertoy.dims.x = WIDTH;
		s->shadertoy.dims.y = HEIGHT;
	}
	else if (s->kind == FX) {
		s->fx_slot = fx_load_from_mem(f->name, vfs_read(f->name));
	}

	s->reload = 0;
}

__declspec(dllexport) int plug_init(asset_t *f) {
	shader_asset_t a = {0};
	a.reload = 1;

	if (strstri(f->name, "shadertoys")) {
		a.kind = SHADERTOY;
	}
	// else if (strbegi(file_name(f->name), "vs_") || strendi(f->name, ".vs")) {
	// 	a.kind = VS;
	// }
	// else if (strbegi(file_name(f->name), "fs_") || strendi(f->name, ".fs")) {
	// 	a.kind = FS;
	// }
	else if (strbegi(file_name(f->name), "fx") && strendi(f->name, ".fs")) {
		a.kind = FX;
	} else {
		PRINTF("unsupported shader: %s\n", f->name);
		return 1;
	}

	a.tex = texture_create(WIDTH, HEIGHT, 4, NULL, TEXTURE_RGBA);
	a.flipTex = texture_create(WIDTH, HEIGHT, 4, NULL, TEXTURE_RGBA);
	a.texDepth = texture_create(WIDTH, HEIGHT, 1, NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
	a.fb = fbo(a.tex.id, a.texDepth.id, 0);
	a.flipFB = fbo(a.flipTex.id, 0, 0);

	f->slot = array_count(shaders);
	array_push(shaders, a);
	return 0;
}

__declspec(dllexport) int plug_tick(asset_t *f) {
	shader_asset_t *s = (shaders+f->slot);

	if (s->reload) {
		reload_shader(f, s);
		ui_dims(f->name, UI_WIDTH, UI_HEIGHT*1.45);
	}

	fx_enable_all(0);

	// vec2 win_dims = ui_get_dims();
	// ui_dims(f->name, win_dims.x, win_dims.x*1.45);

	fbo_bind(s->fb);
	glViewport(0,0,WIDTH,HEIGHT);
	glClearDepth(1);
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	if (s->kind == SHADERTOY) {
		shadertoy_render(&s->shadertoy, window_delta());
	}
	else if (s->kind == FX) {
		fx_enable(s->fx_slot, 1);
		// fx_begin();
		fx_begin_res(WIDTH, HEIGHT);
		enum { CUBE, SPHERE, SUZANNE, SHADERBALL, MAX_MODELS };
		static camera_t cam;
		static skybox_t sky;
		static model_t models[MAX_MODELS];
		do_once {
			cam = camera();
			sky = skybox(0,0);
			camera_fps(&cam, 0,45);
			models[CUBE] = model("meshes/cube.obj", MODEL_NO_ANIMATIONS);
			// model_set_texture(models[CUBE], texture_checker());
            rotation44(models[CUBE].pivot, -90, 1,0,0);
			models[SPHERE] = model("meshes/sphere.obj", MODEL_NO_ANIMATIONS); 
            rotation44(models[SPHERE].pivot, -90, 1,0,0);
			models[SUZANNE] = model("meshes/suzanne.obj", MODEL_NO_ANIMATIONS); 
            rotation44(models[SUZANNE].pivot, -90, 1,0,0);
			models[SHADERBALL] = model("meshes/shaderBall.glb", MODEL_NO_ANIMATIONS); 
			mat44 sca; scale44(sca, 1,1,1);
            mat44 rot; rotation44(rot, -90, 1,0,0);
            multiply44x2(models[SHADERBALL].pivot, sca, rot);
		}

		bool active = (ui_active() || ui_hover()) && input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        // camera_move(&cam, wasdecq.x,wasdecq.y,wasdecq.z);
        camera_orbit(&cam, mouse.x,mouse.y, input_diff(MOUSE_W));
    	perspective44(cam.proj, cam.fov, UI_WIDTH/(float)UI_HEIGHT, 0.1f, 100.f);
    	skybox_render(&sky, cam.proj, cam.view);
    	ddraw_grid(0);
    	ddraw_flush();
		model_render(models[s->model], cam.proj, cam.view, models[s->model].pivot, 0);
		fx_end(s->fb);

		// fbo_bind(s->fb);
		// fullscreen_quad_rgb(s->tex, 1.0);
		// fbo_unbind();
	}

	fbo_bind(s->flipFB);
	fullscreen_quad_rgb(s->tex, 1.0);
	fbo_unbind();

	ui_image(0, s->flipTex.id, UI_WIDTH, UI_HEIGHT);

	if (s->kind == SHADERTOY) {
		if (ui_bool("Flip Y", &s->flip_y)) {
			s->reload = 1;
		}
		if (ui_bool("Allow mouse", &s->allow_mouse)) {
			s->reload = 1;
		}
	}
	else if (s->kind == FX) {
		static char* model_selections[] = {"Cube", "Sphere", "Suzanne", "Shader ball"};
		ui_list("Model", model_selections, 4, &s->model);
	}
	return 0;
}

__declspec(dllexport) int plug_quit(asset_t *f) {
	shader_asset_t *s = (shaders+f->slot);
	
	fbo_destroy(s->fb);
	fbo_destroy(s->flipFB);
	return 0;
}
