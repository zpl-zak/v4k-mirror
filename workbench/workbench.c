// #define COOK_ON_DEMAND 1
// #define MAX_CACHED_FILES 0
// #define VFS_ALWAYS_PACK 1
#include "pluginapi.h"

array(editor_t) editors = 0;
array(asset_t) assets = 0;

#define PLUGINS\
    X(shader_editor)

#define X(name) extern editor_vtable_t name##__procs;
    PLUGINS
#undef X

editor_vtable_t load_editor_vtable(char *pname) {
    char *name = va("%s.dll", pname);
    editor_vtable_t f;
    f.init = dll(name, "plug_init");
    f.tick = dll(name, "plug_tick");
    f.quit = dll(name, "plug_quit");
    f.ext = dll(name, "plug_ext");
    if (!f.init || !f.tick || !f.quit || !f.ext)
        return (editor_vtable_t){0};
    return f;
}

void load_editor(char *pname) {
    editor_t ed = {0};
    ed.name = file_base(STRDUP(pname)); // @leak
    ed.f = load_editor_vtable(pname);
    if (!ed.f.init) {
        // PRINTF("unsupported plugin: '%s'\n", ed.name);
        return;
    }
    PRINTF("loaded plugin: '%s'\n", ed.name);
    array_push(editors, ed);
}

void load_editors() {
    #define X(name) load_editor(#name);
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
    app_spawn(va("%s \"\" \"%s\"", ifdef(win32, "start", ifdef(osx, "open", "xdg-open")), fname));
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
    asset.opened = 1;
    asset.last_modified = file_stamp(fname);
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

    #if 0
    load_asset("demos/art/shadertoys/!default.fs");
    load_asset("demos/art/fx/fxDithering.fs");

    #endif

    while( window_swap() && !input(KEY_ESC) ) {
        if (ui_panel("Workbench", PANEL_OPEN)) {
            static const char *file;
            static bool show_browser = 1;
            if( ui_browse(&file, &show_browser) ) {
                load_asset(file);
                show_browser = 1;
            }
        }
        ui_panel_end();

        // static bool show_main_window = 1;
        // if ( ui_window("Workbench", &show_main_window) ) {
        //     ui_label("v4.games");
        // }
        // ui_window_end();

        for (int i=0; i<array_count(assets); i++) {
            asset_t *f = (assets+i);
            if (ui_window(f->name, &f->opened)) {
                f->ed->f.tick(f);

                // was the asset modified?
                bool modified = f->last_modified != file_stamp(f->name);

                ui_separator();

                if (ui_button("reload asset") || modified) {
                    f->last_modified = file_stamp(f->name);
                    f->ed->f.quit(f);
                    f->ed->f.init(f);
                }
                if (ui_button("edit asset")) {
                    edit_asset(assets[i].name);
                }
                if (ui_button("close asset") || !f->opened) {
                    f->ed->f.quit(f);
                    FREE(assets[i].name);
                    array_erase(assets, i);
                    --i;
                }
            }
            ui_window_end();
        }
    }
    return 0;
}
