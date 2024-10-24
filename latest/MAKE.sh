#!/bin/bash

set -e

# show help
if [[ "$1" == "-?" || "$1" == "/?" || "$1" == "-h" || "$1" == "help" ]]; then
    echo "$0                         ; compile everything: \`make dll dev\` alias"
    echo "$0 [help]                  ; show this screen"
    echo "$0 [docs]                  ; generate tools/docs/docs.html file"
    echo "$0 [cook]                  ; cook .zipfiles with tools/cook.ini cookbook"
    echo "$0 [build_cook]            ; build cook tool in release mode"
    echo "$0 [lua]                   ; execute lua script with v4k"
    echo "$0 [html5]                 ; build HTML5 demo"
    echo "$0 [web]                   ; run Python webserver in html5 dir"
    echo "$0 [push]                  ; sync with VCS"
    echo "$0 [pull]                  ; pull from VCS"
    echo "$0 [sync]                  ; Push changes to GitHub mirror"
    echo "$0 [shipit]                ; Release a new version to GitHub"
    echo "$0 [fuse]                  ; fuse all binaries and cooked zipfiles found together"
    echo "$0 [vps]                   ; upload the release to VPS"
    echo "$0 [tidy]                  ; clean up temp files"
    echo "$0 [bind]                  ; generate lua bindings"
    echo "$0 [checkmem]              ; check untracked allocators in V4K"
    echo "$0 [test]                  ; run autotests"
    echo "$0 [todo]                  ; check for @fixme and @todo"
    echo "$0 [leak]                  ; check for @leak"
    echo "$0 [lua]                   ; execute lua script with v4k"
    echo "$0 [sln]                   ; generate a xcode/gmake/ninja/visual studio solution"
    echo "$0 [addons[ names ] ]      ; specify list of addons you want to compile with the engine"
    echo "$0 [cl|tcc|cc|gcc|clang|clang-cl] [dbg|dev|rel|ret] [static|dll] [nov4k|nodemos|editor] [vis] [-- args]"
    echo "   cl       \\"
    echo "   tcc      |"
    echo "   cc       | select compiler. must be accessible in PATH"
    echo "   gcc      | (will be autodetected when no option is provided)"
    echo "   clang    |"
    echo "   clang-cl /"
    echo "   dbg      \   debug build: [x] ASAN [x] poison [x] asserts [x] profiler [x] symbols                    [ ] zero optimizations"
    echo "   dev      | develop build: [ ] ASAN [x] poison [x] asserts [x] profiler [x] symbols                    [*] some optimizations (default)"
    echo "   rel      / release build: [ ] ASAN [ ] poison [ ] asserts [ ] profiler [x] symbols (cl,clang-cl only) [x] many optimizations"
    echo "   static   \ link v4k as static library"
    echo "   dll      / link v4k as dynamic library (dll) (default)"
    echo "   nov4k    \ do not compile framework"
    echo "   demos    | do compile demos"
    echo "   editor   / do compile editor"
    echo "   run      | run compiled .exe"
    echo "   vis      > visualize invokation cmdline."
    echo "   args     > after \`--\` separator is found, pass all remaining arguments to compiler as-is"
    echo "   run_args > after \`//\` separator is found, pass all remaining arguments to runtime exe as-is"
    echo
    exit 0
fi

# cook asset files
if [[ "$1" == "cook" ]]; then
    # generate cooker twice: use multi-threaded version if available (cl). then cook.
    # gcc tools/cook.c -Iengine engine/v4k.c
    #              cl tools/cook.c -Iengine engine/v4k.c
    # cook
    tools/cook
    exit 0
fi

if [[ "$1" == "build_cook" ]]; then
    pushd tools
        gcc cook.c -I../engine -fopenmp -Os -O2 -ffast-math -march=native -mtune=native -DNDEBUG -ffunction-sections -fdata-sections -Wl,--gc-sections
    popd
    exit 0
fi

# generate bindings
if [[ "$1" == "bind" ]]; then
    # luajit
    tools/luajit tools/luajit_make_bindings.lua > v4k.lua
    mv -f v4k.lua bind
    exit 0
fi

if [[ "$1" == "lua" ]]; then
    pushd bind
        luajit "../$2"
    popd
    exit 0
fi

if [[ -d ".ark" ]]; then
    config_file=".ark/workspace.cfg"

    if [[ -f "$config_file" ]]; then
        while IFS='=' read -r key value; do
            if [[ "$key" == "current_cl_id" ]]; then
                current_cl_id="$value"
            fi
            if [[ "$key" == "branch_id" ]]; then
                current_branch="$value"
            fi
        done < "$config_file"
    else
        echo "The file $config_file does not exist."
    fi
    BUILD_CHANGELIST="$current_cl_id"
    BUILD_BRANCH="$current_branch"
fi

# generate documentation
if [[ "$1" == "docs" ]]; then
    # set symbols...
    VERSION=$(cat VERSION)
    CHANGELIST="$BUILD_CHANGELIST"
    BRANCH="$BUILD_BRANCH"
    LAST_MODIFIED=$(date)
    # git --no-pager log --pretty="format:[%h](https://dev.v4.games/v4games/v4k/commit/%H): %s (**%cN**)" > changelog.txt
    cat CHANGELOG.md > changelog.txt

    # ...and generate docs
    # gcc tools/docs/docs.c engine/v4k.c -Iengine -O2 -DNDEBUG $2

    # Concatenate all eng_*.h files into v4k_header_temp
    > v4k_header_temp
    for f in engine/split/eng_*.h; do
        echo "#line 1 \"${f##*/}\"" >> v4k_header_temp
        cat "$f" >> v4k_header_temp
    done

    tools/docs v4k_header_temp --excluded=3rd_glad.h,v4k.h,eng_compat.h, > v4k.html
    mv -f v4k.html engine/
    rm changelog.txt
    rm v4k_header_temp

    exit 0
fi

if [[ "$1" == "push" ]]; then
    bash make.sh bind
    # bash make.sh vps
    # bash make.sh tidy

    if [[ -d ".ark" ]]; then
        tools/ark commit -ws_cl 1
    fi

    exit 0
fi

if [[ "$1" == "pull" ]]; then
    if [[ -d ".ark" ]]; then
        tools/ark get -cl latest
    fi
    exit 0
fi

if [[ "$1" == "sync" ]]; then
    ssh -i ~/.ssh/id_rsa node@192.168.1.28 "/bin/bash /home/node/sync.sh"
    exit 0
fi

if [[ "$1" == "shipit" ]]; then
    ssh -i ~/.ssh/id_rsa node@192.168.1.28 "/bin/bash /home/node/release.sh $2"
    exit 0
fi

# fuse binaries and zipfiles
if [[ "$1" == "fuse" ]]; then
    if [[ "$2" == "cook" ]]; then
        rm -f *.zip
        tools/cook --cook-jobs=1
    fi
    for i in *.exe; do
        if [[ "${i:0:6}" != "fused_" ]]; then
            cp -f "$i" "fused_$i" 2>/dev/null
            tools/fuser "fused_$i" *.zip
        fi
    done
    exit 0
fi

# run autotests
if [[ "$1" == "test" ]]; then
    bash TEST.sh
    exit $?
fi

# check memory api calls
if [[ "$1" == "checkmem" ]]; then
    grep -RnC "[^_xv]realloc("  engine/split/eng*
    grep -RnC "[^_xv]REALLOC("  engine/split/eng*
    grep -RnC "[^_xv]MALLOC("   engine/split/eng*
    grep -RnC "[^_xv]xrealloc(" engine/split/eng*
    grep -RnC "[^_xv]malloc("   engine/split/eng*
    grep -RnC "[^_xv]free("     engine/split/eng*
    grep -RnC "[^_xv]calloc("   engine/split/eng*
    grep -RnC "[^_xv]strdup("   engine/split/eng*
    grep -RnC "[^_xv]array_init("   engine/split/eng*
    grep -RnC "[^_xv]array_resize("   engine/split/eng*
    grep -RnC "[^_xv]array_reserve("   engine/split/eng*
    grep -RnC "[^_xv]array_push("   engine/split/eng*
    grep -RnC "[^_xv]array_push_front("   engine/split/eng*
    grep -RnC "[^_xv]array_free("   engine/split/eng*
    grep -RnC "[^_xv]array_free("   engine/split/eng*
    grep -RnC "[^_xv]set_init("   engine/split/eng*
    grep -RnC "[^_xv]set_insert("   engine/split/eng*
    grep -RnC "[^_xv]map_init("   engine/split/eng*
    grep -RnC "[^_xv]map_insert("   engine/split/eng*
    exit 0
fi

if [[ "$1" == "todo" ]]; then
    grep -RnC "[^_xv]@todo"  engine/split/eng*
    grep -RnC "[^_xv]@fixme"  engine/split/eng*
    exit 0
fi

if [[ "$1" == "leak" ]]; then
    grep -RnC "[^_xv]@leak"  engine/split/eng*
    exit 0
fi

if [[ "$1" == "html5" ]]; then
    bash MAKE.sh prep
    pushd demos/html5
        bash make.sh $2
    popd
    exit 0
fi

if [[ "$1" == "web" ]]; then
    python demos/html5/host.py --directory demos/html5 --bind 127.0.0.1 8000
    exit 0
fi

if [[ "$1" == "vps" ]]; then
    bash make.sh docs
    scp -i ~/.ssh/id_rsa engine/v4k.html app@192.168.1.21:/home/v4k/htdocs/v4k.dev/index.html
    exit 0
fi

# tidy environment
if [[ "$1" == "tidy" ]]; then
    bash demos/html5/make.sh tidy      > /dev/null 2>&1
    mv -f ??-*.png demos          > /dev/null 2>&1
    mv -f ??-*.c demos            > /dev/null 2>&1
    rm -f bind/v4k.dll                > /dev/null 2>&1
    rm -f bind/*.zip                  > /dev/null 2>&1
    rm -f .temp*.*                    > /dev/null 2>&1
    rm -f *.zip                       > /dev/null 2>&1
    rm -f *.mem                       > /dev/null 2>&1
    rm -f *.exp                       > /dev/null 2>&1
    rm -f *.exe.manifest              > /dev/null 2>&1
    rm -f tools/*.exp                 > /dev/null 2>&1
    rm -f *.lib                       > /dev/null 2>&1
    rm -f *.tmp                       > /dev/null 2>&1
    rm -f *.exe                       > /dev/null 2>&1
    rm -f *.log                       > /dev/null 2>&1
    rm -f *.obj                       > /dev/null 2>&1
    rm -f tools/*.obj                 > /dev/null 2>&1
    rm -f *.o                         > /dev/null 2>&1
    rm -f *.a                         > /dev/null 2>&1
    rm -f *.pdb                       > /dev/null 2>&1
    rm -f *.ilk                       > /dev/null 2>&1
    rm -f *.png                       > /dev/null 2>&1
    rm -f *.mp4                       > /dev/null 2>&1
    rm -f *.def                       > /dev/null 2>&1
    rm -f *.dll                       > /dev/null 2>&1
    rm -f *.ini                       > /dev/null 2>&1
    rm -f *.csv                       > /dev/null 2>&1
    rm -f 3rd_*.*                     > /dev/null 2>&1
    rm -f v4k_*.*                     > /dev/null 2>&1
    rm -f v4k.html                    > /dev/null 2>&1
    rm -f changelog.txt               > /dev/null 2>&1
    rm -f steam_appid.txt             > /dev/null 2>&1
# rm -f ??-*.*                      > /dev/null 2>&1
    rm -f temp_*.*                    > /dev/null 2>&1
    rm -rf .vs                    > /dev/null 2>&1
    rm -rf _debug                 > /dev/null 2>&1
    rm -rf _devel                 > /dev/null 2>&1
    rm -rf _release               > /dev/null 2>&1
    rm -rf _cache                 > /dev/null 2>&1
    rm -rf _deploy                > /dev/null 2>&1
    rm -rf tests/out              > /dev/null 2>&1
    rm -rf tests/diff             > /dev/null 2>&1
# rm -rf _project               > /dev/null 2>&1
    rm -f tcc.sh                     > /dev/null 2>&1
    rm -f sh.sh                      > /dev/null 2>&1
    exit 0
fi

cc=${cc:-}
dll=dll
build=dev
args="-Iengine"
run_args=""
other=""
v4k=yes
hello=no
demos=no
lab=no
editor=no
vis=no
proj=no
rc=0
run=no
share=no
addons=""
addons_names=""
addons_includes=""

if [[ "$1" == "addons[" ]]; then
    # plugins are always included in form "<gh username>/<gh repo>/plugin.h"
    addon_includes="-Iplugins $addon_includes"
    shift
    while [[ "$1" != "]" ]]; do
        addon_names="$1 $addon_names"

        # depot folder
        if [[ -d "depot/deps/$1" ]]; then
            addon_includes="-Idepot/deps/$1 $addon_includes"
            if [[ -f "depot/deps/$1/$1.cpp" ]]; then
                addons="depot/deps/$1/$1.cpp $addons"
            else
                addons="depot/deps/$1/$1.c $addons"
            fi
            if [[ -d "depot/deps/$1/include" ]]; then
                addon_includes="-Idepot/deps/$1/include $addon_includes"
            fi
        fi
        if [[ -d "plugins/$1" ]]; then
            # addon_includes="-Iplugins/$1 $addon_includes"
            if [[ -f "plugins/$1/$1.cpp" ]]; then
                addons="plugins/$1/$1.cpp $addons"
            else
                addons="plugins/$1/$1.c $addons"
            fi
            if [[ -f "plugins/$1/plugin.cpp" ]]; then
                addons="plugins/$1/plugin.cpp $addons"
            else
                addons="plugins/$1/plugin.c $addons"
            fi
            if [[ -d "plugins/$1/include" ]]; then
                addon_includes="-Iplugins/$1/include $addon_includes"
            fi
        fi
        shift
    done
    shift
fi

parse_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --) shift; break ;;
            //) shift; run_args="$*"; break ;;
            dll) dll="$1" ;;
            static) dll="$1" ;;
            dbg|dev|rel|ret) build="$1" ;;
            debug) build="dbg" ;;
            devel|develop|developer|development) build="dev" ;;
            release) build="rel" ;;
            vis) vis=yes ;;
            nov4k) v4k=no ;;
            nodemos) demos=no ;;
            demos) demos=yes; hello=no ;;
            lab) lab=yes; hello=no ;;
            noeditor) editor=no ;;
            editor) editor=yes; v4k=yes; hello=no ;;
            run) run=yes ;;
            share) share=yes; dll=static; cc=gcc ;;
            all) v4k=yes; demos=yes; lab=yes; hello=yes ;;
            tcc|cl|cc|gcc|clang|clang-cl) cc="$1" ;;
            vc) cc=gcc ;;
            proj) proj=yes ;;
            *) other="$other $1"; editor=no; demos=no ;;
        esac
        shift
    done
    
    while [[ $# -gt 0 ]]; do
        compiler_flag="${1//=/ }"
        args="$args $compiler_flag"
        shift
    done
}

parse_args "$@"

vs=00
# detect setup
if [[ -z "$cc" ]]; then
    cc=gcc
    echo "GCC!"
    if ! command -v gcc &> /dev/null; then
        echo "Detecting GCC ..."
        if ! command -v gcc &> /dev/null; then
            echo "Detecting TCC ..."
            cc=tcc
        fi
    fi
fi

# solution. @todo: lin/osx
if [[ "$proj" == "yes" ]]; then
    if [[ "$vs" != "00" ]]; then
        pushd tools && premake5 vs20$vs && popd
    else
        pushd tools && premake5 vs2013 && popd
    fi
    pushd tools && premake5 ninja && popd
    pushd tools && premake5 gmake && popd
    exit 0
fi

if [[ "$share" == "yes" ]]; then
    bash make.sh tidy
fi

echo "build=$build, type=$dll, cc=$cc, other=$other, args=$args"
echo "import=$import, export=$export"
echo "addons=$addon_names"

if [[ -z "$BUILD_CHANGELIST" ]]; then BUILD_CHANGELIST=0; fi
if [[ -z "$BUILD_BRANCH" ]]; then BUILD_BRANCH=0; fi

LAST_MODIFIED=$(date)

args="-DBUILD_VERSION=\"$LAST_MODIFIED$BUILD_CHANGELIST-$BUILD_BRANCH-$build-$dll\" $args"

if [[ "$cc" == "tcc" ]]; then cc="tools/tcc"; fi

# detect whether user-defined sources use single-header distro
# if so, remove API=IMPORT flags and also do not produce v4k.dll by default
if [[ -n "$other" ]]; then
    if grep -q "V4K_IMPLEMENTATION" $other; then
        echo "V4K_IMPLEMENTATION found in $other"
    fi
fi

# framework
if [[ "$v4k" == "yes" ]]; then
    tools/file2hash engine/v4k.c engine/v4k.h engine/v4k $addons -- $build $import $export $args $dll > /dev/null
    cache="_cache/.$?"
    mkdir -p _cache

    if [[ ! -f "$cache" ]]; then
        echo "v4k"
        $cc engine/v4k.c $addons -DADDON $addon_includes $export $args || rc=1
        if [[ "$rc" != "1" ]]; then
            if [[ "$dll" == "dll" ]]; then cp -f v4k.dll bind/v4k.dll > /dev/null; fi

            # cache for `make rel` gcc:48s->25s, tcc:3.3s->1.8s
            touch "$cache"
            [[ -f v4k.o ]] && cp -f v4k.o "$cache.o" 2>/dev/null
            [[ -f v4k.obj ]] && cp -f v4k.obj "$cache.obj" 2>/dev/null
            [[ -f v4k.lib ]] && cp -f v4k.lib "$cache.lib" 2>/dev/null
            [[ -f v4k.dll ]] && cp -f v4k.dll "$cache.dll" 2>/dev/null
            [[ -f v4k.def ]] && cp -f v4k.def "$cache.def" 2>/dev/null
            [[ -f v4k.pdb ]] && cp -f v4k.pdb "$cache.pdb" 2>/dev/null
        fi
    else
        # cached. do not compile...
        echo "v4k.c (cached)"
        if [[ "$dll" == "dll" ]]; then cp -f "$cache.dll" bind/v4k.dll > /dev/null || rc=1; fi

        [[ -f "$cache.o" ]] && cp -f "$cache.o" v4k.o 2>/dev/null
        [[ -f "$cache.obj" ]] && cp -f "$cache.obj" v4k.obj 2>/dev/null
        [[ -f "$cache.lib" ]] && cp -f "$cache.lib" v4k.lib 2>/dev/null
        [[ -f "$cache.dll" ]] && cp -f "$cache.dll" v4k.dll 2>/dev/null
        [[ -f "$cache.def" ]] && cp -f "$cache.def" v4k.def 2>/dev/null
        [[ -f "$cache.pdb" ]] && cp -f "$cache.pdb" v4k.pdb 2>/dev/null
    fi
fi

# editor
if [[ "$editor" == "yes" ]]; then
    edit="-DCOOK_ON_DEMAND"
    # edit="-DUI_LESSER_SPACING -DUI_ICONS_SMALL $edit"
    echo "editor"
    $cc -o editor engine/editor.c engine/v4k.c $addon_includes $edit $args || rc=1

    # if [[ "$cc" == "gcc" ]]; then
    # plug_export="-shared"
    # elif [[ "$cc" == "clang" ]]; then
    # plug_export="-shared"
    # else
    # plug_export="-shared"
    # fi

    # for f in workbench/plugins/*.c; do
    #     echo "${f##*/}"
    #     $cc -o "${f%.*}.so" "$f" -Iworkbench $plug_export $args $import || rc=1
    # done

    # echo "workbench"
    # $cc -o workbench workbench/workbench.c -Iworkbench $args $import || rc=1
fi

# demos
if [[ "$demos" == "yes" ]]; then
    for f in demos/??-*; do
        fname="${f##*/}"
        fname="${fname%.*}"
        if [[ "$fname" =~ ^[0-9][0-9]- ]]; then
            if [[ "${fname:0:2}" != "99" ]]; then
                limport="$import"
                if grep -q "V4K_IMPLEMENTATION" "demos/$fname.c"; then
                    echo "V4K_IMPLEMENTATION found in demos/$fname.c"
                fi
                echo "$fname"
                $cc -o "$fname" "demos/$fname.c" $addon_includes $limport $args || rc=1
            fi
        fi
    done
fi

# lab
if [[ "$lab" == "yes" ]]; then
    for f in demos/99-*; do
        limport="$import"
        if grep -q "V4K_IMPLEMENTATION" "$f"; then
            echo "V4K_IMPLEMENTATION found in $f"
        fi
        echo "${f##*/}"
        $cc -o "${f%.*}" "$f" $addon_includes $limport $args || rc=1
    done
fi

# user-defined apps
if [[ -n "$other" ]]; then
    if [[ "$vis" == "yes" ]]; then echo "$cc $other $import $args"; fi
    # if [[ "$cc" == "gcc" ]]; then
    #     if [[ "$build" == "rel" ]]; then
    #         import="$import engine/v4k_win32_rel_glue.c"
    #         args="$args -mwindows"
    #     fi
    # fi
    for f in $other; do
        exename="${f%.*}"
        rm -f "$exename"
        echo "$f"
        $cc "$f" $addon_includes $import $args || rc=1
    done
fi

if [[ "$run" == "yes" ]]; then
    if [[ "$rc" == "1" ]]; then
        echo "build failed. skipping run!"
    else
        exename="hello"
        if [[ -n "$other" ]]; then
            exename="${other%.*}"
        fi
        if [[ "$build" == "dbg" ]]; then
            echo "dbg $exename $run_args"
            # if ! pgrep -x "gdb" > /dev/null; then
                gdb --args "./$exename" $run_args || rc=1
            # else
            #     echo "gdb is already running. Skipping launch."
            # fi
        else
            echo "run $exename $run_args"
            "./$exename" $run_args || rc=1
        fi
    fi
fi

if [[ "$share" == "yes" ]]; then
    if [[ "$rc" == "1" ]]; then
        echo "build failed. skipping run!"
    else
        exename="hello"
        if [[ -n "$other" ]]; then
            exename="${other%.*}"
        fi
        echo "run $exename $run_args"
        "./$exename" --cook-on-demand=1 $run_args || rc=1
        # tools/upx "$exename"
        bash make.sh fuse
    fi
fi

exit $rc
