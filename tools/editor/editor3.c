// ## Editor long-term plan
// - editor = tree of nodes. levels and objects are nodes, and their widgets are also nodes
// - you can perform actions on nodes, with or without descendants, top-bottom or bottom-top
// - these operations include load/save, undo/redo, reset, play/render, ddraw, etc
// - nodes are saved to disk as a filesystem layout: parents are folders, and leafs are files
// - network replication can be done by external tools by comparing the filesystems and by sending the resulting diff zipped
//
// ## Editor roadmap
// - Gizmos✱, scene tree, property editor✱, load/save✱, undo/redo✱, copy/paste, on/off (vis,tick,ddraw,log), vcs.
// - Scenenode pass: node singleton display, node console, node labels, node outlines✱.<!-- node == gameobj ? -->
// - Render pass: billboards✱, materials, un/lit, cast shadows, wireframe, skybox✱/mie✱, fog/atmosphere
// - Level pass: volumes, triggers, platforms, level streaming, collide✱, physics
// - Edit pass: Procedural content, brushes, noise and CSG.
// - GUI pass: timeline and data tracks, node graphs. <!-- worthy: will be reused into materials, animgraphs and blueprints -->

// ## Alt plan
// editor is a database + window/tile manager + ui toolkit; all network driven.
// to be precise, editor is a dumb app and ...
// - does not know a thing about what it stores.
// - does not know how to render the game graphics.
// - does not know how to run the game logic.
//
// the editor will create a canvas for your game to render.
// your game will be responsible to tick the logic and render the window inside the editor.
//
// that being said, editor...
// - can store datas hierarchically.
// - can perform diffs and merges, and version the datas into repositories.
// - can be instructed to render UI on top of game and window views.
// - can download new .natvis and plugins quickly.
// - can dump whole project in a filesystem form (zip).

// - editor reflects database contents up-to-date.
// - database can be queried and modified via OSC(UDP) commands.

// editor database uses one table, and stores two kind of payload types:
// - classes: defines typename and dna. class names are prefixed by '@'
// - instances: defines typename and datas. instance names are as-is, not prefixed.
//
// every save contains 5Ws: what, who, when, where, how,
// every save can be diffed/merged.

// ----------------------------------------------------------------------------

#define COOK_ON_DEMAND 1 // @fixme: these directives should be on client, not within v4k.dll
#define ENABLE_AUTOTESTS 1
#define V4K_IMPLEMENTATION
#include "v4k.c"
#include "3rd_icon_mdi.h"
//#include "objtests.h"
#include "editor3.h"
#define EXTEND obj_extend

// ----------------------------------------------------------------------------

TODO("file_id: glow.hdr vs glow.png");
TODO("reflect: iterate components+metas on REFLECT too, so they're properly saved/loaded");

TODO("edit: tree nav");
TODO("edit:   keys up,down,left,right -> move selection");
TODO("edit:   reordering/dragging items on a list. ctrl+cursors");
TODO("edit:   tab -> cycle next node of matching highlighted type");
TODO("edit: ^C^V^X thru clipboard. ^C to serialize to clipboard.");
TODO("edit: ^Z^Y cursor too. also fix undo ordering");
TODO("edit: ^S^L^N. load/save as filesystems");
TODO("edit: ^B(toolbar)");
TODO("edit: ^E prop single-view for multiple selections: should inspect common fields only");
TODO("edit:    two-column way (or Nth) for multiple selections");
TODO("edit: tab/caps view, world + entity only, obj printf");
TODO("edit: obj bounds, obj picking, obj collisions");
TODO("edit:   LMB object picking, RMB object serialization + log, floating ICONS bulb light");
TODO("edit:   worldtraveller component");
TODO("edit:   levelstreamer component");
TODO("edit: OSC server/client port 2023");
TODO("edit: add/rem entities, add/rem components, add/rem/pause/resume systems");
TODO("edit: event loop: tick,draw*,spawn,delete,un/load from bvh stream,");

TODO("edit: overlay scene editor");
TODO("edit:   overlay0 artwork");
TODO("edit:   overlay1 gizmo, grid, snap, splats (@todo: fixed screen size gizmos)");
TODO("edit:   overlay2 script editor");
TODO("edit:   overlay3 track, spline, keys editor");
TODO("edit:   overlay4 node editor (shader/anim/bt/hfsm/material/audio/blueprints)");
TODO("edit:   overlay5 csv editor");
TODO("edit:   overlay6 bug/task editor");

TODO("gfx: tree traversal from game");
TODO("gfx:   bvh and collision queries");
TODO("gfx:   visibility and pvs queries");

TODO("obj: finish SYSTEMS and join queries");
TODO("obj: make it work with /GL flag (VC)");
TODO("obj: impl obj_mutate() ... deprecate?");
TODO("obj: make() from mpack(m) + native(n)");
TODO("obj: make() should save schema `[{mn`+version. and (m)pack and (n)ative should start with objtype");
TODO("obj: super()");
TODO("obj: lock()/trylock()/unlock()/barrier(N)");
TODO("obj: diff()/patch()");
TODO("obj: free obj_children()/payload");
TODO("obj: free obj_components()/payload2");

TODO("pack: mp2json, json2mp");
TODO("pack: simplify msgpack API, make it growth similar to va()")
#if 0 // v4k_pack proposal
static __thread char*    mpin;
static __thread unsigned mpinlen;
static __thread char     mpinbuf[256];
static __thread char*    mpout;
static __thread unsigned mpoutlen;
static __thread char     mpoutbuf[256];
#define PACKMSG(...) (msgpack_new(mpin = mpinbuf, mpinlen = sizeof(mpinbuf)), mpinlen = msgpack(__VA_ARGS__), cobs_encode(mpin, mpinlen, mpout = mpoutbuf, mpoutlen = cobs_bounds(mpinlen)), mpout)
#define UNPACKMSG(ptr,fmt,...) (mpin = (char*)ptr, mpinlen = strlen(ptr), mpout = mpoutbuf, mpoutlen = sizeof(mpoutbuf), mpoutlen = cobs_decode(mpin, mpinlen, mpout, mpoutlen), msgunpack_new(mpout, mpoutlen) && msgunpack(fmt, __VA_ARGS__))
#endif

// ----------------------------------------------------------------------------

array(void*) editor_persist_kv;

#define editor_new_property(property_name,T,defaults) \
typedef map(void*,T) editor_##property_name##_map_t; \
editor_##property_name##_map_t *editor_##property_name##_map() { \
    static editor_##property_name##_map_t map = 0; do_once map_init_ptr(map); \
    return &map; \
} \
T editor_##property_name(const void *obj) { \
    return *map_find_or_add(*editor_##property_name##_map(), (void*)obj, ((T) defaults)); \
} \
void editor_set##property_name(const void *obj, T value) { \
    *map_find_or_add(*editor_##property_name##_map(), (void*)obj, ((T) value)) = ((T) value); \
} \
void editor_alt##property_name(const void *obj) { \
    T* found = map_find_or_add(*editor_##property_name##_map(), (void*)obj, ((T) defaults)); \
    *found = (T)(uintptr_t)!(*found); \
} \
void editor_no##property_name(void *obj) { \
    T* found = map_find_or_add(*editor_##property_name##_map(), (void*)obj, ((T) defaults)); \
    map_erase(*editor_##property_name##_map(), (void*)obj); \
} \
AUTORUN { array_push(editor_persist_kv, #T); array_push(editor_persist_kv, editor_##property_name##_map()); }

editor_new_property(open,         int,    0);
editor_new_property(selected,     int,    0);
editor_new_property(changed,      int,    0);
editor_new_property(bookmarked,   int,    0);
editor_new_property(visible,      int,    0);
editor_new_property(script,       int,    0);
editor_new_property(event,        int,    0);
editor_new_property(iconinstance, char*,  0);
editor_new_property(iconclass,    char*,  0);
editor_new_property(treeoffsety,  int,    0);
// new prop: breakpoint: request to break on any given node

void editor_no_properties(void *o) {
editor_noopen(o);
editor_noselected(o);
editor_nochanged(o);
editor_nobookmarked(o);
editor_novisible(o);
editor_noscript(o);
editor_noevent(o);
editor_noiconinstance(o);
editor_noiconclass(o);
editor_notreeoffsety(o);
}

void editor_load_on_boot(void) {
    puts("@todo: load editor");
}
void editor_save_on_quit(void) {
    puts("@todo: save editor");
}
AUTORUN {
    editor_load_on_boot();
    (atexit)(editor_save_on_quit);
}

// ----------------------------------------------------------------------------

struct editor_t {
    // time
    unsigned   frame;
    double     t, dt, slomo;
    // controls
    int        attached;
    int        active; // focus? does_grabinput instead?
    int        key;
    vec2       mouse; // 2d coord for ray/picking
    bool       gamepad; // mask instead? |1|2|4|8
    int        hz;
    int        hz_mid;
    int        hz_low;
    int        filter;
    bool       powersave;
    bool       lit;
    bool       ddraw;
    // event root nodes
    obj* root;
    obj*  init;
    obj*   tick;
    obj*   draw;
    obj*   edit;
    obj*  quit;
    // all of them
    array(obj*) objs; // @todo:set() world?
    array(char*) cmds;
} editor = {
    .active = 1,
    .gamepad = 1,
    .hz = 60,
    .hz_mid = 18,
    .hz_low = 5,
    .lit = 1,
    .ddraw = 1,
};

bool editor_active() {
    return ui_hover() || ui_active() || gizmo_active() ? editor.active : 0;
}

int editor_filter() {
    if( editor.filter ) {
        if (nk_begin(ui_ctx, "Filter", nk_rect(window_width()-window_width()*0.33,32, window_width()*0.33, 40),
            NK_WINDOW_NO_SCROLLBAR)) {

            char *bak = ui_filter; ui_filter = 0;
            ui_string(ICON_MD_CLOSE " Filter " ICON_MD_SEARCH, &bak);
            ui_filter = bak;

            if( input(KEY_ESC) || ( ui_label_icon_clicked_L.x > 0 && ui_label_icon_clicked_L.x <= 24 )) {
                if( ui_filter ) ui_filter[0] = '\0';
                editor.filter = 0;
            }
        }
        nk_end(ui_ctx);
    }

    return editor.filter;
}

static
int editor_select_(void *o, const char *mask) {
    int matches = 0;
    int off = mask[0] == '!', inv = mask[0] == '~';
    int match = strmatchi(obj_type(o), mask+off+inv) || strmatchi(obj_name(o), mask+off+inv);
    if( match ) {
        editor_setselected(o, inv ? editor_selected(o) ^ 1 : !off);
        ++matches;
    }
    for each_objchild(o, obj*, oo) {
        matches += editor_select_(oo, mask);
    }
    return matches;
}
void editor_select(const char *mask) {
    for each_array( editor.objs, obj*, o )
        editor_select_(o, mask);
}
void editor_unselect() { // same than editor_select("!**");
    for each_map_ptr(*editor_selected_map(), void*,o, int, k) {
        if( *k ) *k = 0;
    }
}


static obj* active_ = 0;
static void editor_selectgroup_(obj *o, obj *first, obj *last) {
    // printf("%s (looking for %s in [%s..%s])\n", obj_name(o), active_ ? obj_name(active_) : "", obj_name(first), obj_name(last));
    if( !active_ ) if( o == first || o == last ) active_ = o == first ? last : first;
    if( active_ ) editor_setselected(o, 1);
    if( o == active_ ) active_ = 0;
    for each_objchild(o, obj*, oo) {
        editor_selectgroup_(oo, first, last);
    }
}
void editor_selectgroup(obj *first, obj *last) {
    if( last ) {
        if( !first ) first = array_count(editor.objs) ? editor.objs[0] : NULL;
        if( !first ) editor_setselected(last, 1);
        else {
            active_ = 0;
            for each_array(editor.objs,obj*,o) {
                editor_selectgroup_(o, first, last);
            }
        }
    }
}

static obj *find_any_selected_(obj *o) {
    if( editor_selected(o) ) return o;
    for each_objchild(o,obj*,oo) {
        obj *ooo = find_any_selected_(oo);
        if( ooo )
            return ooo;
    }
    return 0;
}
void* editor_first_selected() {
    for each_array(editor.objs,obj*,o) {
        obj *oo = find_any_selected_(o);
        // if( oo ) printf("1st found: %s\n", obj_name(oo));
        if( oo ) return oo;
    }
    return 0;
}

static obj *find_last_selected_(obj *o) {
    void *last = 0;
    if( editor_selected(o) ) last = o;
    for each_objchild(o,obj*,oo) {
        obj *ooo = find_last_selected_(oo);
        if( ooo )
            last = ooo;
    }
    return last;
}
void* editor_last_selected() {
    void *last = 0;
    for each_array(editor.objs,obj*,o) {
        obj *oo = find_last_selected_(o);
        // if( oo ) printf("last found: %s\n", obj_name(oo));
        if( oo ) last = oo;
    }
    return last;
}

// ----------------------------------------------------------------------------------------

void editor_watch(const void *o) {
    array_push(editor.objs, (obj*)o);
    obj_push(o); // save state
}
void* editor_spawn(const char *ini) { // deprecate?
    obj *o = obj_make(ini);
    editor_watch(o);
    return o;
}
void editor_spawn1() {
    obj *selected = editor_first_selected();
    obj *o = selected ? obj_make(obj_saveini(selected)) : obj_new(obj);
    if( selected ) obj_attach(selected, o), editor_setopen(selected, 1);
    else
    editor_watch(o);

    editor_unselect();
    editor_setselected(o, 1);
}

typedef set(obj*) set_objp_t;
static
void editor_glob_recurse(set_objp_t*list, obj *o) {
    set_find_or_add(*list, o);
    for each_objchild(o,obj*,oo) {
        editor_glob_recurse(list, oo);
    }
}
void editor_destroy_selected() {
    set_objp_t list = 0;
    set_init_ptr(list);
    for each_map_ptr(*editor_selected_map(), obj*,o, int,selected) {
        if( *selected ) { editor_glob_recurse(&list, *o); }
    }
    for each_set(list, obj*, o) {
        obj_detach(o);
    }
    for each_set(list, obj*, o) {
        // printf("deleting %p %s\n", o, obj_name(o));
        // remove from watched items
        for (int i = 0, end = array_count(editor.objs); i < end; ++i) {
            if (editor.objs[i] == o) {
                editor.objs[i] = 0;
                array_erase_slow(editor.objs, i);
                --end;
                --i;
            }
        }
        // delete properties + obj
        editor_no_properties(o);
        obj_free(o);
    }
    set_free(list);
}
void editor_inspect(obj *o) {
    ui_section(va("%s (%s)", obj_type(o), obj_name(o)));

    if( obj_edit[obj_typeid(o)] ) {
        obj_edit(o);
    }

    for each_objmember(o,TYPE,NAME,PTR) {
        if( !editor_changed(PTR) ) {
            obj_push(o);
        }
        ui_label_icon_highlight = editor_changed(PTR); // @hack: remove ui_label_icon_highlight hack
        char *label = va(ICON_MD_UNDO "%s", NAME);
        int changed = 0;
        /**/ if( !strcmp(TYPE,"float") ) changed = ui_float(label, PTR);
        else if( !strcmp(TYPE,"vec2") )  changed = ui_float2(label, PTR);
        else if( !strcmp(TYPE,"vec3") )  changed = ui_float3(label, PTR);
        else if( !strcmp(TYPE,"vec4") )  changed = ui_float4(label, PTR);
        else if( !strcmp(TYPE,"color") ) changed = ui_color4(label, PTR);
        else if( !strcmp(TYPE,"char*") ) changed = ui_string(label, PTR);
        else ui_label2(label, va("(%s)", TYPE)); // INFO instead of (TYPE)?
        if( changed ) {
            editor_setchanged(PTR, 1);
        }
        if( ui_label_icon_highlight )
        if( ui_label_icon_clicked_L.x >= 6 && ui_label_icon_clicked_L.x <= 26 ) { // @hack: if clicked on UNDO icon (1st icon)
            editor_setchanged(PTR, 0);
        }
        if( !editor_changed(PTR) ) {
            obj_pop(o);
        }
    }
}

enum {
    TREE_RECURSE = 1,
    TREE_SELECTION = 2,
    TREE_CHECKBOX = 4,
    TREE_INDENT = 8,
    TREE_ALL = ~0u
};

static
void editor_tree_(obj *o, unsigned flags) {
    static unsigned tabs = ~0u;
    ++tabs;

    if( o ) {
        unsigned do_tags = 1;
        unsigned do_indent    = !!(flags & TREE_INDENT);
        unsigned do_checkbox  = !!(flags & TREE_CHECKBOX);
        unsigned do_recurse = !!(flags & TREE_RECURSE);
        unsigned do_selection = !!(flags & TREE_SELECTION);

        nk_layout_row_dynamic(ui_ctx, 25, 1);

        const char *objicon = editor_iconinstance(o);
        if(!objicon) objicon = editor_iconclass(obj_type(o));
        if(!objicon) objicon = ICON_MDI_CUBE_OUTLINE;

        const char *objname = va("%s (%s)", obj_type(o), obj_name(o));

        const char *objchevron =
            !do_recurse || array_count(*obj_children(o)) <= 1 ? ICON_MDI_CIRCLE_SMALL :
            editor_open(o) ? ICON_MDI_CHEVRON_DOWN : ICON_MDI_CHEVRON_RIGHT;

        char *label = va("%*s%s%s %s", do_indent*(4+2*tabs), "", objchevron, objicon, objname);

        const char *iconsL =
            //editor_selected(o) ? ICON_MD_CHECK_BOX : ICON_MD_CHECK_BOX_OUTLINE_BLANK;
            editor_selected(o) ? ICON_MDI_CHECKBOX_MARKED : ICON_MDI_CHECKBOX_BLANK_OUTLINE;

        const char *iconsR = va("%s%s%s",
            editor_script(o) ? ICON_MDI_SCRIPT : ICON_MDI_CIRCLE_SMALL,
            editor_event(o) ? ICON_MDI_CALENDAR : ICON_MDI_CIRCLE_SMALL,
            editor_visible(o) ? ICON_MDI_EYE_OUTLINE : ICON_MDI_EYE_CLOSED );

        UI_TOOLBAR_OVERLAY_DECLARE(int choiceL, choiceR);

        struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
        struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx);

        int clicked = nk_hovered_text(ui_ctx, label, strlen(label), NK_TEXT_LEFT, editor_selected(o));
        if( clicked && nk_input_is_mouse_hovering_rect(&ui_ctx->input, ((struct nk_rect) { bounds.x,bounds.y,bounds.w*0.66,bounds.h })) )
            editor_altselected( o );

        vec2i treeoffset = {0};

        if( do_indent ) {
            float thickness = 2.f;
            struct nk_color color = {255,255,255,64};

            int offsx = 30;
            int spacx = 10;
            int lenx = (tabs+1)*spacx;
            int halfy = bounds.h / 2;
            int offsy = halfy + 2;

            treeoffset = vec2i(bounds.x+offsx+lenx-spacx,bounds.y+offsy);

            editor_settreeoffsety(o, treeoffset.y);

            for( obj *p = obj_parent(o); p ; p = 0 )
            nk_stroke_line(canvas, treeoffset.x-6,treeoffset.y, treeoffset.x-spacx,treeoffset.y, thickness, color),
            nk_stroke_line(canvas, treeoffset.x-spacx,treeoffset.y,treeoffset.x-spacx,editor_treeoffsety(p)+4, thickness, color);
        }

        if( ui_contextual() ) {
            API int editor_send(const char *);

            int choice = ui_label(ICON_MD_BOOKMARK_ADDED "Toggle bookmarks (CTRL+B)");
            if( choice & 1 ) editor_send("bookmark");

            ui_contextual_end(!!choice);
        }

        UI_TOOLBAR_OVERLAY(choiceL,iconsL,nk_rgba_f(1,1,1,do_checkbox*ui_alpha*0.65),NK_TEXT_LEFT);

        if( do_tags )
        UI_TOOLBAR_OVERLAY(choiceR,iconsR,nk_rgba_f(1,1,1,ui_alpha*0.65),NK_TEXT_RIGHT);

        if( choiceR == 3 ) editor_altscript( o );
        if( choiceR == 2 ) editor_altevent( o);
        if( choiceR == 1 ) editor_altvisible( o );

        if( do_recurse && editor_open(o) ) {
            for each_objchild(o,obj*,oo) {
                editor_tree_(oo,flags);
            }
        }

        if( clicked && !choiceL && !choiceR ) {
            int is_picking = input(KEY_CTRL);
            if( !is_picking ) {
                if( input(KEY_SHIFT) ) {
                    editor_selectgroup( editor_first_selected(), editor_last_selected() );
                } else {
                    editor_unselect();
                    editor_setselected(o, 1);
                }
            }
            for( obj *p = obj_parent(o); p; p = obj_parent(p) ) {
                editor_setopen(p, 1);
            }
            if( nk_input_is_mouse_hovering_rect(&ui_ctx->input, ((struct nk_rect) { bounds.x,bounds.y,treeoffset.x-bounds.x+UI_ICON_FONTSIZE/2,bounds.h })) ) {
                editor_altopen( o );
            }
        }
    }

    --tabs;
}

void editor_tree() {
    // #define HELP ICON_MDI_INFORMATION_OUTLINE "@-A\n-B\n-C\n" ";"
    int choice = ui_toolbar(ICON_MDI_PLUS "@New node (CTRL+N);" ICON_MDI_DOWNLOAD "@Save node (CTRL+S);" ICON_MDI_DOWNLOAD "@Save scene (SHIFT+CTRL+S);" ICON_MD_BOOKMARK_ADDED "@Toggle Bookmark (CTRL+B);");
    if( choice == 1 ) editor_send("node.new");
    if( choice == 2 ) editor_send("node.save");
    if( choice == 3 ) editor_send("tree.save");
    if( choice == 4 ) editor_send("bookmark");

    array(obj*) bookmarks = 0;
    for each_map_ptr(*editor_bookmarked_map(), void*,o,int,bookmarked) {
        if( *bookmarked ) {
            array_push(bookmarks, *o);
        }
    }
    if( ui_collapse("!" ICON_MD_BOOKMARK "Bookmark.toggles", "DEBUG:BOOKMARK")) {
        for each_array( bookmarks, obj*, o )
            editor_tree_( o, TREE_ALL & ~(TREE_RECURSE|TREE_INDENT|TREE_CHECKBOX) );
        ui_collapse_end();
    }
    array_free(bookmarks);

    editor_tree_( editor.root, TREE_ALL );

    for each_array( editor.objs, obj*, o )
        editor_tree_( o, TREE_ALL );

    ui_separator();

    // edit selection
    for each_map(*editor_selected_map(), void*,o, int, k) {
        if( k ) editor_inspect(o);
    }
}

// ----------------------------------------------------------------------------------------
// tty

static thread_mutex_t *console_lock;
static array(char*) editor_jobs;
int editor_send(const char *cmd) { // return job-id
    int skip = strspn(cmd, " \t\r\n");
    char *buf = STRDUP(cmd + skip);
    strswap(buf, "\r\n", "");
    int jobid;
    do_threadlock(console_lock) {
        array_push(editor_jobs, buf);
        jobid = array_count(editor_jobs) - 1;
    }
    return jobid;
}
const char* editor_recv(int jobid, double timeout_ss) {
    char *answer = 0;

    while(!answer && timeout_ss >= 0 ) {
        do_threadlock(console_lock) {
            if( editor_jobs[jobid][0] == '\0' )
                answer = editor_jobs[jobid];
        }
        timeout_ss -= 0.1;
        if( timeout_ss > 0 ) sleep_ms(100); // thread_yield()
    }

    return answer + 1;
}

typedef struct editor_bind_t {
    const char *command;
    const char *bindings;
    void (*fn)();
} editor_bind_t;

array(editor_bind_t) editor_binds;

#define EDITOR_BIND(CMD,KEYS,...) void macro(editor_bind_fn_)() { __VA_ARGS__ }; AUTORUN { array_push(editor_binds, ((editor_bind_t){CMD,KEYS,macro(editor_bind_fn_)}) ); }

EDITOR_BIND("play", "held(CTRL) & down(SPC)",             { window_pause(0); if(!editor.slomo) editor.active = 0; editor.slomo = 1; } );
EDITOR_BIND("slomo", "",                                  { window_pause(0); editor.slomo = maxf(fmod(editor.slomo * 2, 16), 0.125); } );
EDITOR_BIND("stop", "(held(ALT)|held(SHIFT))&down(ESC)",  { window_pause(1), editor.frame = 0, editor.t = 0, editor.dt = 0, editor.slomo = 0, editor.active = 1; editor_select("**"); editor_destroy_selected(); } );
EDITOR_BIND("pause", "down(ESC)",                         { window_pause( window_has_pause() ^ 1 ); } );
EDITOR_BIND("frame", "held(CTRL) & held(RIGHT)",          { window_pause(1); editor.frame++, editor.t += (editor.dt = 1/60.f); } );
EDITOR_BIND("eject", "held(SHIFT) & down(F1)",            { editor.active ^= 1; } );
EDITOR_BIND("reload", "down(F5)",                         { window_reload(); } );
EDITOR_BIND("quit", "held(ALT) & down(F4)",               { record_stop(), exit(0); } );
EDITOR_BIND("battery", "held(ALT) & down(B)",             { editor.powersave ^= 1; } );
EDITOR_BIND("browser", "down(F1)",                        { ui_show("Browser", ui_visible("Browser") ^ true); } );
EDITOR_BIND("outliner", "held(ALT) & down(O)",            { ui_show("Outliner", ui_visible("Outliner") ^ true); } );
EDITOR_BIND("profiler", "held(ALT) & down(P)",            { ui_show("Profiler", profiler_enable(ui_visible("Profiler") ^ true)); } );
EDITOR_BIND("fullscreen", "down(F11)",                    { record_stop(), window_fullscreen( window_has_fullscreen() ^ 1 ); } ); // close any recording before framebuffer resizing, which would corrupt video stream
EDITOR_BIND("mute", "held(ALT) & down(M)",                { audio_volume_master( 1 ^ !!audio_volume_master(-1) ); } );
EDITOR_BIND("filter", "held(CTRL) & down(F)",             { editor.filter ^= 1; } );
EDITOR_BIND("gamepad", "held(ALT) & down(G)",             { editor.gamepad ^= 1; } );
EDITOR_BIND("lit", "held(ALT) & down(L)",                 { editor.lit ^= 1; } );
EDITOR_BIND("ddraw", "held(ALT) & down(D)",               { editor.ddraw ^= 1; } );
EDITOR_BIND("node.new", "down(INS)",                      { editor_spawn1(); } );
EDITOR_BIND("node.del", "down(DEL)",                      { editor_destroy_selected(); } );
EDITOR_BIND("node.save", "held(CTRL)&down(S)",            { puts("@todo"); } );
EDITOR_BIND("tree.save", "held(CTRL)&down(S)&held(SHIFT)",{ puts("@todo"); } );
EDITOR_BIND("select.all", "held(CTRL) & down(A)",         { editor_select("**"); } );
EDITOR_BIND("select.none", "held(CTRL) & down(D)",        { editor_select("!**"); } );
EDITOR_BIND("select.invert", "held(CTRL) & down(I)",      { editor_select("~**"); } );
EDITOR_BIND("screenshot", "held(ALT) & down(X)",          { char *name = file_counter(va("%s.png",app_name())); window_screenshot(name), ui_notify(va("Screenshot: %s", name), date_string()); } );
EDITOR_BIND("record", "held(ALT) & down(Z)",              { if(record_active()) record_stop(), ui_notify(va("Video recorded"), date_string()); else { char *name = file_counter(va("%s.mp4",app_name())); app_beep(), window_record(name); } } );
EDITOR_BIND("bookmark", "held(CTRL) & down(B)",           { editor_selected_map_t *map = editor_selected_map(); \
    int on = 0; \
    for each_map_ptr(*map,void*,o,int,selected) if(*selected) on |= !editor_bookmarked(*o); \
    for each_map_ptr(*map,void*,o,int,selected) if(*selected) editor_setbookmarked(*o, on); \
} );

void editor_pump() {
    for each_array(editor_binds,editor_bind_t,b) {
        if( input_eval(b.bindings) ) {
            editor_send(b.command);
        }
    }

    do_threadlock(console_lock) {
        for each_array_ptr(editor_jobs,char*,cmd) {
            if( (*cmd)[0] ) {
                int found = 0;
                for each_array(editor_binds,editor_bind_t,b) {
                    if( !strcmpi(b.command, *cmd)) {
                        b.fn();
                        found = 1;
                        break;
                    }
                }

                if( !found ) {
                    // alert(va("Editor: could not handle `%s` command.", *cmd));
                    (*cmd)[0] = '\0'; strcatf(&(*cmd), "\1%s\n", "Err\n"); (*cmd)[0] = '\0';
                }

                if( (*cmd)[0] ) {
                    (*cmd)[0] = '\0'; strcatf(&(*cmd), "\1%s\n", "Ok\n"); (*cmd)[0] = '\0';
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------------------

void editor_frame( void (*game)(unsigned, float, double) ) {
    do_once {
        //set_init_ptr(editor.world);
        //set_init_ptr(editor.selection);
        profiler_enable( false );

        window_pause( true );
        window_cursor_shape(CURSOR_SW_AUTO);

        fx_load("editorOutline.fs");
        fx_enable(0, 1);

        obj_setname(editor.root = obj_new(obj), "Signals");
        obj_setname(editor.init = obj_new(obj), "Init");
        obj_setname(editor.tick = obj_new(obj),  "Tick");
        obj_setname(editor.draw = obj_new(obj),  "Draw");
        obj_setname(editor.quit = obj_new(obj), "Quit");
//      obj_setname(editor.book = obj_new(obj), "Bookmark.toggles");

        obj_attach(editor.root, editor.init);
        obj_attach(editor.root, editor.tick);
        obj_attach(editor.root, editor.draw);
        obj_attach(editor.root, editor.quit);

        editor_seticoninstance(editor.root, ICON_MDI_SIGNAL_VARIANT);
        editor_seticoninstance(editor.init, ICON_MDI_SIGNAL_VARIANT);
        editor_seticoninstance(editor.tick, ICON_MDI_SIGNAL_VARIANT);
        editor_seticoninstance(editor.draw, ICON_MDI_SIGNAL_VARIANT);
        editor_seticoninstance(editor.quit, ICON_MDI_SIGNAL_VARIANT);
//      editor_seticoninstance(editor.book, ICON_MD_BOOKMARK_ADDED);

        editor_seticonclass(obj_type(editor.root), ICON_MDI_CUBE_OUTLINE);
    }

    // game tick
    game(editor.frame, editor.dt, editor.t);

    // timing
    editor.dt = clampf(window_delta(), 0, 1/60.f) * !window_has_pause() * editor.slomo;
    editor.t += editor.dt;
    editor.frame += !window_has_pause();
    editor.frame += !editor.frame;

    // process inputs & messages
    editor_pump();

    // draw menubar
    static double last_fps = 0; if(!window_has_pause()) last_fps = window_fps();
    int fps_target = window_fps_target() > 0.0f ? (int)window_fps_target() : 60;
    const char *TITLE = va("%02dF %5.2f/%dfps x%4.3f", editor.frame % (int)fps_target, last_fps, fps_target, editor.slomo);
    const char *ICON_PL4Y = window_has_pause() ? ICON_MDI_PLAY : ICON_MDI_PAUSE;
    const char *ICON_SKIP = window_has_pause() ? ICON_MDI_STEP_FORWARD/*ICON_MDI_SKIP_NEXT*/ : ICON_MDI_FAST_FORWARD;

    #define EDITOR_TOOLBAR \
    UI_MENU(10, \
        UI_MENU_POPUP(ICON_MD_SETTINGS, vec2(0.33,1.00), ui_debug()) \
        UI_MENU_ITEM(ICON_PL4Y, editor_send(window_has_pause() ? "play" : "pause")) \
        UI_MENU_ITEM(ICON_SKIP, editor_send(window_has_pause() ? "frame" : "slomo")) \
        UI_MENU_ITEM(ICON_MDI_STOP, editor_send("stop")) \
        UI_MENU_ITEM(ICON_MDI_EJECT, editor_send("eject")) \
        UI_MENU_ITEM(TITLE,) \
        UI_MENU_ALIGN_RIGHT(30+30+30) \
        UI_MENU_ITEM(ICON_MD_FOLDER_SPECIAL, editor_send("browser")) \
        UI_MENU_ITEM(ICON_MD_SEARCH, editor_send("filter")) \
        UI_MENU_ITEM(ICON_MD_CLOSE, editor_send("quit")) \
    );

    EDITOR_TOOLBAR

    if(! editor.active ) return;

    // draw silhouettes
    sprite_flush();
    for each_map_ptr(*editor_selected_map(),void*,o,int,selected) {
        if(*selected && obj_draw[obj_typeid(*o)]) {
            fx_begin();
            obj_draw(*o);
            sprite_flush();
            fx_end();
        }
    }

    // draw ui
    if( ui_panel("Console " ICON_MDI_CONSOLE, 0)) {
            // allocate complete window space
            struct nk_rect bounds = nk_window_get_content_region(ui_ctx);

            enum { CONSOLE_LINE_HEIGHT = 20 };
            static array(char*) lines = 0;
            do_once {
                array_push(lines, stringf("> Editor v%s. Type `%s` for help.", EDITOR_VERSION, ""));
            }
            int max_lines = (bounds.h - UI_ROW_HEIGHT) / (CONSOLE_LINE_HEIGHT * 2);
            if( max_lines >= 1 ) {
                nk_layout_row_static(ui_ctx, bounds.h - UI_ROW_HEIGHT, bounds.w, 1);
                if(nk_group_begin(ui_ctx, "console.group", NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER)) {
                    nk_layout_row_static(ui_ctx, CONSOLE_LINE_HEIGHT, bounds.w, 1);
                    for( int i = array_count(lines); i < max_lines; ++i )
                        array_push_front(lines, 0);
                    for( int i = array_count(lines) - max_lines; i < array_count(lines); ++i ) {
                        if( !lines[i] ) {
                            #if 0 // debug
                            nk_label_wrap(ui_ctx, va("%d.A/%d",i+1,max_lines));
                            nk_label_wrap(ui_ctx, va("%d.B/%d",i+1,max_lines));
                            #else
                            nk_label_wrap(ui_ctx, "");
                            nk_label_wrap(ui_ctx, "");
                            #endif
                        } else {
                            nk_label_wrap(ui_ctx, lines[i]);
                            const char *answer = isdigit(*lines[i]) ? editor_recv( atoi(lines[i]), 0 ) : NULL;
                            nk_label_wrap(ui_ctx, answer ? answer : "");
                        }
                    }
                    nk_group_end(ui_ctx);
                }
            }
            static char *cmd = 0;
            if( ui_string(NULL, &cmd) ) {
                int jobid = editor_send(cmd);
                array_push(lines, stringf("%d> %s", jobid, cmd));
                cmd[0] = 0;
            }

        ui_panel_end();
    }
    if( ui_panel("Scene " ICON_MDI_FILE_TREE, PANEL_OPEN)) {
        editor_tree();
        ui_panel_end();
    }

    // draw ui browser
    if( ui_window("Browser", 0) ) {
        const char *file = 0;
        if( ui_browse(&file, NULL) ) {
            const char *sep = ifdef(win32, "\"", "'");
            app_exec(va("%s %s%s%s", ifdef(win32, "start \"\"", ifdef(osx, "open", "xdg-open")), sep, file, sep));
        }
        ui_window_end();
    }

    // draw ui filter (note: render at end-of-frame, so it's hopefully on-top)
    editor_filter();
}

// ----------------------------------------------------------------------------
// demo

typedef struct my_sprite { OBJ
    char *filename;
    vec3 position;
    float tilt;
    vec4 tint;
    // --- private
    unsigned rgba_;
    texture_t texture_;
} my_sprite;

OBJTYPEDEF(my_sprite,201);

void my_sprite_ctor(my_sprite *obj) {
    obj->texture_ = texture(obj->filename, TEXTURE_RGBA);
    obj->rgba_ = rgbaf( obj->tint.x/255.0, obj->tint.y/255.0, obj->tint.z/255.0, obj->tint.w/255.0 );
}
void my_sprite_draw(my_sprite *obj) {
    obj->rgba_ = rgbaf( obj->tint.x/255.0, obj->tint.y/255.0, obj->tint.z/255.0, obj->tint.w/255.0 ); // @fixme: del me
    sprite( obj->texture_, &(obj->position.x), obj->tilt, obj->rgba_ );
}
void my_sprite_edit(my_sprite *obj) {
    ui_label("Private section");
    ui_color4("Tint_", &obj->tint.x);
    ui_texture("Texture_", obj->texture_);
    ui_separator();
}

AUTORUN {
    // reflect
    STRUCT( my_sprite, char*, filename, "Filename" );
    STRUCT( my_sprite, vec3, position, "Position" );
    STRUCT( my_sprite, float, tilt, "Tilt degrees" );
    STRUCT( my_sprite, vec4, tint, "Tint color" );

    // extend
    EXTEND(my_sprite,ctor);
    EXTEND(my_sprite,draw);
    EXTEND(my_sprite,edit);
}

void game(unsigned frame, float dt, double t) {
    static my_sprite *root;
    static my_sprite *o1;
    static my_sprite *o2;
    static camera_t cam;
    if( !frame ) {
        cam = camera();
        camera_enable(&cam);

        root = obj_make(
            "[my_sprite]\n"
            "filename=cat.png\n"
            "position=5,2,100\n"
            "tilt=46 + 45 -90\n"
            "tint=255, 255, 0, 255\n"
        );
        o1 = obj_make(
            "[my_sprite]\n"
            "filename=cat.png\n"
            "position=1,2,100\n"
            "tilt=45 + 45 -90\n"
            "tint=255, 0, 0, 255\n"
        );
        o2 = obj_make(
            "[my_sprite]\n"
            "filename=cat.png\n"
            "position=1,2,100\n"
            "tilt=45\n"
            "tint=0, 0, 255, 255\n"
        );

        //obj_setname(root, "root");
        obj_setname(o1, "o1");
        obj_setname(o2, "o2");

        obj*o3 = obj_new_ext(obj, "o3");
        obj*o4 = obj_new_ext(obj, "o4");
        obj*o5 = obj_new_ext(obj, "o5");

        obj_attach(root, o1);
            obj_attach(o1, o2);
                obj_attach(o2, o3);
            obj_attach(o1, o4);
        obj_attach(root, o5);

        editor_watch(root);
    }

    // draw game
    my_sprite_draw(root);
    my_sprite_draw(o1);
    my_sprite_draw(o2);

    // tick game
    root->tilt = 5 * sin(t+dt);
}

int main(){
    for( window_create(flag("--transparent") ? 101 : 80,0); window_swap(); ) {
        editor_frame(game);
    }
}
