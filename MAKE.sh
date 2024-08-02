#!/bin/bash

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
    rm demos/lua/.art*.zip 2> /dev/null
    rm demos/lua/libv4k* 2> /dev/null
    rm demos/html5/.art*.zip 2> /dev/null
    rm v4k_*.* 2> /dev/null
    rm 3rd_*.* 2> /dev/null
    rm libv4k* 2> /dev/null
    rm -rf *.dSYM 2> /dev/null
    rm *.png 2> /dev/null
    rm *.mp4 2> /dev/null
    rm editor 2> /dev/null
    rm temp_* 2> /dev/null
    rm hello 2> /dev/null
    exit
fi
# shortcuts for split & join scripts
if [ "$1" = "split" ]; then
    sh tools/split.bat
    exit
fi
if [ "$1" = "join" ]; then
    sh tools/join.bat
    exit
fi
# cook
if [ "$1" = "cook" ]; then
    cc -o cook tools/cook.c -Iengine
    ./cook
    exit
fi
# sync
if [ "$1" = "sync" ]; then
    git reset --hard HEAD^1 && git pull
    sh MAKE.bat tidy
    exit
fi

export dll=dll
export build=dev
export args=
export cc=cc
export app=$1

while [ $# -ge 1 ]; do
    if [ "$1" = "help" ]; then 
        echo sh MAKE.bat
        echo sh MAKE.bat [gcc,clang,tcc] [dbg,dev,rel] [dll,static]
        echo sh MAKE.bat [tidy]
        echo sh MAKE.bat [split,join]
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
        export flags="-O0 -g -DDEBUG"
    fi
    if [ "$1" = "dev" ]; then 
        export build=dev
        export flags="-O0 -g -DNDEBUG=1"
    fi
    if [ "$1" = "ret" ]; then 
        export build=ret
        export flags="-O3 -DNDEBUG=3 -DENABLE_RETAIL"
    fi
    if [ "$1" = "rel" ]; then 
        export build=rel
        export flags="-O3 -DNDEBUG=2"
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
    chmod +x demos/lua/luajit.linux

    export args="-lm -ldl -lpthread -lX11 -w -Iengine/ $args"
    echo build=$build, type=$dll, cc=$cc, args=$args

    # framework (as dynamic library)
    if [ "$dll" = "dll" ]; then
        echo libv4k.so  && $cc -o libv4k.so engine/v4k.c -shared -fPIC $flags $args
        cp libv4k.so demos/lua/
        export import="libv4k.so -Wl,-rpath,./"
    else
    # framework (static)
        echo v4k        && $cc -c engine/v4k.c -w    $flags $args
        export import=v4k.o
    fi

    # editor
    echo editor        && $cc -o editor        tools/editor/editor.c $flags $import $args &

    # demos
    echo hello         && $cc -o hello         hello.c               $flags         $args &
    echo 00-loop       && $cc -o 00-loop       demos/00-loop.c       $flags $import $args &
    echo 00-script     && $cc -o 00-script     demos/00-script.c     $flags $import $args &
    echo 01-demo2d     && $cc -o 01-demo2d     demos/01-demo2d.c     $flags $import $args &
    echo 01-ui         && $cc -o 01-ui         demos/01-ui.c         $flags $import $args &
    echo 01-easing     && $cc -o 01-easing     demos/01-easing.c     $flags $import $args &
    echo 01-font       && $cc -o 01-font       demos/01-font.c       $flags $import $args &
    echo 02-ddraw      && $cc -o 02-ddraw      demos/02-ddraw.c      $flags $import $args &
    echo 02-frustum    && $cc -o 02-frustum    demos/02-frustum.c    $flags $import $args &
    echo 03-anims      && $cc -o 03-anims      demos/03-anims.c      $flags $import $args &
    echo 04-actor      && $cc -o 04-actor      demos/04-actor.c      $flags $import $args &
    echo 06-scene      && $cc -o 06-scene      demos/06-scene.c      $flags $import $args &
    echo 07-netsync    && $cc -o 07-netsync    demos/07-netsync.c    $flags $import $args &
    echo 06-material   && $cc -o 06-material   demos/06-material.c   $flags $import $args &
    echo 07-network    && $cc -o 07-network    demos/07-network.c    $flags $import $args &
    echo 08-audio      && $cc -o 08-audio      demos/08-audio.c      $flags $import $args &
    echo 08-video      && $cc -o 08-video      demos/08-video.c      $flags $import $args &
    echo 09-cubemap    && $cc -o 09-cubemap    demos/09-cubemap.c    $flags $import $args &
    echo 09-shadertoy  && $cc -o 09-shadertoy  demos/09-shadertoy.c  $flags $import $args
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
    chmod +x demos/lua/luajit.osx

    export args="-w -Iengine/ -framework cocoa -framework iokit -framework CoreFoundation -framework CoreAudio -framework AudioToolbox $args $flags"
    echo build=$build, type=$dll, cc=$cc, args=$args

    if [ ! -f libv4k.dylib ]; then
        # framework (as dynamic library)
        if [ "$dll" = "dll" ]; then
            echo libv4k    && cc -ObjC -dynamiclib -o libv4k.dylib engine/v4k.c $flags $args
            cp libv4k.dylib demos/lua
            export import=libv4k.dylib
        else
        # framework
            echo v4k       && cc -c -ObjC engine/v4k.c $flags $args
            export import=v4k.o
        fi
    fi

    # User-defined apps
    if [ -n "$app" ]; then
        echo "$app" && $cc -ObjC "$app" libv4k.dylib $args -o "v4k.osx" || rc=1
        # echo "$app" && $cc -ObjC "$app" engine/v4k.c $args -o "v4k.osx" || rc=1
    fi

    # if [ "$run" == "yes" ]; then
    #     exename="$1.osx"
    #     if [ -n "$other" ]; then
    #         exename=$(basename "$other" .exe)
    #     fi
    #     echo "run $exename $run_args"
    #     ./"$exename" $run_args || rc=1
    # fi


    # # editor
    # echo editor        && cc -o editor        tools/editor/editor.c $import $flags $args &

    # # demos
    # echo hello         && cc -o hello -ObjC   hello.c                       $flags $args &
    # echo 00-loop       && cc -o 00-loop       demos/00-loop.c       $import $flags $args &
    # echo 00-script     && cc -o 00-script     demos/00-script.c     $import $flags $args &
    # echo 01-demo2d     && cc -o 01-demo2d     demos/01-demo2d.c     $import $flags $args &
    # echo 01-ui         && cc -o 01-ui         demos/01-ui.c         $import $flags $args &
    # echo 01-easing     && cc -o 01-easing     demos/01-easing.c     $import $flags $args &
    # echo 01-font       && cc -o 01-font       demos/01-font.c       $import $flags $args &
    # echo 02-ddraw      && cc -o 02-ddraw      demos/02-ddraw.c      $import $flags $args &
    # echo 02-frustum    && cc -o 02-frustum    demos/02-frustum.c    $import $flags $args &
    # echo 03-anims      && cc -o 03-anims      demos/03-anims.c      $import $flags $args &
    # echo 04-actor      && cc -o 04-actor      demos/04-actor.c      $import $flags $args &
    # echo 06-scene      && cc -o 06-scene      demos/06-scene.c      $import $flags $args &
    # echo 07-netsync    && cc -o 07-netsync    demos/07-netsync.c    $import $flags $args &
    # echo 06-material   && cc -o 06-material   demos/06-material.c   $import $flags $args &
    # echo 07-network    && cc -o 07-network    demos/07-network.c    $import $flags $args &
    # echo 08-audio      && cc -o 08-audio      demos/08-audio.c      $import $flags $args &
    # echo 08-video      && cc -o 08-video      demos/08-video.c      $import $flags $args &
    # echo 09-cubemap    && cc -o 09-cubemap    demos/09-cubemap.c    $import $flags $args &
    # echo 09-shadertoy  && cc -o 09-shadertoy  demos/09-shadertoy.c  $import $flags $args
fi

exit
