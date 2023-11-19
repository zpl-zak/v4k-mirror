// -----------------------------------------------------------------------------
// in-game editor
// - rlyeh, public domain.

API int editor_send(const char *command);

//API void  editor();
//API bool  editor_active();
API vec3   editor_pick(float mouse_x, float mouse_y);
API char*  editor_path(const char *path);

API float* engine_getf(const char *key);
API int*   engine_geti(const char *key);
API char** engine_gets(const char *key);
API int    engine_send(const char *cmd, const char *optional_value);

API int    ui_debug();

// open file dialog

API char* dialog_load();
API char* dialog_save();

// transform gizmos

API int   gizmo(vec3 *pos, vec3 *rot, vec3 *sca);
API bool  gizmo_active();
API bool  gizmo_hover();
