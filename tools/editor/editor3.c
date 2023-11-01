#define COOK_ON_DEMAND 1 // @fixme: these directives should be on client, not within v4k.dll
#define ENABLE_AUTOTESTS 1
#define V4K_IMPLEMENTATION
#include "v4k.h"
#include "objtests.h"

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

#include "3rd_icon_mdi.h"
#include "v4k_editor.h"

// ----------------------------------------------------------------------------
// demo

typedef struct kid { OBJ
    int kid;
    vec2 pos;
    vec2 vel;
    float angle;
    vec4  color;
    int controllerid;

    // --- private
    char *filename;
    unsigned rgba_;
    texture_t texture_;
} kid;

void kid_ctor(kid *obj) {
    obj->kid = randi(0,3);
    obj->pos = vec2(randi(0, window_width()), randi(0, window_height()));
    obj->vel.x = obj->vel.y = 100 + 200 * randf();
    obj->controllerid = randi(0,3);

    obj->texture_ = texture(obj->filename, TEXTURE_RGBA|TEXTURE_LINEAR);
    obj->rgba_ = rgbaf( obj->color.x/255.0, obj->color.y/255.0, obj->color.z/255.0, obj->color.w/255.0 );
}
void kid_tick(kid *obj, float dt) {
    // add velocity to position
    vec2 off = vec2( input(KEY_RIGHT)-input(KEY_LEFT), input(KEY_DOWN)-input(KEY_UP) );
    obj->pos = add2(obj->pos, scale2(mul2(obj->vel, off), dt * (obj->controllerid == 0)));

    // wrap at screen boundaries
    const int appw = window_width(), apph = window_height();
    if( obj->pos.x < 0 ) obj->pos.x += appw; else if( obj->pos.x > appw ) obj->pos.x -= appw;
    if( obj->pos.y < 0 ) obj->pos.y += apph; else if( obj->pos.y > apph ) obj->pos.y -= apph;
}
void kid_draw(kid *obj) {
    // 4x4 tilesheet
    int col = (((int)obj->kid) % 4);
    int row = (((int)obj->pos.x / 10 ^ (int)obj->pos.y / 10) % 4);
    float position[3] = {obj->pos.x,obj->pos.y,obj->pos.y}; // position(x,y,depth: sort by Y)
    float offset[2]={0,0}, scale[2]={1,1};
    float coords[3]={col * 4 + row,4,4}; // num_frame(x) in a 4x4(y,z) spritesheet
    sprite_sheet(obj->texture_, coords, position, obj->angle*TO_DEG, offset, scale,
        0, obj->rgba_, 0); // is_additive, tint color, resolution independant
}
int kid_aabb(kid *obj, aabb *box) {
    *box = aabb( vec3(obj->pos.x-16,obj->pos.y-16,0), vec3(obj->pos.x+16,obj->pos.y+16,1) );
    return 1;
}
void kid_menu(kid *obj, const char *argv) {
    ui_label("Private section");
    ui_color4("Color_", &obj->color.x);
    ui_texture("Texture_", obj->texture_);
    ui_separator();

    obj->rgba_ = rgbaf( obj->color.x/255.0, obj->color.y/255.0, obj->color.z/255.0, obj->color.w/255.0 );
}
const char* kid_icon(kid *obj) {
    if(obj->controllerid == 0) {
        editor_symbol(obj->pos.x+16,obj->pos.y-16,ICON_MD_VIDEOGAME_ASSET);
        return ICON_MD_VIDEOGAME_ASSET;
    }
    return NULL;
}

OBJTYPEDEF(kid,201);

AUTORUN {
    // reflect
    STRUCT(kid, int, kid);
    STRUCT(kid, vec2, pos);
    STRUCT(kid, vec2, vel);
    STRUCT(kid, float, angle, "Tilt degrees");
    STRUCT(kid, vec4, color, "Tint color");
    STRUCT(kid, char*, filename, "Filename" );
    EXTEND(kid, ctor,tick,draw,aabb,menu,icon);
}

void game(unsigned frame, float dt, double t) {
    static kid *root;
    static kid *o1;
    static kid *o2;
    static camera_t cam;
    if( !frame ) {
        // init camera (x,y) (z = zoom)
        cam = camera();
        cam.position = vec3(window_width()/2,window_height()/2,1);
        camera_enable(&cam);

        root = obj_make(
            "[kid]\n"
            "filename=spriteSheetExample.png\n"
            "pos=5,2\n"
            "angle=pi/12\n"
            "color=255, 255, 192, 255\n"
        );
        o1 = obj_make(
            "[kid]\n"
            "filename=spriteSheetExample.png\n"
            "pos=1,100\n"
            "angle=pi/12\n"
            "color=255, 192, 192, 255\n"
        );
        o2 = obj_make(
            "[kid]\n"
            "filename=spriteSheetExample.png\n"
            "pos=50,200\n"
            "angle=pi/12\n"
            "color=192, 192, 255, 255\n"
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

    // camera panning (x,y) & zooming (z)
    if(0)
    if( !ui_hover() && !ui_active() ) {
        if( input(MOUSE_L) ) cam.position.x -= input_diff(MOUSE_X);
        if( input(MOUSE_L) ) cam.position.y -= input_diff(MOUSE_Y);
        cam.position.z += input_diff(MOUSE_W) * 0.1; // cam.p.z += 0.001f; for tests
    }

    // tick game
    if( dt ) {
        kid_tick(root, dt);
        kid_tick(o1, dt);
        kid_tick(o2, dt);

        root->angle = 5 * sin(t+dt);
    }

    // draw world
    ddraw_ontop_push(0);
    ddraw_grid(0);
    ddraw_ontop_pop();
    ddraw_flush();

    // draw game
    kid_draw(root);
    kid_draw(o1);
    kid_draw(o2);
}

int main(){
    window_title("Editor " EDITOR_VERSION);
    window_create(flag("--transparent") ? 101 : 80,0);
    window_icon("logo.png");

    while( window_swap() ) {
        editor_frame(game);
    }
}
