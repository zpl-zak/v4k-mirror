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
TODO("edit:   overlay1 world editor: gizmo, grid, snap, splats (@todo: fixed screen size gizmos)");
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

void editor_gizmos(int dim) {
    // debugdraw
    if(dim == 2) ddraw_push_2d();
    ddraw_ontop_push(0);

    // draw gizmos, aabbs, markers, etc
    for each_map_ptr(*editor_selected_map(),void*,o,int,selected) {
        if( !*selected ) continue;

        void *obj = *o;

        // get transform
        vec3 *p = NULL;
        vec3 *r = NULL;
        vec3 *s = NULL;
        aabb *a = NULL;

        for each_objmember(obj,TYPE,NAME,PTR) {
            /**/ if( !strcmp(NAME, "position") ) p = PTR;
            else if( !strcmp(NAME, "pos") ) p = PTR;
            else if( !strcmp(NAME, "rotation") ) r = PTR;
            else if( !strcmp(NAME, "rot") ) r = PTR;
            else if( !strcmp(NAME, "scale") ) s = PTR;
            else if( !strcmp(NAME, "sca") ) s = PTR;
            else if( !strcmp(NAME, "aabb") ) a = PTR;
        }

        ddraw_ontop(0);

        // bounding box 3d
        if( 0 ) {
            aabb box;
            if( obj_hasmethod(*o, aabb) && obj_aabb(*o, &box) ) {
                ddraw_color_push(YELLOW);
                ddraw_aabb(box.min, box.max);
                ddraw_color_pop();
            }
        }

        // position marker
        if( p ) {
            static map(void*, vec3) prev_dir = 0;
            do_once map_init_ptr(prev_dir);
            vec3* dir = map_find_or_add(prev_dir, obj, vec3(1,0,0));

            static map(void*, vec3) prev_pos = 0;
            do_once map_init_ptr(prev_pos);
            vec3* found = map_find_or_add(prev_pos, obj, *p), fwd = sub3(*p, *found);
            if( (fwd.y = 0, len3sq(fwd)) ) {
                *found = *p;
                *dir = norm3(fwd);
            }

            // float diameter = len2( sub2(vec2(box->max.x,box->max.z), vec2(box->min.x,box->min.z) ));
            // float radius = diameter * 0.5;
            ddraw_position_dir(*p, *dir, 1);
        }

        ddraw_ontop(1);

        // transform gizmo
        if( p && r && s ) {
            gizmo(p,r,s);
        }
    }

    ddraw_ontop_pop();
    if(dim == 2) ddraw_pop_2d();
}

// ----------------------------------------------------------------------------
// demo

typedef struct lit { OBJ
    vec3 pos;
    vec3 dir;
    int type;
} lit;

int lit_aabb(lit *obj, aabb *box) {
    *box = aabb( vec3(obj->pos.x-16,obj->pos.y-16,0), vec3(obj->pos.x+16,obj->pos.y+16,1) );
    return 1;
}
const char *lit_icon(lit *obj) {
    const char *icon =
        obj->type == 0 ? ICON_MD_WB_IRIDESCENT :
        obj->type == 1 ? ICON_MD_WB_INCANDESCENT :
        obj->type == 2 ? ICON_MD_FLARE :
        obj->type == 3 ? ICON_MD_WB_SUNNY : "";
    return icon;
}
int lit_edit(lit *obj) {
    const char *all_icons =
        ICON_MD_WB_IRIDESCENT
        ICON_MD_WB_INCANDESCENT
        ICON_MD_FLARE
        ICON_MD_WB_SUNNY

        ICON_MD_LIGHT_MODE
        ICON_MD_LIGHT

        ICON_MD_FLASHLIGHT_OFF
        ICON_MD_FLASHLIGHT_ON
        ICON_MD_HIGHLIGHT
        ICON_MD_HIGHLIGHT_ALT
        ICON_MD_LIGHTBULB
        ICON_MD_LIGHTBULB_OUTLINE
        ICON_MD_NIGHTLIGHT
        ICON_MD_NIGHTLIGHT_ROUND

        // MDI
        ICON_MDI_LIGHTBULB_ON_OUTLINE        // generic
        ICON_MDI_WALL_SCONCE_ROUND           //
        ICON_MDI_WALL_SCONCE_FLAT            // emissive
        ICON_MDI_CEILING_LIGHT               // spotlight
        ICON_MDI_TRACK_LIGHT                 // spotlight
        ICON_MDI_WEATHER_SUNNY               // directional
        ICON_MDI_LIGHTBULB_FLUORESCENT_TUBE_OUTLINE
    ;
    // editor_symbol(obj->pos.x+16,obj->pos.y-32,all_icons);
    if( editor_selected(obj) ) {
    obj->pos.x += input(KEY_RIGHT) - input(KEY_LEFT);
    obj->pos.y += input(KEY_DOWN) - input(KEY_UP);
    obj->type = (obj->type + !!input_down(KEY_SPACE)) % 4;
    }
    editor_symbol(obj->pos.x,obj->pos.y,lit_icon(obj));



    return 1;
}

OBJTYPEDEF(lit,200);

AUTORUN {
    STRUCT(lit, vec3, pos);
    STRUCT(lit, vec3, dir);
    STRUCT(lit, int, type);
    EXTEND(lit, edit,icon,aabb);
}

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
int kid_edit(kid *obj) {
    aabb box;
    if( kid_aabb(obj, &box) ) {
        ddraw_color_push(YELLOW);
        ddraw_push_2d();
        ddraw_aabb(box.min, box.max);
        ddraw_pop_2d();
        ddraw_color_pop();
    }
    if( editor_selected(obj) ) {
        obj->pos.x += input(KEY_RIGHT) - input(KEY_LEFT);
        obj->pos.y += input(KEY_DOWN) - input(KEY_UP);

        editor_symbol(obj->pos.x+16,obj->pos.y-16,ICON_MD_VIDEOGAME_ASSET);
    }
    return 1;
}
void kid_menu(kid *obj, const char *argv) {
    ui_label("Private section");
    ui_color4("Color_", &obj->color.x);
    ui_texture("Texture_", obj->texture_);
    ui_separator();

    obj->rgba_ = rgbaf( obj->color.x/255.0, obj->color.y/255.0, obj->color.z/255.0, obj->color.w/255.0 );
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
    EXTEND(kid, ctor,tick,draw,aabb,edit,menu);
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

        obj*o3 = obj_make(
            "[lit]\n"
            "pos=300,300,0\n"
            "type=1"
        );
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

    // fps camera
    bool active = 0;
    if( input_down(MOUSE_M) || input_down(MOUSE_R) ) {
        active = ui_hover() || ui_active() || gizmo_active() || editor_first_selected() ? false : true;
    } else {
        active = !window_has_cursor() && (input(MOUSE_M) || input(MOUSE_R));
    }
    window_cursor( !active );
    if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
    vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
    vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
    camera_moveby(&cam, wasdecq);
    camera_fps(&cam, mouse.x,mouse.y);

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
    window_icon("scale-ruler-icon.png");

    while( window_swap() ) {
        editor_frame(game);
        editor_gizmos(2);
    }
}
