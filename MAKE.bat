#!/bin/bash 2>nul || goto :windows

# linux + osx -----------------------------------------------------------------
cd `dirname $0`

# copy demos to root folder. local changes are preserved
# cp -n demos/*.c .

# rem tests
# clang editor.c -I. -lm -lX11 -g -fsanitize=address,undefined && ./a.out
# cl editor.c -I. -fsanitize=address /DEBUG /Zi && editor

# tidy environment
if [ "$1" = "tidy" ]; then
    rm 0?-* 2> /dev/null
    rm v4k.o 2> /dev/null
    rm .art*.zip 2> /dev/null
    rm engine/bind/.art*.zip 2> /dev/null
    rm demos/html5/.art*.zip 2> /dev/null
    rm engine/bind/libv4k* 2> /dev/null
    rm v4k_*.* 2> /dev/null
    rm 3rd_*.* 2> /dev/null
    rm libv4k* 2> /dev/null
    rm -rf *.dSYM 2> /dev/null
    rm *.png 2> /dev/null
    rm *.mp4 2> /dev/null
    rm editor.linux 2> /dev/null
    rm editor.osx 2> /dev/null
    rm temp_* 2> /dev/null
    exit
fi
# shortcuts for split & join amalgamation scripts
if [ "$1" = "split" ]; then
    sh tools/split.bat
    exit
fi
if [ "$1" = "join" ]; then
    sh tools/join.bat
    exit
fi
if [ "$1" = "prep" ]; then
    sh MAKE.bat join
    sh MAKE.bat amalgamation
    exit
fi
# cook
if [ "$1" = "cook" ]; then
    cc -o cook tools/cook.c -I.
    ./cook
    exit
fi
# sync
if [ "$1" = "sync" ]; then
    git reset --hard HEAD^^1 && git pull
    sh MAKE.bat tidy
    exit
fi

export dll=dll
export build=dev
export args=
export cc=cc

while [ $# -ge 1 ]; do
    if [ "$1" = "help" ]; then 
        echo sh MAKE.bat
        echo sh MAKE.bat [gcc,clang,tcc] [dbg,dev,rel] [dll,static]
        echo sh MAKE.bat [tidy]
        echo sh MAKE.bat [split,join]
        echo sh MAKE.bat [amalgamation]
        echo sh MAKE.bat [prep]
        echo sh MAKE.bat [cook]
        echo sh MAKE.bat [proj]
        exit
    fi
    if [ "$1" = "dll" ]; then 
        export dll=dll
    fi
    if [ "$1" = "static" ]; then 
        export dll=static
    fi
    if [ "$1" = "dbg" ]; then 
        export build=dbg
        export flags="-O0 -g"
    fi
    if [ "$1" = "dev" ]; then 
        export build=dev
        export flags="-O1 -g"
    fi
    if [ "$1" = "rel" ]; then 
        export build=rel
        export flags="-O3 -DNDEBUG"
    fi
    if [ "$1" = "gcc" ]; then 
        export cc=gcc
    fi
    if [ "$1" = "clang" ]; then 
        export cc=clang
    fi
    if [ "$1" = "tcc" ]; then 
        export cc="tcc -D__STDC_NO_VLA__"
    fi
    if [ "$1" = "proj" ]; then
        if [ "$(uname)" != "Darwin" ]; then
            chmod +x tools/premake5.linux
            tools/premake5.linux gmake
            tools/premake5.linux ninja
            exit
        fi
        if [ "$(uname)" = "Darwin" ]; then
            chmod +x tools/premake5.osx
            tools/premake5.osx xcode4
            tools/premake5.osx ninja
            exit
        fi
    fi
    if [ "$1" = "--" ]; then 
        shift
        export args=$*
        shift $#
    fi
    if [ $# -ge 1 ]; then
        shift
    fi
done

if [ "$(uname)" != "Darwin" ]; then

    # setup (ArchLinux)
    [ ! -f ".setup" ] && sudo pacman -S --noconfirm tcc && echo>.setup
    # setup (Debian, Ubuntu, etc)
    [ ! -f ".setup" ] && sudo apt-get -y update
    [ ! -f ".setup" ] && sudo apt-get -y install tcc libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev && echo>.setup       # absolute minimum
    #                      sudo apt-get -y install clang xorg-dev                                                                             # memorable, around 100 mib
    #                      sudo apt-get -y install clang xorg-dev libglfw3-dev libassimp-dev gcc                                              # initial revision
    #                      sudo apt-get -y install ffmpeg || (sudo apt-get install snapd && sudo snap install ffmpeg)                         # variant

    # pipeline
    #cc tools/ass2iqe.c   -o tools/ass2iqe.linux  -lm -ldl -lpthread -w -g -lassimp
    #cc tools/iqe2iqm.cpp -o tools/iqe2iqm.linux  -lm -ldl -lpthread -w -g -lstdc++
    #cc tools/mid2wav.c   -o tools/mid2wav.linux  -lm -ldl -lpthread -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x tools/ass2iqe.linux
    chmod +x tools/cook.linux
    chmod +x tools/cuttlefish.linux
    chmod +x tools/ffmpeg.linux
    chmod +x tools/furnace.linux
    chmod +x tools/iqe2iqm.linux
    chmod +x tools/mid2wav.linux
    chmod +x tools/mod2wav.linux
    chmod +x tools/PVRTexToolCLI.linux
    chmod +x tools/sfxr2wav.linux
    chmod +x tools/xlsx2ini.linux
    chmod +x tools/premake5.linux
    chmod +x tools/ninja.linux
    chmod +x engine/bind/luajit.linux

    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libv4k.so  && $cc -o libv4k.so engine/v4k.c -shared -fPIC -w -lX11 -lm -ldl -lpthread $flags $args
        cp libv4k.so engine/bind/
        export import="libv4k.so -Wl,-rpath,./"
    else
    # framework (static)
        echo v4k        && $cc -c engine/v4k.c -w    $flags $args
        export import=v4k.o
    fi

    # editor
  # echo editor        && $cc -o editor.linux  tools/editor/editor.c      -lm -ldl -lpthread -lX11 -w -Iengine/ $flags $import $args &

    # demos
    echo hello         && $cc -o hello         hello.c               -lm -ldl -lpthread -lX11 -w -Iengine/ $flags         $args &
    echo 00-ui         && $cc -o 00-ui         demos/00-ui.c         -lm -ldl -lpthread -lX11 -w -Iengine/ $flags $import $args &
    echo 01-sprite     && $cc -o 01-sprite     demos/01-sprite.c     -lm -ldl -lpthread -lX11 -w -Iengine/ $flags $import $args &
    echo 02-ddraw      && $cc -o 02-ddraw      demos/02-ddraw.c      -lm -ldl -lpthread -lX11 -w -Iengine/ $flags $import $args &
    echo 03-anims      && $cc -o 03-anims      demos/03-anims.c      -lm -ldl -lpthread -lX11 -w -Iengine/ $flags $import $args &
    echo 04-actor      && $cc -o 04-actor      demos/04-actor.c      -lm -ldl -lpthread -lX11 -w -Iengine/ $flags $import $args &
    echo 05-scene      && $cc -o 05-scene      demos/05-scene.c      -lm -ldl -lpthread -lX11 -w -Iengine/ $flags $import $args &
    echo 06-controller && $cc -o 06-controller demos/06-controller.c -lm -ldl -lpthread -lX11 -w -Iengine/ $flags $import $args &
    echo 07-network    && $cc -o 07-network    demos/07-network.c    -lm -ldl -lpthread -lX11 -w -Iengine/ $flags $import $args
fi

if [ "$(uname)" = "Darwin" ]; then
    # setup (osx)
    export SDKROOT=$(xcrun --show-sdk-path)
    # brew install glfw

    # pipeline
    #cc tools/ass2iqe.c   -o tools/ass2iqe.osx  -w -g -lassimp
    #cc tools/iqe2iqm.cpp -o tools/iqe2iqm.osx  -w -g -lstdc++
    #cc tools/mid2wav.c   -o tools/mid2wav.osx  -w -g

    # change permissions of precompiled tools binaries because of 'Permission denied' runtime error (@procedural)
    chmod +x tools/ass2iqe.osx
    chmod +x tools/cook.osx
    chmod +x tools/cuttlefish.osx
    chmod +x tools/ffmpeg.osx
    chmod +x tools/furnace.osx
    chmod +x tools/iqe2iqm.osx
    chmod +x tools/mid2wav.osx
    chmod +x tools/mod2wav.osx
    chmod +x tools/PVRTexToolCLI.osx
    chmod +x tools/sfxr2wav.osx
    chmod +x tools/xlsx2ini.osx
    chmod +x tools/premake5.osx
    chmod +x tools/ninja.osx
    chmod +x engine/bind/luajit.osx

    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libv4k     && cc -ObjC -dynamiclib -o libv4k.dylib engine/v4k.c -framework cocoa -framework iokit -framework audiotoolbox -w $flags $args
        cp libv4k.dylib engine/bind
        export import=libv4k.dylib
    else
    # framework
        echo v4k        && cc -c -ObjC engine/v4k.c -w $flags $args
        export import=v4k.o
    fi

    # editor
  # echo editor        && cc -o editor.osx -ObjC tools/editor/editor.c  -w -Iengine/ $import $flags $args -framework cocoa -framework iokit -framework audiotoolbox &

    # demos
    echo hello         && cc -o hello -ObjC   hello.c               -w -Iengine/         $flags $args -framework cocoa -framework iokit -framework audiotoolbox &
    echo 00-ui         && cc -o 00-ui         demos/00-ui.c         -w -Iengine/ $import $flags $args -framework cocoa -framework iokit -framework audiotoolbox &
    echo 01-sprite     && cc -o 01-sprite     demos/01-sprite.c     -w -Iengine/ $import $flags $args -framework cocoa -framework iokit -framework audiotoolbox &
    echo 02-ddraw      && cc -o 02-ddraw      demos/02-ddraw.c      -w -Iengine/ $import $flags $args -framework cocoa -framework iokit -framework audiotoolbox &
    echo 03-anims      && cc -o 03-anims      demos/03-anims.c      -w -Iengine/ $import $flags $args -framework cocoa -framework iokit -framework audiotoolbox &
    echo 04-actor      && cc -o 04-actor      demos/04-actor.c      -w -Iengine/ $import $flags $args -framework cocoa -framework iokit -framework audiotoolbox &
    echo 05-scene      && cc -o 05-scene      demos/05-scene.c      -w -Iengine/ $import $flags $args -framework cocoa -framework iokit -framework audiotoolbox &
    echo 06-controller && cc -o 06-controller demos/06-controller.c -w -Iengine/ $import $flags $args -framework cocoa -framework iokit -framework audiotoolbox &
    echo 07-network    && cc -o 07-network    demos/07-network.c    -w -Iengine/ $import $flags $args -framework cocoa -framework iokit -framework audiotoolbox
fi

exit


:: -----------------------------------------------------------------------------
:windows

@echo off
setlocal enableDelayedExpansion
cd /d "%~dp0"

rem show help
if "%1"=="-?" goto showhelp
if "%1"=="-h" goto showhelp
if "%1"=="help" (
    :showhelp
    echo %0                         ; compile everything: `make dll dev` alias
    echo %0 [help]                  ; show this screen
    echo %0 [docs]                  ; generate tools/docs/docs.html file
    echo %0 [cook]                  ; cook .zipfiles with tools/cook.ini cookbook
    echo %0 [sync]                  ; sync repo to latest
    echo %0 [fwk]                   ; prepare files for fwk PR
    echo %0 [lua]                   ; execute lua script with v4k
    echo %0 [html5]                 ; build HTML5 demo
    echo %0 [web]                   ; run Python webserver in html5 dir
    echo %0 [pull]                  ; pull changes from 'latest' upstream
    echo %0 [git]                   ; prepare for commit
    echo %0 [push]                  ; prepare for commit, stage changes and commit them
    echo %0 [tidy]                  ; clean up temp files
    echo %0 [bind]                  ; generate lua bindings
    echo %0 [checkmem]              ; check untracked allocators in V4K
    echo %0 [split^|join]            ; engine/v4k* ^>split^> engine/split/* or engine/split/* ^>join^> engine/v4k*
    echo %0 [glsl_split^|glsl_join]  ; join/split GLSL shaders
    echo %0 [lua]                   ; execute lua script with v4k
    echo %0 [amalgamation]          ; combine engine/v4k* into a single-header file
    echo %0 [prep]                  ; combine split files into a single-header file, ready for use
    echo %0 [sln]                   ; generate a xcode/gmake/ninja/visual studio solution
    echo %0 [cl^|tcc^|cc^|gcc^|clang^|clang-cl] [dbg^|dev^|rel] [static^|dll] [nov4k^|nodemos^|editor] [vis] [-- args]
    echo    cl       \
    echo    tcc      ^|
    echo    cc       ^| select compiler. must be accessible in PATH
    echo    gcc      ^| (will be autodetected when no option is provided^)
    echo    clang    ^|
    echo    clang-cl /
    echo    dbg      \   debug build: [x] ASAN [x] poison [x] asserts [x] profiler [x] symbols                    [ ] zero optimizations
    echo    dev      ^| develop build: [ ] ASAN [x] poison [x] asserts [x] profiler [x] symbols                    [*] some optimizations (default^)
    echo    rel      / release build: [ ] ASAN [ ] poison [ ] asserts [ ] profiler [x] symbols (cl,clang-cl only^) [x] many optimizations
    echo    static   \ link v4k as static library
    echo    dll      / link v4k as dynamic library (dll^) (default^)
    echo    nov4k    \ do not compile framework
    echo    demos    ^| do compile demos
    echo    editor   / do compile editor
    echo    vis      ^> visualize invokation cmdline.
    echo    args     ^> after `--` separator is found, pass all remaining arguments to compiler as-is
    echo.
    exit /b
)

rem sync repo to latest
if "%1"=="sync" (
    call MAKE.bat tidy
    git reset --hard HEAD~1 && git pull
    exit /b
)

rem cook asset files
if "%1"=="cook" (
    rem generate cooker twice: use multi-threaded version if available (cl). then cook.
    rem call tools\tcc tools\cook.c -Iengine engine\v4k.c
    rem             cl tools\cook.c -Iengine engine\v4k.c
    rem cook
    tools\cook

    exit /b
)
rem generate bindings
if "%1"=="bind" (
    rem luajit
    tools\luajit tools\luajit_make_bindings.lua > v4k.lua
    move /y v4k.lua engine\bind

    exit /b
)

if "%1"=="lua" (
    pushd engine\bind
        luajit "..\..\%2"
    popd
    exit /b
)

rem generate documentation
if "%1"=="docs" (
    rem set symbols...
    git describe --tags --abbrev=0 > info.obj
    set /p VERSION=<info.obj
    git rev-list --count --first-parent HEAD > info.obj
    set /p GIT_REVISION=<info.obj
    git rev-parse --abbrev-ref HEAD > info.obj
    set /p GIT_BRANCH=<info.obj
    date /t > info.obj
    set /p LAST_MODIFIED=<info.obj

    rem ...and generate docs
    cl   tools\docs\docs.c engine\v4k.c -Iengine %2
    docs engine\v4k.h --excluded=3rd_glad.h,v4k.h,v4k_compat.h, > v4k.html
    move /y v4k.html engine\

    exit /b
)
rem generate single-header distribution
if "%1"=="amalgamation" (
echo // This file is intended to be consumed by a compiler. Do not read.  > v4k.h
echo // **Browse to any of the sources in engine/split/ folder instead** >> v4k.h
echo // ---------------------------------------------------------------- >> v4k.h
echo // #define V4K_IMPLEMENTATION early in **one** C file to unroll the >> v4k.h
echo // implementation. The symbol must be defined in a C (not C++^) file>> v4k.h
echo // ---------------------------------------------------------------- >> v4k.h
echo #pragma once                                                        >> v4k.h
type engine\split\3rd_font_md.h                                          >> v4k.h
type engine\split\3rd_glad.h                                             >> v4k.h
type engine\v4k.h                                                        >> v4k.h
echo #ifdef V4K_IMPLEMENTATION                                           >> v4k.h
echo #define V4K_3RD                                                     >> v4k.h
type engine\v4k                                                          >> v4k.h
type engine\v4k.c                                                        >> v4k.h
echo #endif // V4K_IMPLEMENTATION                                        >> v4k.h
move /y v4k.h engine\joint
exit /b
)

rem generate prior files to a git release
if "%1"=="git" (
    rem call make.bat dll
    call make.bat docs
    call make.bat bind

    call make.bat glsl_join
    call make.bat amalgamation
    call make.bat split

rem rd /q /s engine\split
rem md engine\split
rem move /y v4k_*.? engine\split\
rem move /y 3rd_*.? engine\split\

    call make.bat tidy

    exit /b
)

if "%1"=="pull" (
    git remote add fwk git@github.com:r-lyeh/FWK.git 2>NUL
    git fetch fwk
    git merge -Xrename-threshold=50 --allow-unrelated-histories fwk/main
    exit /b
)

if "%1"=="push" (
    call make.bat tidy

    git status
    git add .
    git commit
    if "%2"=="out" (
        git push
    )

    exit /b
)

if "%1"=="prep" (
    call make.bat glsl_join
    call make.bat join
    call make.bat amalgamation
    exit /b
)

rem shortcuts for split & join amalgamation scripts
if "%1"=="split" (
    call tools\split
    exit /b
)
if "%1"=="join" (
    call tools\join
    exit /b
)

if "%1"=="glsl_split" (
    call tools\glsl_split
    exit /b
)
if "%1"=="glsl_join" (
    call tools\glsl_join
    exit /b
)

rem check memory api calls
if "%1"=="checkmem" (
    findstr /RNC:"[^_xv]realloc[(]"  engine\v4k.c engine\split\v4k*
    findstr /RNC:"[^_xv]xrealloc[(]" engine\v4k.c engine\split\v4k*
    findstr /RNC:"[^_xv]malloc[(]"   engine\v4k.c engine\split\v4k*
    findstr /RNC:"[^_xv]free[(]"     engine\v4k.c engine\split\v4k*
    findstr /RNC:"[^_xv]calloc[(]"   engine\v4k.c engine\split\v4k*
    findstr /RNC:"[^_xv]strdup[(]"   engine\v4k.c engine\split\v4k*
    exit /b
)

if "%1"=="html5" (
    pushd demos\html5
        call make.bat %2
    popd
    exit /b
)

if "%1"=="web" (
    python -m http.server --directory demos\html5 --bind 127.0.0.1 8000
    exit /b
)

if "%1"=="fwk" (
    if not exist "_fwk" mkdir "_fwk"
    setlocal enabledelayedexpansion
    xcopy /y "engine\split\3rd_*" "_fwk\"
    xcopy /y "demos\*.c" "_fwk\"
    for %%f in ("engine\split\v4k*") do (
        set "filename=%%~nf"
        set "newname=fwk!filename:v4k=!%%~xf"
        echo Copying and renaming "%%f" to "_fwk\!newname!"
        copy "%%f" "_fwk\!newname!"
    )
    for %%f in (_fwk\*) do (
        set "filename=%%~nxf"
        if /i not "!filename:~0,4!"=="3rd_" (
            echo Processing: %%f
            tools\fwkren.exe %%f from
        ) else (
            echo Skipping %%f
        )
    )

    echo All done.
    endlocal
    exit /b
)

if "%1"=="back" (
    if not exist "_fwk" exit /b
    setlocal enabledelayedexpansion
    for %%f in ("_fwk\fwk*") do (
        set "filename=%%~nf"
        set "newname=v4k!filename:fwk=!%%~xf"
        echo Copying and renaming "%%f" to "engine\split\!newname!"
        copy "%%f" "engine\split\!newname!"
    )
    for %%f in (engine\split\*) do (
        set "filename=%%~nxf"
        if /i not "!filename:~0,4!"=="3rd_" (
            echo Processing: %%f
            tools\fwkren.exe %%f to
        ) else (
            echo Skipping %%f
        )
    )

    echo All done.
    endlocal
    exit /b
)

rem copy demos to root folder. local changes are preserved
rem echo n | copy /-y demos\*.c 1> nul 2> nul

rem tidy environment
if "%1"=="tidy" (
    move /y ??-*.png demos          > nul 2> nul
    move /y ??-*.c demos            > nul 2> nul
    del engine\bind\v4k.dll         > nul 2> nul
    del .temp*.*                    > nul 2> nul
    del *.zip                       > nul 2> nul
    del *.mem                       > nul 2> nul
    del *.exp                       > nul 2> nul
    del tools\*.exp                 > nul 2> nul
    del *.lib                       > nul 2> nul
    del *.exe                       > nul 2> nul
    del *.obj                       > nul 2> nul
    del tools\*.obj                 > nul 2> nul
    del *.o                         > nul 2> nul
    del *.a                         > nul 2> nul
    del *.pdb                       > nul 2> nul
    del *.ilk                       > nul 2> nul
    del *.png                       > nul 2> nul
    del *.mp4                       > nul 2> nul
    del *.def                       > nul 2> nul
    del *.dll                       > nul 2> nul
    del 3rd_*.*                     > nul 2> nul
    del v4k_*.*                     > nul 2> nul
rem del ??-*.*                      > nul 2> nul
    del temp_*.*                    > nul 2> nul
    rd /q /s .vs                    > nul 2> nul
    rd /q /s _debug                 > nul 2> nul
    rd /q /s _devel                 > nul 2> nul
    rd /q /s _release               > nul 2> nul
    rd /q /s _fwk                   > nul 2> nul
rem rd /q /s _project               > nul 2> nul
    del tcc.bat                     > nul 2> nul
    del sh.bat                      > nul 2> nul
    exit /b
)

set cc=%cc%
set dll=dll
set build=dev
set args=-Iengine
set other=
set v4k=yes
set hello=no
set demos=no
set extras=no
set editor=no
set vis=no
set proj=no
set rc=0

:parse_args
    if "%1"=="--"       shift && goto parse_compiler_args

    if "%1"=="dll"      set "dll=%1" && goto loop
    if "%1"=="static"   set "dll=%1" && goto loop

    if "%1"=="dbg"      set "build=%1" && goto loop
    if "%1"=="dev"      set "build=%1" && goto loop
    if "%1"=="rel"      set "build=%1" && goto loop

    if "%1"=="debug"       set "build=dbg" && goto loop
    if "%1"=="devel"       set "build=dev" && goto loop
    if "%1"=="develop"     set "build=dev" && goto loop
    if "%1"=="developer"   set "build=dev" && goto loop
    if "%1"=="development" set "build=dev" && goto loop
    if "%1"=="release"     set "build=rel" && goto loop

    if "%1"=="vis"      set "vis=yes" && goto loop

    if "%1"=="nov4k"    set "v4k=no" && goto loop
    if "%1"=="nodemos"  set "demos=no" && goto loop
    if "%1"=="demos"    set "demos=yes" && set "hello=no" && goto loop
    if "%1"=="extras"   set "extras=yes" && set "hello=no" && goto loop
    if "%1"=="noeditor" set "editor=no" && goto loop
    if "%1"=="hello"    set "hello=yes" && goto loop
    if "%1"=="editor"   set "editor=yes" && set "hello=no"&& goto loop
    if "%1"=="all"      set "v4k=yes" && set "demos=yes" && set "extras=yes" && set "editor=yes" && set "hello=yes" && goto loop

    if "%1"=="tcc"      set "cc=%1" && goto loop
    if "%1"=="cl"       set "cc=%1" && goto loop
    if "%1"=="vc"       set "cc=cl" && goto loop
    if "%1"=="cc"       set "cc=%1" && goto loop
    if "%1"=="gcc"      set "cc=%1" && goto loop
    if "%1"=="clang"    set "cc=%1" && goto loop
    if "%1"=="clang-cl" set "cc=%1" && goto loop

    if "%1"=="proj"     set "proj=yes" && goto loop

    if not "%1"==""     set "other=!other! %1" && set "editor=no" && set "demos=no"

:loop
    if not "%1"==""     shift && goto parse_args

:parse_compiler_args
    if not "%1"==""     set "args=!args! %1" && shift && goto parse_compiler_args

set vs=00
rem detect setup
if "!cc!"=="" (
    echo Detecting VS 2022/2019/2017/2015/2013 x64 ...
    set cc=cl
           if exist "%VS170COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS170COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul && set "vs=22"
    ) else if exist "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS160COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul && set "vs=19"
    ) else if exist "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%VS150COMNTOOLS%/../../VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul && set "vs=17"
    ) else if exist "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
              @call "%VS140COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" > nul && set "vs=15"
    ) else if exist "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" (
              @call "%VS120COMNTOOLS%/../../VC/bin/x86_amd64/vcvarsx86_amd64.bat" > nul && set "vs=13"
    ) else if exist "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles%/microsoft visual studio/2022/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul && set "vs=22"
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles(x86)%/microsoft visual studio/2019/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul && set "vs=19"
    ) else if exist "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" (
              @call "%ProgramFiles(x86)%/microsoft visual studio/2017/community/VC/Auxiliary/Build/vcvarsx86_amd64.bat" > nul && set "vs=17"
    ) else (
        echo Detecting Mingw64 ...
        set cc=gcc
        where /q gcc.exe || ( echo Detecting TCC ... && set "cc=tcc" )
    )
)

rem solution. @todo: lin/osx
if "!proj!"=="yes" if not "%vs%"=="00" pushd tools && premake5 vs20%vs% & popd
if "!proj!"=="yes" if     "%vs%"=="00" pushd tools && premake5 vs2013   & popd
if "!proj!"=="yes"                     pushd tools && premake5 ninja    & popd
if "!proj!"=="yes"                     pushd tools && premake5 gmake    & popd & exit /b

rem --- pipeline
rem cl tools/ass2iqe.c   /Fetools/ass2iqe.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG assimp.lib
rem cl tools/iqe2iqm.cpp /Fetools/iqe2iqm.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG
rem cl tools/mid2wav.c   /Fetools/mid2wav.exe  /nologo /openmp /O2 /Oy /MT /DNDEBUG
rem cl tools/xml2json.c  /Fetools/xml2json.exe /nologo /openmp /O2 /Oy /MT /DNDEBUG
rem --- pipeline
rem gcc tools/ass2iqe.c   -o tools/ass2iqe.exe  -w -lassimp
rem gcc tools/iqe2iqm.cpp -o tools/iqe2iqm.exe  -w -lstdc++
rem gcc tools/mid2wav.c   -o tools/mid2wav.exe  -w
rem gcc tools/xml2json.c  -o tools/xml2json.exe -w
rem --- different strategies for release builds
rem  4.6s 6.9MiB (default)
rem 33.7s 6.6MiB             /Ox     /Oy /MT /DNDEBUG
rem 35.8s 5.3MiB                 /O2 /Oy /MT /DNDEBUG
rem 17.9s 4.6MiB                 /O1     /MT /DNDEBUG /GL /GF     /arch:AVX2
rem 17.8s 4.6MiB         /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF     /arch:AVX2
rem 18.8s 4.6MiB         /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw            /link /OPT:ICF /LTCG
rem 18.0s 4.6MiB /openmp /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG

if "!cc!"=="cl" (

    if "!dll!"=="static" (
        set export=/c
        set import=v4k.obj
    ) else (
        set export=/DAPI=EXPORT /LD
        set import=/DAPI=IMPORT v4k.lib
    )

    if "!build!"=="rel" (
        set args=/nologo /Zi /MT /openmp /DNDEBUG !args!        /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
    )
    if "!build!"=="dev" (
        set args=/nologo /Zi /MT /openmp /DEBUG   !args! && REM /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2
    )
    if "!build!"=="dbg" (
        set args=/nologo /Zi /MT         /DEBUG   !args!        /Od  /fsanitize=address               
        rem make -- /RTC1, or make -- /Zi /fsanitize=address /DEBUG
    )

    set o=/Fe:
    set echo=REM

) else if "!cc!"=="clang-cl" (

    if "!dll!"=="static" (
        set export=/c
        set import=v4k.obj
    ) else (
        set export=/DAPI=EXPORT /LD
        set import=/DAPI=IMPORT v4k.lib
    )

    set warnings_v4kc=-Wno-deprecated-declarations -Wno-tautological-constant-out-of-range-compare
    set warnings_demos=-Wno-empty-body -Wno-format-security -Wno-pointer-sign
    set warnings=!warnings_v4kc! !warnings_demos!

    if "!build!"=="rel" (
        set args=!warnings! /nologo /Zi /MT /openmp /DNDEBUG !args!        /Os /Ox /O2 /Oy /GF /Gw /arch:AVX2
    )
    if "!build!"=="dev" (
        set args=!warnings! /nologo /Zi /MT /openmp /DEBUG   !args! && REM /Os /Ox /O2 /Oy /GF /Gw /arch:AVX2
    )
    if "!build!"=="dbg" (
        set args=!warnings! /nologo /Zi /MT         /DEBUG   !args!        /Od  /fsanitize=address
    )

    set o=-o
    set echo=echo

) else if "!cc!"=="tcc" (

    if "!dll!"=="static" (
        set export=-c
        set import=v4k.o
    ) else (
        set export=-DAPI=EXPORT -shared
        set import=-DAPI=IMPORT v4k.def
    )

    if "!build!"=="rel" (
        set args=-O3 -DNDEBUG !args!
    )
    if "!build!"=="dev" (
        set args=-O2 -g !args!
    )
    if "!build!"=="dbg" (
        set args=-O0 -g !args!
    )

    set o=-o
    set echo=echo

) else ( rem if "!cc!"=="gcc" or "clang"

    set libs=-lws2_32 -lgdi32 -lwinmm -ldbghelp -lole32 -lshell32 -lcomdlg32

    if "!dll!"=="static" (
        set export=-c
        set import=v4k.o !libs! -Wl,--allow-multiple-definition
    ) else (
        set export=-DAPI=EXPORT -shared -o v4k.dll !libs! -Wl,--out-implib,v4k.a
        set import=-DAPI=IMPORT v4k.a
    )

    set args=-Wno-implicit-function-declaration !args!

    if "!build!"=="rel" (
        rem @todo see: https://stackoverflow.com/questions/866721/how-to-generate-gcc-debug-symbol-outside-the-build-target
        set args=-O3 -DNDEBUG !args!
    )
    if "!build!"=="dev" (
        set args=-g -O1 !args!
    )
    if "!build!"=="dbg" (
        set args=-g -O0 !args!
    )

    set o=-o
    set echo=echo
)

echo build=!build!, type=!dll!, cc=!cc!, other=!other!, args=!args!
echo import=!import!, export=!export!

if "!cc!"=="tcc" set "cc=call tools\tcc"

rem detect wether user-defined sources use single-header distro
rem if so, remove API=IMPORT flags and also do not produce v4k.dll by default
if not "!other!"=="" (
    >nul find "V4K_IMPLEMENTATION" !other! && (
      set import=
      set v4k=no
    )
)

rem framework
if "!v4k!"=="yes" (
if "!vis!"=="yes" echo !cc! engine\v4k.c !export! !args! ^&^& if "!dll!"=="dll" copy /y v4k.dll engine\bind ^> nul
!echo! v4k          && !cc! engine\v4k.c !export! !args!   && if "!dll!"=="dll" copy /y v4k.dll engine\bind  > nul || set rc=1
)

rem editor
if "!editor!"=="yes" (
set edit=-DCOOK_ON_DEMAND -DUI_LESSER_SPACING -DUI_ICONS_SMALL
if "!vis!"=="yes" echo !cc! !o! editor.exe  tools\editor\editor.c  !edit! !import! !args!
!echo! editor       && !cc! !o! editor.exe  tools\editor\editor.c  !edit! !import! !args! || set rc=1
!echo! editor2      && !cc! !o! editor2.exe tools\editor\editor2.c !edit!          !args! || set rc=1
)

rem demos
if "!demos!"=="yes" (
!echo! 00-demo       && !cc! !o! 00-demo.exe       demos\00-demo.c          !import! !args! || set rc=1
!echo! 00-ui         && !cc! !o! 00-ui.exe         demos\00-ui.c            !import! !args! || set rc=1
!echo! 01-sprite     && !cc! !o! 01-sprite.exe     demos\01-sprite.c        !import! !args! || set rc=1
!echo! 02-ddraw      && !cc! !o! 02-ddraw.exe      demos\02-ddraw.c         !import! !args! || set rc=1
!echo! 02-frustum    && !cc! !o! 02-frustum.exe    demos\02-frustum.c       !import! !args! || set rc=1
!echo! 03-anims      && !cc! !o! 03-anims.exe      demos\03-anims.c         !import! !args! || set rc=1
!echo! 04-actor      && !cc! !o! 04-actor.exe      demos\04-actor.c         !import! !args! || set rc=1
!echo! 04-lod        && !cc! !o! 04-lod.exe        demos\04-lod.c           !import! !args! || set rc=1
!echo! 05-scene      && !cc! !o! 05-scene.exe      demos\05-scene.c         !import! !args! || set rc=1
!echo! 06-controller && !cc! !o! 06-controller.exe demos\06-controller.c    !import! !args! || set rc=1
!echo! 07-network    && !cc! !o! 07-network.exe    demos\07-network.c       !import! !args! || set rc=1
)

rem extras
if "!extras!"=="yes" (
    for %%f in ("demos\99-*") do (
        !echo! %%~nf         && !cc! !o! %%~nf.exe         demos\%%~nf.c            !import! !args! || set rc=1
    )
)

rem hello
if "!hello!"=="yes" (
!echo! hello         && !cc! !o! hello.exe         hello.c                           !args! || set rc=1
)

rem user-defined apps
if not "!other!"=="" (
if "!vis!"=="yes" echo !cc! !other! !import! !args!
     !echo! !other! && !cc! !other! !import! !args! || set rc=1
)

rem PAUSE only if double-clicked from Windows explorer
rem (((echo.%cmdcmdline%)|%WINDIR%\system32\find.exe /I "%~0")>nul)&&pause

cmd /c exit !rc!
