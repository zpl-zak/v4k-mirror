// ----------------------------------------------------------------------------

static void v4k_pre_init() {
    const char *appname = app_name();
    const char *appdir = app_path();
    window_icon(va("%s/%s.png", appdir, appname));
    ifdef(win32,window_icon(va("%s/%s.ico", appdir, appname)));

    glfwPollEvents();

    int i;
    #pragma omp parallel for
    for( i = 0; i <= 3; ++i) {
        /**/ if( i == 0 ) ddraw_init();// init this on thread#0 since it will be compiling shaders, and shaders need to be compiled from the very same thread than glfwMakeContextCurrent() was set up
        else if( i == 1 ) sprite_init();
        else if( i == 2 ) profiler_init();
        else if( i == 3 ) storage_mount("save/"), storage_read(), touch_init(); // for ems
    }

    // window_swap();
}
static void v4k_post_init(float refresh_rate) {
    int i;

    // cook cleanup
    cook_stop();

    vfs_reload();

    // init subsystems that depend on cooked assets now. ui_init() is special case and needs to be safely in single thread
    ui_init();

    // init more subsystems; beware of VFS mounting, as some of these may need cooked assets at this point
    #pragma omp parallel for
    for( i = 0; i <= 3; ++i) {
        /**/ if( i == 0 ) scene_init(); // init these on thread #0, since both will be compiling shaders, and shaders need to be compiled from the very same thread than glfwMakeContextCurrent() was set up
        else if( i == 1 ) audio_init(0); // initialize audio after cooking // reasoning for this: do not launch audio threads while cooks are in progress, so there is more cpu for cooking actually
        else if( i == 2 ) script_init(), kit_init(), midi_init();
        else if( i == 3 ) input_init(), network_init();
    }

    const char *appname = app_name();
    window_icon(va("%s.png", appname));
    window_icon(va("%s.ico", appname));

    // display window
    glfwShowWindow(window);
    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    randset(time_ns());
    boot_time = -time_ss(); // measure boot time, this is continued in window_stats()

    // clean any errno setup by cooking stage
    errno = 0;

    hz = refresh_rate;
    // t = glfwGetTime();
}

// ----------------------------------------------------------------------------

static
void v4k_quit(void) {
    storage_flush();
    midi_quit();
}

void v4k_init() {
    do_once {
        // abort run if any test suite failed
        if( test_errs ) exit(-1);

        // install signal handlers
        ifdef(debug, trap_install());

        // init panic handler
        panic_oom_reserve = SYS_REALLOC(panic_oom_reserve, 1<<20); // 1MiB

        // init glfw
        glfw_init();

        // enable ansi console
        tty_init();

        // chdir to root (if invoked as tcc -g -run)
        // chdir(app_path());

        // skip tcc argvs (if invoked as tcc file.c v4k.c -g -run) (win)
        if( __argc > 1 ) if( strstr(__argv[0], "/tcc") || strstr(__argv[0], "\\tcc") ) {
            __argc = 0;
        }

        // create or update cook.zip file
        if( /* !COOK_ON_DEMAND && */ file_exist(COOK_INI) && cook_jobs() ) {
            cook_start(COOK_INI, "**", 0|COOK_ASYNC|COOK_CANCELABLE );
        }

        atexit(v4k_quit);
    }
}

// Enable more performant GPUs on laptops. Does this work into a dll?
// int NvOptimusEnablement = 1;
// int AmdPowerXpressRequestHighPerformance = 1;

#if is(linux) && is(tcc) // fixes `tcc: error: undefined symbol '__dso_handle'`
int __dso_handle; // compiled with: `tcc demo.c v4k.c -D__STDC_NO_VLA__ -lX11`
#endif

#if is(win32) && is(tcc) // fixes `tcc: error: undefined symbol '_InterlockedExchangeAdd'` when compiling with `-m64` flag
__CRT_INLINE LONG _InterlockedExchangeAdd(LONG volatile *add, LONG val) {
    LONG old;
    do old = *add; while( InterlockedCompareExchange(add, old + val, old) != old );
    return old;
}
__CRT_INLINE LONGLONG _InterlockedExchangeAdd64(LONGLONG volatile *add, LONGLONG val) { // 64bit version, for completeness
    LONGLONG old;
    do old = *add; while( InterlockedCompareExchange64(add, old + val, old) != old );
    return old;
}
#endif

#ifdef ZIG_CC
static int IN6_IS_ADDR_V4MAPPED(const struct in6_addr *a) { return ((a->s6_words[0]==0) && (a->s6_words[1]==0) && (a->s6_words[2]==0) && (a->s6_words[3]==0) && (a->s6_words[4]==0) && (a->s6_words[5]==0xffff)); }
const struct in6_addr in6addr_any; // = IN6ADDR_ANY_INIT;
//static const struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;
#endif

ifdef(retail, AUTORUN { fclose(stderr); fclose(stdout); } )
