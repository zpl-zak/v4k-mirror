#define EDITOR_VERSION "2023.9"

#if 0
#define EDITOR_PRINTF PRINTF

#ifdef ICON_MDI_CUBE
#define ICON_OBJECT       ICON_MDI_CUBE_OUTLINE
#define ICON_OBJECT_ALT   ICON_MDI_CUBE
#define ICON_DOT          ICON_MDI_CIRCLE_SMALL
#define ICON_EVENT        ICON_MDI_CALENDAR
#else
#define ICON_OBJECT       ICON_MD_VIEW_IN_AR
#define ICON_DOT          "  ·   " // ICON_CANCEL // ICON_MD_WIFI_1_BAR // ICON_MD_RADIO_BUTTON_UNCHECKED // ICON_MD_LENS_BLUR
#define ICON_EVENT        ICON_MD_FLAG
#endif

//#define ICON_CIRCLE     ICON_MDI_CIRCLE_OUTLINE
//#define ICON_CIRCLE_ALT ICON_MDI_CIRCLE

#define ICON_PLAY         ICON_MD_PLAY_ARROW
#define ICON_PAUSE        ICON_MD_PAUSE
#define ICON_STOP         ICON_MD_STOP
#define ICON_CANCEL       ICON_MD_CLOSE

#define ICON_WARNING      ICON_MD_WARNING
#define ICON_BROWSER      ICON_MD_FOLDER_SPECIAL
#define ICON_OUTLINER     ICON_MD_VIEW_IN_AR
#define ICON_BUILD        ICON_MD_BUILD
#define ICON_SCREENSHOT   ICON_MD_PHOTO_CAMERA
#define ICON_CAMERA_ON    ICON_MD_VIDEOCAM
#define ICON_CAMERA_OFF   ICON_MD_VIDEOCAM_OFF
#define ICON_GAMEPAD_ON   ICON_MD_VIDEOGAME_ASSET
#define ICON_GAMEPAD_OFF  ICON_MD_VIDEOGAME_ASSET_OFF
#define ICON_AUDIO_ON     ICON_MD_VOLUME_UP
#define ICON_AUDIO_OFF    ICON_MD_VOLUME_OFF
#define ICON_WINDOWED     ICON_MD_FULLSCREEN_EXIT
#define ICON_FULLSCREEN   ICON_MD_FULLSCREEN
#define ICON_LIGHTS_ON    ICON_MD_LIGHTBULB
#define ICON_LIGHTS_OFF   ICON_MD_LIGHTBULB_OUTLINE
#define ICON_RENDER_BASIC ICON_MD_IMAGE_SEARCH
#define ICON_RENDER_FULL  ICON_MD_INSERT_PHOTO

#define ICON_SIGNAL       ICON_MD_SIGNAL_CELLULAR_ALT
#define ICON_DISK         ICON_MD_STORAGE
#define ICON_RATE         ICON_MD_SPEED

#define ICON_CLOCK        ICON_MD_TODAY
#define ICON_CHRONO       ICON_MD_TIMELAPSE

#define ICON_SETTINGS     ICON_MD_SETTINGS
#define ICON_LANGUAGE     ICON_MD_G_TRANSLATE
#define ICON_PERSONA      ICON_MD_FACE
#define ICON_SOCIAL       ICON_MD_MESSAGE
#define ICON_GAME         ICON_MD_ROCKET_LAUNCH
#define ICON_KEYBOARD     ICON_MD_KEYBOARD
#define ICON_MOUSE        ICON_MD_MOUSE
#define ICON_GAMEPAD      ICON_MD_GAMEPAD
#define ICON_MONITOR      ICON_MD_MONITOR
#define ICON_WIFI         ICON_MD_WIFI
#define ICON_BUDGET       ICON_MD_SAVINGS
#define ICON_NEW_FOLDER   ICON_MD_CREATE_NEW_FOLDER
#define ICON_PLUGIN       ICON_MD_EXTENSION
#define ICON_RESTART      ICON_MD_REPLAY
#define ICON_QUIT         ICON_MD_CLOSE

#define ICON_SCRIPT       ICON_MD_CONTENT_PASTE

#define ICON_POWER            ICON_MD_BOLT // ICON_MD_POWER
#define ICON_BATTERY_CHARGING ICON_MD_BATTERY_CHARGING_FULL
#define ICON_BATTERY_LEVELS \
        ICON_MD_BATTERY_ALERT, \
        ICON_MD_BATTERY_0_BAR,ICON_MD_BATTERY_1_BAR, \
        ICON_MD_BATTERY_2_BAR,ICON_MD_BATTERY_3_BAR, \
        ICON_MD_BATTERY_4_BAR,ICON_MD_BATTERY_5_BAR, \
        ICON_MD_BATTERY_6_BAR,ICON_MD_BATTERY_FULL

#endif

enum editor_keys {
    key_screenshot, // @todo: add meta-info in exif or invisibile pixels (cam details, player details, map level, map location, level state, etc)
};

#if 0
void editor_menubar() {
    do_once editor_init();

    if( input_down(KEY_F11) ) editor_key = key_fullscreen;
    if( input_down(KEY_PAUSE) ) editor_key = key_pause;
    if( input_down(KEY_PRINT) ) editor_key = (mods ? key_recording : key_screenshot);
    // if( input_down(KEY_W) && input_held(KEY_LCTRL) ) editor_key = key_quit;

    if( ctrls ) {
        /**/ if( input_down(KEY_Z) ) editor_key = key_undo;
        else if( input_down(KEY_Y) ) editor_key = key_redo;
        else if( input_down(KEY_S) ) editor_key = key_save_disk;
    }

    if( !editor_key && editor_selected_obj ) {
        if( input_up(MOUSE_L)    ) editor_key = key_save_mem;
        if( input_down(MOUSE_R)  ) ui_show("Properties", true);
    }

    // @fixme: send all editor keys to game?
    // if( input_repeat(KEY_ESC, 300)) {}
    // if( input_repeat(KEY_F1, 300)) {}
    // etc...

    // menubar

    if( ui_menu( ICON_SETTINGS "@Preferences;" 
                ICON_LANGUAGE " Language;" 
                ICON_PERSONA " Profile;" // editor account, but also fake profile and 1st party credentials
                ICON_SOCIAL " Social;"
                ICON_GAME " Game;" // 
                ICON_WIFI " Network;"
                ICON_BUDGET " Budget;" // mem,gfx,net,hdd,... also logging
                ICON_NEW_FOLDER " Folders;" // including project folders
                ICON_RESTART " Restart;"
                ICON_QUIT " Quit;"
                "-" ICON_MD_RECYCLING " Reset all preferences;" ICON_MD_SAVE_AS " Save all preferences"
    ) ) {
        if( ui_item() == 3 ) {} // key mappings
        if( ui_item() == 4 ) {} // sensitivity, invert xylrw
        if( ui_item() == 5 ) {} // sensitivity, invert xy,ab, deadzones
        if( ui_item() == 7 ) {} // name,email,icon,link,github
        if( ui_item() == 13) editor_key = key_reload;
        if( ui_item() == 14) editor_key = key_quit;
    }

    static char game_args[16] = "--game-args"; // @fixme @todo remove '_' special char to signal that ui_menu() is writeable (inputbox)
    if( ui_menu_editbox( game_args, 16 ) ) {}

    // ICON_MD_TROUBLESHOOT -> PROFILER
    // ICON_MD_SCHEMA -> GRAPHNODES
    // ICON_MD_ACCOUNT_TREE -> GRAPHNODES
    // ICON_MD_TIPS_AND_UPDATES -> BULB
    // if( ui_menu( ICON_MD_MENU )) {}

    // logic: either plug icon (power saving off) or one of the following ones (power saving on):
    //        if 0% batt (no batt): battery alert
    //        if discharging:       battery levels [alert,0..6,full]
    //        if charging:          battery charging
    int battery_read = app_battery();
    int battery_level = abs(battery_read);
    int battery_discharging = battery_read < 0 && battery_level < 100;
    const char *battery_levels[] = { // @todo: remap [7%..100%] -> [0..1] ?
        ICON_BATTERY_LEVELS
    };
    if( ui_menu( !editor_power_saving ? ICON_POWER"@Full power. Tap to save power." :
        va("%s@Power saving. Tap to go full power. %3d%% battery.",
            battery_read == 0 ? battery_levels[0] :
            battery_discharging ? battery_levels[(int)((countof(battery_levels)-1)*clampf(battery_level/100.f,0,1))] :
              ICON_BATTERY_CHARGING, battery_level) ))
        editor_key = key_battery;

    // @todo: combine-in-1? cycle mem -> cpu/profiler -> network mon -> debugger

    // bug report, profile, warnings, time/chrono (@todo: alarm/snooze? calendar?)
    if( ui_menu( ICON_MD_BUG_REPORT /*"^"*/ "0" ) ) {}
    if( ui_menu( ICON_MD_FACE /*"^"*/ "3" ) ) {} // @todo: do both messaging/warnings + profile settings here
    {
        static double base = 0, tap = 0;

        if( tap == 0 ) tap = time_ss();
        double delta = time_ss() - tap;
        tap = time_ss();
        base += delta * !window_has_pause();

        if( ui_menu( base == 0 ?
            va(ICON_CLOCK "%02d:%02d", (int)((date() / 10000) % 100), (int)((date() / 100) % 100))
            :
            va(ICON_CHRONO "%03dm:%02ds:%02df@Tap to reset chrono.",((int)(base/60))%60,((int)base)%60,(int)((base - (int)base)*window_fps_target())))
            || editor_key == key_stop
        ) {
            base = 0, tap = 0;
        }
    }

#if 0
    for each_map_ptr(editor_state, void *, o, editor_state_t, ed) {
        profile_incstat("Editor.num_objects", +1);

        void *obj = *o;

#if 1
#elif 0
        // auto-load from disk during init. @fixme kvdb database
        if( array_count(ed->history) == 0 )
            if( editor_load_disk(obj, editor_obj_string(obj, ".path")) )
                {}

        // auto-save in-mem during first edit
        if( array_count(ed->history) == 0 )
            editor_save_mem(obj);
#endif

        // @todo: continue if obj not found in selection set
        if( obj != editor_selected_obj )
            continue;

        if( editor_key == key_debugger  ) { breakpoint("User requested breakpoint on this object"); }
#if 1
#elif 0
        if( editor_key == key_reset     ) { const char *ok = editor_reset(obj) ? "ok" : "err"; EDITOR_PRINTF("reset: %s\n", ok); }
        if( editor_key == key_save_mem  ) { const char *ok = editor_save_mem(obj) ? "ok" : "err"; EDITOR_PRINTF("mem saved: %s\n", ok); }
        if( editor_key == key_undo      ) { const char *ok = editor_undo(obj) ? "ok" : "err"; EDITOR_PRINTF("undo: %s\n",  ok); }
        if( editor_key == key_redo      ) { const char *ok = editor_redo(obj) ? "ok" : "err"; EDITOR_PRINTF("redo: %s\n",  ok); }
        if( editor_key == key_save_disk ) { const char *ok = editor_save_disk(obj, editor_obj_string(obj, ".path")) ? "ok" : "err"; EDITOR_PRINTF("save: %s\n",  ok); }
        if( editor_key == key_load_disk ) { const char *ok = editor_load_disk(obj, editor_obj_string(obj, ".path")) ? "ok" : "err"; EDITOR_PRINTF("load: %s\n",  ok); }
#endif
    }
#endif
}
#endif
