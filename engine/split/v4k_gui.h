// ----------------------------------------------------------------------------
// game ui

enum {
    GUI_PANEL,
    GUI_BUTTON,
};

typedef struct gui_state_t {
    int kind;

    union {
        struct {
            bool held;
            bool hover;
        };
    };
} gui_state_t;

typedef struct guiskin_t {
    void (*draw_rect_func)(void* userdata, gui_state_t state, const char *skin, vec4 rect);
    void (*free)(void* userdata);
    void *userdata;
} guiskin_t;

API void    gui_pushskin(guiskin_t skin);
API void*       gui_userdata();
// --
API void        gui_panel(int id, vec4 rect, const char *skin);
API bool        gui_button(int id, vec4 rect, const char *skin);
API void    gui_popskin();

// helpers
#define gui_panel(...) gui_panel(__LINE__, __VA_ARGS__)
#define gui_button(...) gui_button(__LINE__, __VA_ARGS__)

// default skins
API guiskin_t gui_skinned(const char *inifile, float scale);

typedef struct skinned_t {
    atlas_t atlas;
    float scale;
    // unsigned framenum;

    //skins
    char *panel;
    char *button;
} skinned_t;

