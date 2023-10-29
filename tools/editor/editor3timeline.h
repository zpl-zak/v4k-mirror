int ui_window_nk(const char *title, void *open) {
    int ww = window_width();  int w = ww * 0.66;
    int hh = window_height(); int h = hh * 0.66;

    nk_flags flags = NK_WINDOW_TITLE | NK_WINDOW_BORDER |
    NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
    NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MAXIMIZABLE |
    NK_WINDOW_PINNABLE |
    0; // NK_WINDOW_SCROLL_AUTO_HIDE;
    if (nk_begin(ui_ctx, title, nk_rect( (ww-w)/2,(hh-h)/2, w,h), flags))
        return 1;

    nk_end(ui_ctx);
    return 0;
}
int ui_window_nk_end() {
    nk_end(ui_ctx);
    return 0;
}

#define AS_NKCOLOR(color) \
    ((struct nk_color){ ((color>>16))&255,((color>>8))&255,((color>>0))&255,((color>>24))&255 })


/*
typedef struct tween_keyframe_t {
    int easing_mode;
    float t;
    vec3 v;
} tween_keyframe_t;

typedef struct tween_t {
    array(tween_keyframe_t) keyframes;

    vec3 result;
    float time;
    float duration;
} tween_t;

API tween_t tween();
API float     tween_update(tween_t *tw, float dt);
API void      tween_reset(tween_t *tw);
API void    tween_destroy(tween_t *tw);

API void tween_keyframe_set(tween_t *tw, float t, int easing_mode, vec3 v);
API void tween_keyframe_unset(tween_t *tw, float t);
*/

int ui_tween(const char *label, tween_t *t) {
    if( ui_filter && ui_filter[0] ) if( !strstr(label, ui_filter) ) return 0;

    int expand_keys = label[0] == '!'; label += expand_keys;
    const char *id = label;
    if( strchr(id, '@') ) *strchr(((char*)id = va("%s", label)), '@') = '\0';

    enum { LABEL_SPACING = 250 };
    enum { ROUNDING = 0 };
    enum { THICKNESS = 1 };
    enum { PIXELS_PER_SECOND = 60 };
    enum { KEY_WIDTH = 5, KEY_HEIGHT = 5 };
    enum { TIMELINE_HEIGHT = 25 };
    enum { MARKER1_HEIGHT = 5, MARKER10_HEIGHT = 20, MARKER5_HEIGHT = (MARKER1_HEIGHT + MARKER10_HEIGHT) / 2 };
    unsigned base_color = WHITE;
    unsigned time_color = YELLOW;
    unsigned duration_color = ORANGE;
    unsigned key_color = GREEN;

    int changed = 0;

#if 0
    // two rows with height:30 composed of three widgets
    nk_layout_row_template_begin(ui_ctx, 30);
    nk_layout_row_template_push_variable(ui_ctx, t->duration * PIXELS_PER_SECOND); // min 80px. can grow
    nk_layout_row_template_end(ui_ctx);
#endif

        char *sid = va("%s.%d", id, 0);
        uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
        for(int i = 0; sid[i]; ++i) hash = (hash ^ sid[i]) * mult;
        ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

    ui_label(label);

    struct nk_command_buffer *canvas = nk_window_get_canvas(ui_ctx);
    struct nk_rect bounds; nk_layout_peek(&bounds, ui_ctx);
    bounds.y -= 30;

    struct nk_rect baseline = bounds; baseline.y += 30/2;
    baseline.x += LABEL_SPACING;
    baseline.w -= LABEL_SPACING;

    // tween duration
    {
        struct nk_rect pos = baseline;
        pos.w  = pos.x + t->duration * PIXELS_PER_SECOND;
        pos.y -= TIMELINE_HEIGHT/2;
        pos.h  = TIMELINE_HEIGHT;
        nk_stroke_rect(canvas, pos, ROUNDING, THICKNESS*2, AS_NKCOLOR(duration_color));
    }

    // tween ranges
    for(int i = 0, end = array_count(t->keyframes) - 1; i < end; ++i) {
        tween_keyframe_t *k = t->keyframes + i;
        tween_keyframe_t *next = k + 1;

        struct nk_rect pos = baseline;
        pos.x += k->t * PIXELS_PER_SECOND;
        pos.w  = (next->t - k->t) * PIXELS_PER_SECOND;
        pos.y -= TIMELINE_HEIGHT/2;
        pos.h  = TIMELINE_HEIGHT;

        char *sid = va("%s.%d", id, i);
        uint64_t hash = 14695981039346656037ULL, mult = 0x100000001b3ULL;
        for(int i = 0; sid[i]; ++i) hash = (hash ^ sid[i]) * mult;
        ui_hue = (hash & 0x3F) / (float)0x3F; ui_hue += !ui_hue;

        struct nk_color c = nk_hsva_f(ui_hue, 0.75f, 0.8f, ui_alpha);
        nk_fill_rect(canvas, pos, ROUNDING, c); // AS_NKCOLOR(track_color));
    }

    // horizontal line
    nk_stroke_line(canvas, baseline.x, baseline.y, baseline.x+baseline.w,baseline.y, THICKNESS, AS_NKCOLOR(base_color));

    // unit, 5-unit and 10-unit markers
    for( int i = 0, j = 0; i < baseline.w; i += PIXELS_PER_SECOND/10, ++j ) {
        int len = !(j%10) ? MARKER10_HEIGHT : !(j%5) ? MARKER5_HEIGHT : MARKER1_HEIGHT;
        nk_stroke_line(canvas, baseline.x+i, baseline.y-len, baseline.x+i, baseline.y+len, THICKNESS, AS_NKCOLOR(base_color));
    }

    // time marker
    float px = t->time * PIXELS_PER_SECOND;
    nk_stroke_line(canvas, baseline.x+px, bounds.y, baseline.x+px, bounds.y+bounds.h, THICKNESS*2, AS_NKCOLOR(time_color));
    nk_draw_symbol(canvas, NK_SYMBOL_TRIANGLE_DOWN, ((struct nk_rect){ baseline.x+px-4,bounds.y-4-8,8,8}), /*bg*/AS_NKCOLOR(0), /*fg*/AS_NKCOLOR(time_color), 0.f/*border_width*/, ui_ctx->style.font);

    // key markers
    for each_array_ptr(t->keyframes, tween_keyframe_t, k) {
        struct nk_rect pos = baseline;
        pos.x += k->t * PIXELS_PER_SECOND;

        vec2 romboid[] = {
            {pos.x-KEY_WIDTH,pos.y}, {pos.x,pos.y-KEY_HEIGHT},
            {pos.x+KEY_WIDTH,pos.y}, {pos.x,pos.y+KEY_HEIGHT}
        };

        nk_fill_polygon(canvas, (float*)romboid, countof(romboid), AS_NKCOLOR(key_color));
    }

    // keys ui
    static int num_eases = 0; if(!num_eases) while(num_eases[ease_enums()]) ++num_eases;
    if( expand_keys )
    for(int i = 0, end = array_count(t->keyframes); i < end; ++i) {
        tween_keyframe_t *k = t->keyframes + i;
        if( ui_collapse(va("Key %d", i), va("%s.%d", id, i))) {
            changed |= ui_float("time", &k->t);
            changed |= ui_float3("value", &k->v.x);
            changed |= ui_list("easing", ease_enums(), num_eases, &k->easing_mode );
            ui_collapse_end();
        }
    }

    return changed;
}

tween_t* rand_tween() {
    tween_t demo = tween();
    int num_keys = randi(2,8);
    double t = 0;
    for( int i = 0; i < num_keys; ++i) {
        tween_keyframe_set(&demo, t, randi(0,33), scale3(vec3(randf(),randf(),randf()),randi(-5,5)) );
        t += randi(1,5) / ((float)(1 << randi(0,2)));
    }
    tween_t *p = CALLOC(1, sizeof(tween_t));
    memcpy(p, &demo, sizeof(tween_t));
    return p;
}

static array(tween_t*) tweens;

int editor_timeline() {
    do_once {
        array_push(tweens, rand_tween());
    }

    if( editor.t == 0 )
    for each_array(tweens, tween_t*,t) {
        tween_reset(t);
    }
    else
    for each_array(tweens, tween_t*,t) {
        tween_update(t, editor.dt);
    }

    static void *selected = NULL;
    static int open = 1;
    if( ui_window/*_nk*/("Timeline " ICON_MDI_CHART_TIMELINE, &open) ) {

        int choice = ui_toolbar(ICON_MDI_PLUS ";" ICON_MDI_MINUS );
        if( choice == 1 ) array_push(tweens, rand_tween());
        if( choice == 2 && selected ) {
            int target = -1;
            for( int i = 0, end = array_count(tweens); i < end; ++i ) if( tweens[i] == selected ) { target = i; break; }
            if( target >= 0 ) { array_erase_slow(tweens, target); selected = NULL; }
        }

        for each_array(tweens, tween_t*,t) {
            ui_tween(va("%s%p@%05.2fs Value: %s", t == selected ? "!":"", t, t->time, ftoa3(t->result)), t);
            if(ui_label_icon_clicked_L.x) selected = (t != selected) ? t : NULL;
        }

        ui_window_end/*_nk_end*/();
    }
    return 0;
}

AUTORUN {
    array_push(editor.subeditors, editor_timeline);
}