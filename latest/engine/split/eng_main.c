// ----------------------------------------------------------------------------

static void v4k_pre_init() {
    window_icon(va("%s%s.png", app_path(), app_name()));

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
    // cook cleanup
    cook_stop();

    vfs_reload();

    // init subsystems that depend on cooked assets now

    int i;
    #pragma omp parallel for
    for( i = 0; i <= 3; ++i ) {
        if(i == 0) scene_init(); // init these on thread #0, since both will be compiling shaders, and shaders need to be compiled from the very same thread than glfwMakeContextCurrent() was set up
        if(i == 0) ui_init(); // init these on thread #0, since both will be compiling shaders, and shaders need to be compiled from the very same thread than glfwMakeContextCurrent() was set up
        if(i == 0) window_icon(va("%s.png", app_name())); // init on thread #0, because of glfw
        if(i == 0) input_init(); // init on thread #0, because of glfw
        if(i == 1) audio_init(0);
        if(i == 2) script_init(), kit_init(), midi_init();
        if(i == 3) network_init();
    }

    // display window
    glfwShowWindow(window);
    
    // set black screen
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);
    
#if !DISABLE_ENGINE_BRANDING
    texture_t logo_bg = texture("v4_logo_background.png", 0);
    if (logo_bg.id != texture_checker().id) {
        texture_t logo_mask = texture("v4_logo_mask.png", 0);
        font_face(FONT_FACE3, "fonts/Montserrat-Regular.ttf", 48.0f, FONT_EU | FONT_4096 | FONT_OVERSAMPLE_X | FONT_OVERSAMPLE_Y);
        sprite(logo_bg, vec3(w/2.f, h/2.f, 0).array, 0.0f, 0xFFFFFFFF, SPRITE_CENTERED|SPRITE_RESOLUTION_INDEPENDANT);
        sprite(logo_mask, vec3(w/2.f, h/2.f, 1).array, 0.0f, 0xFFFFFFFF, SPRITE_CENTERED|SPRITE_RESOLUTION_INDEPENDANT);
        sprite_flush();
        font_print(
            FONT_BOTTOM FONT_CENTER FONT_FACE3 FONT_H3
            "Powered by V4K engine\n\n\n\n\n\n\n\n\n\n"
        );

        int year = 2024;
        {
            time_t now;
            time(&now);
            struct tm *local_time = localtime(&now);
            year = local_time->tm_year + 1900;
        }

        char *legalese = va(FONT_CENTER FONT_FACE3 FONT_H4
            "© %d v4.games. All rights reserved.\nAll other trademarks and copyrights\n" \
            "are properties of their owners.\n\n\n\n", year);
        vec2 dims = font_rect(legalese);
        font_goto(0, window_height()-dims.y);
        font_print(legalese);

        glfwSwapBuffers(window);
    }
#endif

    glfwGetFramebufferSize(window, &w, &h); //glfwGetWindowSize(window, &w, &h);

    randset(time_ns() * !tests_captureframes());
    boot_time = -time_ss(); // measure boot time, this is continued in window_stats()

    // clean any errno setup by cooking stage
    errno = 0;

    hz = refresh_rate;
    // t = glfwGetTime();

    // preload brdf LUT early
    (void)brdf_lut();

    uint64_t fps = optioni("--fps", 0);
    if( fps ) {
        window_fps_lock(fps);
    }
}

// ----------------------------------------------------------------------------

static
void v4k_quit(void) {
    storage_flush();
    midi_quit();
}

void v4k_init() {
    do_once {
        // install signal handlers
        ifdef(debug, trap_install());

        // init panic handler
        panic_oom_reserve = SYS_MEM_REALLOC(panic_oom_reserve, 1<<20); // 1MiB

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
        if( /* !COOK_ON_DEMAND && */ have_tools() && cook_jobs() ) {
            cook_start(COOK_INI, "**", 0|COOK_ASYNC|COOK_CANCELABLE );
        }

        atexit(v4k_quit);
    }
}
