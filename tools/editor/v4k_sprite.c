#include "engine/v4k.c"

// texture_t texture_createclip(unsigned cx,unsigned cy,unsigned cw,unsigned ch, unsigned tw,unsigned th,unsigned tn,void *pixels, unsigned flags) {
//     return texture_create(tw,th,tn,pixels,flags);
//     static array(unsigned) clip = 0;
//     array_resize(clip, cw*ch*4);
//     for( unsigned y = 0; y < ch; ++y )
//     memcpy((char *)clip + (0+(0+y)*cw)*tn, (char*)pixels + (cx+(cy+y)*tw)*tn, cw*tn);
//     return texture_create(cw,ch,tn,clip,flags);
// }

#define array_reserve_(arr,x) (array_count(arr) > (x) ? (arr) : array_resize(arr, 1+(x)))

#define ui_array(label,type,ptr) do { \
    int changed = 0; \
    if( ui_collapse(label, va(#type "%p",ptr)) ) { \
        char label_ex[8]; \
        for( int idx = 0, iend = array_count(*(ptr)); idx < iend; ++idx ) { \
            type* it = *(ptr) + idx; \
            snprintf(label_ex, sizeof(label_ex), "[%d]", idx); \
            changed |= ui_##type(label_ex, it); \
        } \
        ui_collapse_end(); \
    } \
} while(0)

int ui_vec2i(const char *label, vec2i *v) { return ui_unsigned2(label, (unsigned*)v); }
int ui_vec3i(const char *label, vec3i *v) { return ui_unsigned3(label, (unsigned*)v); }
int ui_vec2(const char *label, vec2 *v) { return ui_float2(label, (float*)v); }
int ui_vec3(const char *label, vec3 *v) { return ui_float3(label, (float*)v); }
int ui_vec4(const char *label, vec4 *v) { return ui_float4(label, (float*)v); }

char *trimspace(char *str) {
    for( char *s = str; *s; ++s )
        if(*s <= 32) memmove(s, s+1, strlen(s));
    return str;
}

char *file_parent(const char *f) {   // folder/folder/abc
    char *p = file_path(f);          // folder/folder/
    char *last = strrchr(p, '/');    //              ^
    if( !last ) return p;            // return parent if no sep
    *last = '\0';                    // folder/folder
    last = strrchr(p, '/');          //       ^
    return last ? last + 1 : p;      // return parent if no sep
}

int ui_obj(const char *fmt, obj *o) {
    int changed = 0, item = 1;
    for each_objmember(o, TYPE,NAME,PTR) {
        char *label = va(fmt, NAME);
        /**/ if(!strcmp(TYPE,"float"))    { if(ui_float(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"int"))      { if(ui_int(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"unsigned")) { if(ui_unsigned(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"vec2"))     { if(ui_float2(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"vec3"))     { if(ui_float3(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"vec4"))     { if(ui_float4(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"rgb"))      { if(ui_color3(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"rgba"))     { if(ui_color4(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"color"))    { if(ui_color4f(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"color3f"))  { if(ui_color3f(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"color4f"))  { if(ui_color4f(label, PTR)) changed = item; }
        else if(!strcmp(TYPE,"char*"))    { if(ui_string(label, PTR)) changed = item; }
        else ui_label2(label, va("(%s)", TYPE)); // INFO instead of (TYPE)?
        ++item;
    }
    return changed;
}


TODO("serialize array(types)")
TODO("serialize map(char*,types)")
TODO("serialize map(int,types)")
TODO("sprite: solid platforms, one-way platforms")
TODO("sprite: shake left-right, up-down")
TODO("sprite: coyote time")
TODO("sprite: jump buffering before grounded")
TODO("sprite: double jump, wall sliding, wall climbing")
TODO("sprite: hitbox for enemies -> wall detection")

#define OBJTYPEDEF2(...) OBJTYPEDEF(__VA_ARGS__); AUTORUN

typedef unsigned quark_t;

typedef struct atlas_frame_t {
    unsigned delay;
    vec4 sheet;
    vec2 anchor; // @todo
    array(vec3i) indices;
    array(vec2) coords;
    array(vec2) uvs;
} atlas_frame_t;

typedef struct atlas_anim_t {
    quark_t name;
    array(unsigned) frames;
} atlas_anim_t;

typedef struct atlas_t {
    texture_t tex;

    array(atlas_frame_t) frames;
    array(atlas_anim_t)  anims;

    quarks_db db;
} atlas_t;

int ui_atlas_frame(atlas_frame_t *f) {
    ui_unsigned("delay", &f->delay);
    ui_vec4("sheet", &f->sheet);
    ui_array("indices", vec3i, &f->indices);
    ui_array("coords", vec2, &f->coords);
    ui_array("uvs", vec2, &f->uvs);
    return 0;
}

int ui_atlas(atlas_t *a) {
    int changed = 0;
    ui_texture(NULL, a->tex);
    for( int i = 0; i < array_count(a->anims); ++i ) {
        if( ui_collapse(quark_string(&a->db, a->anims[i].name), va("%p%d", a, a->anims[i].name) ) ) {
            changed = i+1;
            for( int j = 0; j < array_count(a->anims[i].frames); ++j ) {
                if( ui_collapse(va("[%d]",j), va("%p%d.%d", a, a->anims[i].name,j) ) ) {
                    ui_unsigned("Frame", &a->anims[i].frames[j]);
                    ui_atlas_frame(a->frames + a->anims[i].frames[j]);
                    ui_collapse_end();
                }
            }
            ui_collapse_end();
        }
    }
    return changed;
}

void atlas_destroy(atlas_t *a) {
    if( a ) {
        texture_destroy(&a->tex);
        memset(a, 0, sizeof(atlas_t));
    }
}
atlas_t atlas_create(const char *inifile, unsigned flags) {
    atlas_t a = {0};
    int padding = 0, border = 0;

    ini_t kv = ini(inifile);
    for each_map(kv, char*,k, char*,v ) {
        unsigned index = atoi(k);
        /**/ if( strend(k, ".name") ) {
            array_reserve_(a.anims, index);

            a.anims[index].name = quark_intern(&a.db, v);
        }
        else if( strend(k, ".frames") ) {
            array_reserve_(a.anims, index);

            array(char*) pairs = strsplit(v, ",");
            for( int i = 0, end = array_count(pairs); i < end; i += 2 ) {
                unsigned frame = atoi(pairs[i]);
                unsigned delay = atoi(pairs[i+1]);

                array_reserve_(a.frames, frame);
                a.frames[frame].delay = delay;

                array_push(a.anims[index].frames, frame);
            }
        }
        else if( strend(k, ".sheet") ) {
            array_reserve_(a.frames, index);

            vec4 sheet = atof4(v); //x,y,x2+2,y2+2 -> x,y,w,h (for 2,2 padding)
            a.frames[index].sheet = vec4(sheet.x,sheet.y,sheet.z-sheet.x,sheet.w-sheet.y);
        }
        else if( strend(k, ".indices") ) {
            array_reserve_(a.frames, index);

            const char *text = v;
            array(char*) tuples = strsplit(text, ",");
            for( int i = 0, end = array_count(tuples); i < end; i += 3 ) {
                unsigned p1 = atoi(tuples[i]);
                unsigned p2 = atoi(tuples[i+1]);
                unsigned p3 = atoi(tuples[i+2]);
                array_push(a.frames[index].indices, vec3i(p1,p2,p3));
            }
        }
        else if( strend(k, ".coords") ) {
            array_reserve_(a.frames, index);

            const char *text = v;
            array(char*) pairs = strsplit(text, ",");
            for( int i = 0, end = array_count(pairs); i < end; i += 2 ) {
                unsigned x = atoi(pairs[i]);
                unsigned y = atoi(pairs[i+1]);
                array_push(a.frames[index].coords, vec2(x,y));
            }
        }
        else if( strend(k, ".uvs") ) {
            array_reserve_(a.frames, index);

            const char *text = v;
            array(char*) pairs = strsplit(text, ",");
            for( int i = 0, end = array_count(pairs); i < end; i += 2 ) {
                unsigned u = atoi(pairs[i]);
                unsigned v = atoi(pairs[i+1]);
                array_push(a.frames[index].uvs, vec2(u,v));
            }
        }
        else if( strend(k, "padding") ) {
            padding = atoi(v);
        }
        else if( strend(k, "border") ) {
            border = atoi(v);
        }
        else if( strend(k, "file") ) {
            a.tex = texture(v, 0);
        }
        else if( strend(k, "bitmap") ) {
            const char *text = v;
            array(char) bin = base64_decode(text, strlen(text));
            a.tex = texture_from_mem(bin, array_count(bin), 0);
            array_free(bin);
        }
#if 0
        else if( strend(k, ".frame") ) {
            array_reserve_(a.frames, index);
            puts(k), puts(v);
        }
#endif
    }

    // post-process: normalize uvs and coords into [0..1] ranges
    for each_array_ptr(a.frames, atlas_frame_t, f) {
        for each_array_ptr(f->uvs, vec2, uv) {
            uv->x /= a.tex.w;
            uv->y /= a.tex.h;
        }
        for each_array_ptr(f->coords, vec2, xy) {
            xy->x /= a.tex.w;
            xy->y /= a.tex.h;
        }
        // @todo: adjust padding/border
    }
#if 0
    // post-process: specify an anchor for each anim based on 1st frame dims
    for each_array_ptr(a.anims, atlas_anim_t, anim) {
        atlas_frame_t *first = a.frames + *anim->frames;
        for( int i = 0; i < array_count(anim->frames); i += 2) {
            atlas_frame_t *ff = a.frames + anim->frames[ i ];
            ff->anchor.x = (ff->sheet.z - first->sheet.z) / 2;
            ff->anchor.y = (ff->sheet.w - first->sheet.w) / 2;
        }
    }
#endif

    return a;
}

typedef struct sprite2 { OBJ
    vec4 gamepad; // up,down,left,right
    vec2 fire;    // a,b

    vec3 pos;
    float tilt;
    unsigned tint;
    unsigned frame;
    unsigned timer, timer_ms;
    unsigned flip_, flipped;
    unsigned play;
    bool paused;
    // array(unsigned) play_queue; or unsigned play_next;
    atlas_t *a; // shared
    atlas_t own; // owned
} sprite2;

void sprite2_setanim(sprite2 *s, unsigned name) {
    if( s->play != name ) {
        s->play = name;
        s->frame = 0;

        atlas_frame_t *f = &s->a->frames[ s->a->anims[s->play].frames[s->frame] ];

        s->timer_ms = f->delay;
        s->timer = s->timer_ms;
    }
}

void sprite2_ctor(sprite2 *s) {
    s->tint = WHITE;
    s->timer_ms = 100;
    s->flipped = 1;
}
void sprite2_dtor(sprite2 *s) {
    memset(s, 0, sizeof(*s));
}
void sprite2_tick(sprite2 *s) {
    int move = input(s->gamepad.array[3]) - input(s->gamepad.array[2]); // RIGHT - LEFT
    int dt = 16; // window_delta() * 1000;

    unsigned over = (s->timer - dt) > s->timer;
    if(!s->paused)
    s->timer -= dt;
    if( over ) {

        int len = array_count(s->a->anims[s->play].frames);
        unsigned next = (s->frame + 1) % (len + !len);
        unsigned eoa = next < s->frame;
        s->frame = next;

        atlas_frame_t *f = &s->a->frames[ s->a->anims[s->play].frames[s->frame] ];
        s->timer_ms = f->delay;
        s->timer += s->timer_ms;
    }

    if( s->play == 0 && move ) sprite2_setanim(s, 1);
    if( s->play == 1 ) { //<
        float speed = 1.0f;
        if(move) s->pos.x += speed * move, s->flip_ = move < 0, sprite2_setanim(s, 1);
        else sprite2_setanim(s, 0);
    }
}
void sprite2_draw(sprite2 *s) {
    atlas_frame_t *f = &s->a->frames[ s->a->anims[s->play].frames[s->frame] ];

#if 1
    // @todo {
        unsigned sample = s->a->anims[s->play].frames[s->frame];
        sample = 0;
        f->anchor.x = (-s->a->frames[sample].sheet.z + f->sheet.z) / 2;
        f->anchor.y = (+s->a->frames[sample].sheet.w - f->sheet.w) / 2;
    // }
#endif

    // rect(x,y,w,h) is [0..1] normalized, z-index, pos(x,y,scale), rotation (degrees), color (rgba)
    vec4 rect = { f->sheet.x / s->a->tex.w, f->sheet.y / s->a->tex.h, f->sheet.z / s->a->tex.w, f->sheet.w / s->a->tex.h };
    sprite_rect(s->a->tex, rect, s->pos.y, vec4(s->pos.x+f->anchor.x,s->pos.y+f->anchor.y,s->flip_ ^ s->flipped?1:-1,1), s->tilt, s->tint);
}
void sprite2_edit(sprite2 *s) {
    const char *id = vac("%p", s);
    if( s && ui_collapse(id, id) ) {
        ui_obj("%s", (obj*)s);

        ui_bool("paused", &s->paused);
        ui_label(va("frame anim [%d]", s->a->anims[s->play].frames[s->frame]));

        int k = s->play;
        if( ui_int("anim", &k) ) {
            sprite2_setanim(s, k);
        }

        int selected = ui_atlas(s->a);
        if( selected ) sprite2_setanim(s, selected - 1);

        ui_collapse_end();
    }
}

OBJTYPEDEF(sprite2,10);
AUTORUN {
    STRUCT(sprite2, vec4, gamepad);
    STRUCT(sprite2, vec2, fire);
    STRUCT(sprite2, vec3, pos);
    STRUCT(sprite2, float, tilt);
    STRUCT(sprite2, rgba,  tint);
    STRUCT(sprite2, unsigned, frame);
    STRUCT(sprite2, unsigned, timer);
    STRUCT(sprite2, unsigned, timer_ms);
    STRUCT(sprite2, unsigned, flipped);
    STRUCT(sprite2, unsigned, play);
    EXTEND(sprite2, ctor,edit,draw,tick);
}

sprite2* sprite2_new(const char *ase, int bindings[6]) {
    sprite2 *s = obj_new(sprite2, {bindings[0],bindings[1],bindings[2],bindings[3]}, {bindings[4],bindings[5]});
    s->own = atlas_create(ase, 0);
    s->a = &s->own;
    return s;
}
void sprite2_del(sprite2 *s) {
    if( s ) {
        if( s->a == &s->own ) atlas_destroy(&s->own);
        obj_free(s);
        memset(s, 0, sizeof(sprite2));
    }
}



void game(unsigned frame) {
    static camera_t cam;
    static sprite2 *s1 = 0;
    static sprite2 *s2 = 0;
    static sprite2 *s3 = 0;
    static sprite2 *s4 = 0;
    if( !frame ) {
        // camera center(x,y) zoom(z)
        cam = camera();
        cam.position = vec3(window_width()/2,window_height()/2,8);
        camera_enable(&cam);

        sprite2_del(s1);
        sprite2_del(s2);
        sprite2_del(s3);
        sprite2_del(s4);

        s1 = sprite2_new("Captain Clown Nose.ase", (int[6]){KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_A,KEY_S});
        s2 = sprite2_new("Crew-Crabby.ase", (int[6]){KEY_I,KEY_K,KEY_J,KEY_L} );
        s3 = sprite2_new("Props-Shooter Traps.ase", (int[6]){0} );
        s4 = sprite2_new("Crew-Fierce Tooth.ase", (int[6]){0,0,KEY_N,KEY_M} );

        // pos and z-order
        s1->pos = vec3(window_width()/2, window_height()/2, 2);
        s2->pos = vec3(window_width()/2, window_height()/2, 1);
        s3->pos = vec3(window_width()/2, window_height()/2, 1);
        s4->pos = vec3(window_width()/2, window_height()/2, 1);

        s4->flipped ^= 1;
    }

    // camera panning (x,y) & zooming (z)
    if( !ui_hover() && !ui_active() ) {
        if( input(MOUSE_L) ) cam.position.x -= input_diff(MOUSE_X);
        if( input(MOUSE_L) ) cam.position.y -= input_diff(MOUSE_Y);
        cam.position.z += input_diff(MOUSE_W) * 0.1; // cam.p.z += 0.001f; for tests
    }

    obj_tick(s1);
    obj_draw(s1);

    obj_tick(s2);
    obj_draw(s2);

    obj_tick(s3);
    obj_draw(s3);

    obj_tick(s4);
    obj_draw(s4);

    if( ui_panel("Sprites", PANEL_OPEN)) {
        obj_edit(s1);
        obj_edit(s2);
        obj_edit(s3);
        obj_edit(s4);
        ui_panel_end();
    }
}

int main() {
    unsigned frame = 0;

    for( window_create(0.75, 0); window_swap(); ) {
        if( input_down(KEY_Z) && input(KEY_ALT) ) window_record(file_counter(va("%s.mp4",app_name())));
        if( input_down(KEY_F5) ) frame = 0;
        game( frame++ );
    }
}