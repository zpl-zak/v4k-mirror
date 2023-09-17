#define COOK_ON_DEMAND 1
#include "v4k.c"
#include "pluginapi.h"

array(editor_t) editors = 0;
array(asset_t) assets = 0;

#define PLUGINS\
    X(shader_editor)

#define X(name) extern editor_vtable_t name##__procs;
    PLUGINS
#undef X

void load_editor(char *pname, editor_vtable_t f) {
    char *name = STRDUP(pname); // @leak

    editor_t ed = {0};
    ed.f = f;
    ed.name = file_base(STRDUP(name)); // @leak
    PRINTF("loaded plugin: '%s'\n", ed.name);
    array_push(editors, ed);
}

void load_editors() {
    #define X(name) load_editor(#name, name##__procs);
        PLUGINS
    #undef X
}

map(char*, editor_t*) assocs;

void register_extensions() {
    map_init(assocs, less_str, hash_str);

    for (int i=0; i<array_count(editors); i++) {
        char *types = editors[i].f.ext();

        for each_substring(types, ",", ext) {
            map_find_or_add(assocs, STRDUP(ext), (editors+i));
            PRINTF("extension '%s' mapped to '%s'\n", ext, editors[i].name);
        }
    }
}

void edit_asset(char *fname) {
    app_exec(va("%s %s", ifdef(win32, "start", ifdef(osx, "open", "xdg-open")), fname));
}

void load_asset(const char *fname) {
    char *ext = (file_ext(fname)+1);
    PRINTF("opened asset: %s (%s)\n", fname, ext);

    // see if we have a supported editor plugin registered
    editor_t **ed = map_find(assocs, ext);
    if (!ed) {
        PRINTF("fallback exec %s\n", fname);
        edit_asset(fname);
        return;
    }

    asset_t asset = {0};
    asset.name = STRDUP(fname);
    asset.ed = *ed;
    array_push(assets, asset);
    if (asset.ed->f.init((struct asset_t*)array_back(assets))) {
        FREE(asset.name);
        array_pop(assets);
        PRINTF("fallback exec %s\n", fname);
        edit_asset(fname);
    }
}

int main() {
    window_create(75.0, WINDOW_MSAA8);
    window_fps_unlock();

    camera_t cam = camera();
    camera_enable(&cam);

    load_editors();
    register_extensions();

    #if 1
    load_asset("demos/art/shadertoys/!default.fs");
    load_asset("demos/art/fx/fxDithering.fs");

    #endif

    while( window_swap() && !input(KEY_ESC) ) {
        static int wb_enabled = 1;
        if (ui_window("Workbench", &wb_enabled)) {
            static const char *file;
            static bool show_browser = 1;
            if( ui_browse(&file, &show_browser) ) {
                load_asset(file);
                show_browser = 1;
            }
            ui_window_end();
        }

        for (int i=0; i<array_count(assets); i++) {
            asset_t *f = (assets+i);
            int open = 1;
            if (ui_window(f->name, &open)) {
                f->ed->f.tick(f);

                ui_separator();
                if (ui_button("reload asset") || !open) {
                    f->ed->f.quit(f);
                    f->ed->f.init(f);
                }
                if (ui_button("edit asset") || !open) {
                    edit_asset(assets[i].name);
                }
                if (ui_button("close asset") || !open) {
                    f->ed->f.quit(f);
                    FREE(assets[i].name);
                    array_erase(assets, i);
                    --i;
                }
                ui_window_end();
            }
        }
    }
    return 0;
}
