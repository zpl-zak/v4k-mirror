// ----------------------------------------------------------------------------
// game ui

typedef struct guiskin_t {
    void (*drawrect)(void* userdata, const char *skin, vec4 rect);
    void (*getskinsize)(void* userdata, const char *skin, vec2 *size);
    void (*getscissorrect)(void* userdata, const char *skin, vec4 rect, vec4 *dims);
    bool (*ismouseinrect)(void* userdata, const char *skin, vec4 rect);
    void (*free)(void* userdata);
    void *userdata;
} guiskin_t;

API void    gui_pushskin(guiskin_t skin);
API void*       gui_userdata();
API vec2        gui_getskinsize(const char *skin);
API bool        gui_ismouseinrect(const char *skin, vec4 rect);
// --
API void        gui_panel_id(int id, vec4 rect, const char *skin);
API void            gui_rect_id(int id, vec4 rect, const char *skin);
API bool            gui_button_id(int id, vec4 rect, const char *skin);
API bool            gui_slider_id(int id, vec4 rect, const char *skin, float min, float max, float step, float *value);
API void        gui_panel_end();
API void    gui_popskin();

// helpers
#define gui_panel(...) gui_panel_id(__LINE__, __VA_ARGS__)
#define gui_rect(...) gui_rect_id(__LINE__, __VA_ARGS__)
#define gui_button(...) gui_button_id(__LINE__, __VA_ARGS__)
#define gui_slider(...) gui_slider_id(__LINE__, __VA_ARGS__)

// default renderers

/// skinned
typedef struct skinned_t {
    atlas_t atlas;
    float scale;
} skinned_t;

API guiskin_t gui_skinned(const char *inifile, float scale);
