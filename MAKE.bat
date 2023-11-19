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
    echo %0 [fwk_prep]              ; prepare files for fwk PR
    echo %0 [fwk]                   ; update fwk-mirror fork
    echo %0 [lua]                   ; execute lua script with v4k
    echo %0 [html5]                 ; build HTML5 demo
    echo %0 [web]                   ; run Python webserver in html5 dir
    echo %0 [pull]                  ; pull changes from origin
    echo %0 [push]                  ; prepare for commit, stage changes and commit them
    echo %0 [dstat]                 ; show depot changes
    echo %0 [dpush]                 ; push depot changes
    echo %0 [depot]                 ; sync depot changes
    echo %0 [fuse]                  ; fuse all binaries and cooked zipfiles found together
    echo %0 [git]                   ; prepare for commit
    echo %0 [vps]                   ; upload the release to VPS
    echo %0 [tidy]                  ; clean up temp files
    echo %0 [bind]                  ; generate lua bindings
    echo %0 [test]                  ; check untracked allocators in V4K
    echo %0 [todo]                  ; check for @fixme and @todo
    echo %0 [v4web]                 ; sync v4 website
    echo %0 [split^|join]            ; engine/v4k* ^>split^> engine/split/* or engine/split/* ^>join^> engine/v4k*
    echo %0 [lua]                   ; execute lua script with v4k
    echo %0 [amalgamation]          ; combine engine/v4k* into a single-header file
    echo %0 [prep]                  ; combine split files into a single-header file, ready for use
    echo %0 [sln]                   ; generate a xcode/gmake/ninja/visual studio solution
    echo %0 [cl^|tcc^|cc^|gcc^|clang^|clang-cl] [dbg^|dev^|rel^|ret] [static^|dll] [nov4k^|nodemos^|editor] [vis] [-- args]
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
    echo    run      ^| run compiled .exe
    echo    vis      ^> visualize invokation cmdline.
    echo    args     ^> after `--` separator is found, pass all remaining arguments to compiler as-is
    echo    run_args ^> after `//` separator is found, pass all remaining arguments to runtime exe as-is
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
if "%1"=="build_cook" (
    pushd tools
        cl  cook.c      -I..\engine /openmp /Os /Ox /O2 /Oy /MT /DNDEBUG /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
    popd

    exit /b
)
rem generate bindings
if "%1"=="bind" (
    rem luajit
    tools\luajit tools\luajit_make_bindings.lua > v4k.lua
    move /y v4k.lua bind

    exit /b
)

if "%1"=="lua" (
    pushd bind
        luajit "..\%2"
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
    git --no-pager log --pretty="format:[%%h](https://dev.v4.games/v4games/v4k/commit/%%H): %%s (**%%cN**)" > changelog.txt

    rem ...and generate docs
    cl   tools\docs\docs.c engine\v4k.c -Iengine %2
    docs engine\v4k.h --excluded=3rd_glad.h,v4k.h,v4k_compat.h, > v4k.html
    move /y v4k.html engine\
    del changelog.txt
    del info.obj

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
type engine\split\3rd_icon_md.h                                          >> v4k.h
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
    call make.bat prep
    call make.bat bind
    rem call make.bat docs

    call make.bat amalgamation
    rem call make.bat split

rem rd /q /s engine\split
rem md engine\split
rem move /y v4k_*.? engine\split\
rem move /y 3rd_*.? engine\split\

    call make.bat tidy

    exit /b
)

if "%1"=="pull" (
    git pull
    exit /b
)

if "%1"=="depot" (
    pushd depot\
        git pull
    popd
    git submodule update --remote --merge depot/
    exit /b
)

if "%1"=="dstat" (
    pushd depot
        git status
    popd
    exit /b
)

if "%1"=="dpush" (
    pushd depot
        git add .
        if "%2"=="auto" (
            git commit -m "asset update"
            git push
        ) else (
            git commit
            if not "%2"=="local" (
                git push
            )
        )
    popd

    if not "%2"=="noroot" (
        git stash
        git add depot
        git commit -m "sync depot"
        if not "%3"=="local" (
            git pull
            git push
        )
        git stash pop
    )

    exit /b
)

if "%1"=="push" (
    call make.bat tidy

    git status
    if "%2"=="dp" (
        call MAKE.bat dpush auto
    )
    git add .
    git commit
    if not "%2"=="local" (
        git push
    )
    call make.bat vps
    call make.bat tidy

    exit /b
)

if "%1"=="prep" (
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

rem fuse binaries and zipfiles
if "%1"=="fuse" (
    setlocal enableDelayedExpansion
    if "%2"=="cook" (
        del *.zip 2> nul 1> nul & tools\cook --cook-jobs=1
    )
    for %%i in (*.exe) do set "var=%%i" && if not "!var:~0,6!"=="fused_" ( copy /y !var! fused_!var! 2>nul 1>nul & tools\ark fused_!var! *.zip )
    endlocal
    exit /b
)

rem check memory api calls
if "%1"=="test" (
    findstr /RNC:"[^_xv]realloc[(]"  engine\split\v4k*
    findstr /RNC:"[^_xv]REALLOC[(]"  engine\split\v4k*
    findstr /RNC:"[^_xv]MALLOC[(]"   engine\split\v4k*
    findstr /RNC:"[^_xv]xrealloc[(]" engine\split\v4k*
    findstr /RNC:"[^_xv]malloc[(]"   engine\split\v4k*
    findstr /RNC:"[^_xv]free[(]"     engine\split\v4k*
    findstr /RNC:"[^_xv]calloc[(]"   engine\split\v4k*
    findstr /RNC:"[^_xv]strdup[(]"   engine\split\v4k*
    exit /b
)

if "%1"=="todo" (
    findstr /RNC:"[^_xv]@todo"  engine\split\v4k*
    findstr /RNC:"[^_xv]@fixme"  engine\split\v4k*
    exit /b
)

if "%1"=="v4web" (
    pushd website\
        git pull origin main
        git add .
        git commit -m "website update"
        git push origin main
    popd
    git stash
    git add website
    git commit -m "sync website"
    git pull
    git push
    git stash pop
    exit /b
)

if "%1"=="html5" (
    pushd demos\html5
        call make.bat %2
    popd
    exit /b
)

if "%1"=="web" (
    python demos\html5\host.py --directory demos\html5 --bind 127.0.0.1 8000
    exit /b
)

if "%1"=="vps" (
    call make.bat docs
    tools\pscp -4 -batch -agent -P 22 -l app engine\v4k.html 128.140.14.212:/home/app/microblog/app/static/v4k/index.html
    rem tools\pscp -4 -batch -agent -P 22 -l app engine\joint\v4k.h 128.140.14.212:/home/app/microblog/app/static/v4k/v4k.h
    exit /b
)

if "%1"=="fwk" (
    pushd ..\fwk-mirror
        call MAKE.bat sync
    popd
    call MAKE.bat fwk_prep
    start "" fwk_diff.WinMerge
    exit /b
)

if "%1"=="fwk_prep" (
    if not exist "_fwk" mkdir "_fwk"
    if not exist "_fwk\demos" mkdir "_fwk\demos"
    if not exist "_fwk\tools" mkdir "_fwk\tools"
    if not exist "_fwk\tools\editor" mkdir "_fwk\tools\editor"
    if not exist "_fwk\engine" mkdir "_fwk\engine"
    if not exist "_fwk\engine\art" mkdir "_fwk\engine\art"
    if not exist "_fwk\engine\art\shaders" mkdir "_fwk\engine\art\shaders"
    if not exist "_fwk\engine\split" mkdir "_fwk\engine\split"
    setlocal enabledelayedexpansion
    xcopy /y "*" "_fwk"
    xcopy /y "engine\split\3rd_*" "_fwk\engine\split"
    xcopy /y "engine\art\shaders\*" "_fwk\engine\art\shaders"
    xcopy /y "demos" "_fwk\demos"
    copy /y "engine\editor.c" "_fwk\engine\editor.c"
    rem xcopy /y/E "tools "_fwk\tools"
    for %%f in ("engine\split\v4k*") do (
        set "filename=%%~nf"
        set "newname=fwk!filename:v4k=!%%~xf"
        echo Copying and renaming "%%f" to "_fwk\engine\split\!newname!"
        copy "%%f" "_fwk\engine\split\!newname!"
    )
    for %%f in (_fwk\engine\split\*) do (
        set "filename=%%~nxf"
        if /i not "!filename:~0,4!"=="3rd_" (
            echo Processing: %%f
            tools\fwkren.exe %%f from
        ) else (
            echo Skipping %%f
        )
    )
    for %%f in (_fwk\demos\*.c) do (
        set "filename=%%~nxf"
        echo Processing: %%f
        tools\fwkren.exe %%f from
    )
    rem for %%f in (_fwk\tools\*) do (
    rem     set "filename=%%~nxf"
    rem     echo Processing: %%f
    rem     tools\fwkren.exe %%f from
    rem )
    rem for %%f in (_fwk\tools\editor\*.c) do (
    rem     set "filename=%%~nxf"
    rem     echo Processing: %%f
    rem     tools\fwkren.exe %%f from
    rem )

    rem tools\fwkren.exe tools\cook.ini from

    echo All done.
    endlocal
    exit /b
)

if "%1"=="back" (
    if not exist "_fwk" exit /b
    setlocal enabledelayedexpansion

    xcopy /y "_fwk" "."
    xcopy /y "_fwk\engine\split\3rd_*" "engine\split"
    xcopy /y "_fwk\engine\art\shaders\*" "engine\art\shaders"
    del "demos\*.c"
    xcopy /y "_fwk\demos" "demos"
    xcopy /y "_fwk\engine\editor.c" "engine\editor.c"
    tools\fwkren.exe "engine\editor.c" to
    tools\fwkren.exe "hello.c" to

    rem xcopy /y/E "_fwk\tools "tools"
    for %%f in ("_fwk\engine\split\fwk*") do (
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
    for %%f in (demos\*.c) do (
        set "filename=%%~nxf"
        echo Processing: %%f
        tools\fwkren.exe %%f to
    )
    rem for %%f in (tools\*.c) do (
    rem     set "filename=%%~nxf"
    rem     echo Processing: %%f
    rem     tools\fwkren.exe %%f to
    rem )
    rem for %%f in (tools\editor\*.c) do (
    rem     set "filename=%%~nxf"
    rem     echo Processing: %%f
    rem     tools\fwkren.exe %%f to
    rem )

    rem tools\fwkren.exe tools\cook.ini to

    echo All done.
    endlocal
    exit /b
)

if "%1"=="fwk_mir" (
    rd/q/s _fwk
    rd/q/s ..\_fwk
    mkdir ..\_fwk
    setlocal enabledelayedexpansion
    xcopy /y/E "*" "..\_fwk"
    move "..\_fwk" _fwk
    for %%f in ("engine\split\v4k*") do (
        set "filename=%%~nf"
        set "newname=fwk!filename:v4k=!%%~xf"
        echo Copying and renaming "%%f" to "_fwk\engine\split\!newname!"
        copy "%%f" "_fwk\engine\split\!newname!"
    )
    for %%f in (_fwk\engine\split\*) do (
        set "filename=%%~nxf"
        if /i not "!filename:~0,4!"=="3rd_" (
            echo Processing: %%f
            tools\fwkren.exe %%f from
        ) else (
            echo Skipping %%f
        )
    )
    for %%f in (_fwk\demos\*.c) do (
        set "filename=%%~nxf"
        echo Processing: %%f
        tools\fwkren.exe %%f from
    )
    for %%f in (_fwk\tools\*) do (
        set "filename=%%~nxf"
        echo Processing: %%f
        tools\fwkren.exe %%f from
    )
    for %%f in (_fwk\tools\editor\*.c) do (
        set "filename=%%~nxf"
        echo Processing: %%f
        tools\fwkren.exe %%f from
    )

    tools\fwkren.exe _fwk\tools\cook.ini from

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
    del bind\v4k.dll                > nul 2> nul
    del .temp*.*                    > nul 2> nul
    del *.zip                       > nul 2> nul
    del *.mem                       > nul 2> nul
    del *.exp                       > nul 2> nul
    del *.exe.manifest              > nul 2> nul
    del tools\*.exp                 > nul 2> nul
    del *.lib                       > nul 2> nul
    del *.exe                       > nul 2> nul
    del *.log                       > nul 2> nul
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
    del *.csv                       > nul 2> nul
    del 3rd_*.*                     > nul 2> nul
    del v4k_*.*                     > nul 2> nul
    del v4k.html                    > nul 2> nul
    del changelog.txt               > nul 2> nul
rem del ??-*.*                      > nul 2> nul
    del temp_*.*                    > nul 2> nul
    rd /q /s .vs                    > nul 2> nul
    rd /q /s _debug                 > nul 2> nul
    rd /q /s _devel                 > nul 2> nul
    rd /q /s _release               > nul 2> nul
    rd /q /s _fwk                   > nul 2> nul
    rd /q /s _cache                 > nul 2> nul
    rd /q /s _deploy                 > nul 2> nul
rem rd /q /s _project               > nul 2> nul
    del tcc.bat                     > nul 2> nul
    del sh.bat                      > nul 2> nul
    exit /b
)

set cc=%cc%
set dll=dll
set build=dev
set args=-Iengine
set run_args=
set other=
set v4k=yes
set hello=no
set demos=no
set lab=no
set editor=no
set vis=no
set proj=no
set rc=0
set run=no

:parse_args
    if "%1"=="--"       shift && goto parse_compiler_args
    if "%1"=="//"       shift && goto parse_runtime_args

    if "%1"=="dll"      set "dll=%1" && goto loop
    if "%1"=="static"   set "dll=%1" && goto loop

    if "%1"=="dbg"      set "build=%1" && goto loop
    if "%1"=="dev"      set "build=%1" && goto loop
    if "%1"=="rel"      set "build=%1" && goto loop
    if "%1"=="ret"      set "build=%1" && goto loop

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
    if "%1"=="lab"      set "lab=yes" && set "hello=no" && goto loop
    if "%1"=="noeditor" set "editor=no" && goto loop
    if "%1"=="hello"    set "hello=yes" && goto loop
    if "%1"=="editor"   set "editor=yes" && set "v4k=yes" && set "hello=no"&& goto loop
    if "%1"=="run"      set "run=yes" && goto loop
    if "%1"=="all"      set "v4k=yes" && set "demos=yes" && set "lab=yes" && set "editor=yes" && set "hello=yes" && goto loop

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

:parse_runtime_args
    if not "%1"==""     set "run_args=!run_args! %1" && shift && goto parse_runtime_args

set vs=00
rem detect setup
if "!cc!"=="" (
    set cc=cl
    where cl /q
    if not %ERRORLEVEL%==0 (
        echo Detecting VS 2022/2019/2017/2015/2013 x64 ...
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

    if "!build!"=="ret" (
        set args=-DENABLE_RETAIL -Dmain=WinMain !args!
        set args=/nologo /Zi /MT /openmp /DNDEBUG=3 !args!        /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
    )
    if "!build!"=="rel" (
        set args=/nologo /Zi /MT /openmp /DNDEBUG=2 !args!        /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2 /link /OPT:ICF /LTCG
    )
    if "!build!"=="dev" (
        set args=/nologo /Zi /MT /openmp /DNDEBUG=1 !args!        && REM /Os /Ox /O2 /Oy /GL /GF /Gw /arch:AVX2
    )
    if "!build!"=="dbg" (
        set args=/nologo /Zi /MT         /DEBUG   !args!          /Od  /fsanitize=address
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

    set warnings_fwkc=-Wno-deprecated-declarations -Wno-tautological-constant-out-of-range-compare
    set warnings_demos=-Wno-empty-body -Wno-format-security -Wno-pointer-sign
    set warnings=!warnings_fwkc! !warnings_demos!

    if "!build!"=="ret" (
        set args=-DENABLE_RETAIL -Dmain=WinMain !args!
        set args=!warnings! /nologo /Zi /MT /openmp /DNDEBUG=3 !args!        /Os /Ox /O2 /Oy /GF /Gw /arch:AVX2
    )
    if "!build!"=="rel" (
        set args=!warnings! /nologo /Zi /MT /openmp /DNDEBUG=2 !args!        /Os /Ox /O2 /Oy /GF /Gw /arch:AVX2
    )
    if "!build!"=="dev" (
        set args=!warnings! /nologo /Zi /MT /openmp /DNDEBUG=1 !args!        && REM /Os /Ox /O2 /Oy /GF /Gw /arch:AVX2
    )
    if "!build!"=="dbg" (
        set args=!warnings! /nologo /Zi /MT         /DEBUG     !args!        /Od  /fsanitize=address
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

    if "!build!"=="ret" (
        set args=-DENABLE_RETAIL -Dmain=WinMain !args!
        set args=-O3 -DNDEBUG=3    !args!
    )
    if "!build!"=="rel" (
        set args=-O2 -DNDEBUG=2    !args!
    )
    if "!build!"=="dev" (
        set args=-O1 -DNDEBUG=1 -g !args!
    )
    if "!build!"=="dbg" (
        set args=-O0            -g !args!
    )

    set o=-o
    set echo=echo

) else ( rem if "!cc!"=="gcc" or "clang"

    set libs=-lws2_32 -lgdi32 -lwinmm -ldbghelp -lole32 -lshell32 -lcomdlg32

    if "!dll!"=="static" (
        set export=-c
        set import=v4k.o -Wl,--allow-multiple-definition
    ) else (
        set export=-DAPI=EXPORT -shared -o v4k.dll -Wl,--out-implib,v4k.a
        set import=-DAPI=IMPORT v4k.a
    )

    set args=-Wno-implicit-function-declaration !libs! !args!

    if "!build!"=="ret" (
        set args=-DENABLE_RETAIL   !args!
        set args=-O3 -DNDEBUG=3    !args!
    )
    if "!build!"=="rel" (
        rem @todo see: https://stackoverflow.com/questions/866721/how-to-generate-gcc-debug-symbol-outside-the-build-target
        set args=-O2 -DNDEBUG=2    !args!
    )
    if "!build!"=="dev" (
        set args=-O1 -DNDEBUG=1 -g !args!
    )
    if "!build!"=="dbg" (
        set args=-O0            -g !args!
    )

    set o=-o
    set echo=echo
)

echo build=!build!, type=!dll!, cc=!cc!, other=!other!, args=!args!
echo import=!import!, export=!export!

rem set BUILD_VERSION symbol
git describe --tags --abbrev=0 > info.obj
set /p VERSION=<info.obj
git rev-list --count --first-parent HEAD > info.obj
set /p GIT_REVISION=<info.obj
git rev-parse --abbrev-ref HEAD > info.obj
set /p GIT_BRANCH=<info.obj
date /t > info.obj
set /p LAST_MODIFIED=<info.obj
set args=-DBUILD_VERSION="\"!GIT_BRANCH!-!GIT_REVISION!-!build!-!dll!\"" !args!

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
    tools\file2hash engine\v4k.c engine\v4k.h engine\v4k. engine\joint\v4k.h -- !build! !import! !export! !args! !dll! > nul
    set cache=_cache\.!errorlevel!
    md _cache 2>nul >nul

    rem cache for `make rel` cl:48s->25s, tcc:3.3s->1.8s
    if exist !cache!.o   copy /y !cache!.o   v4k.o   2>nul >nul
    if exist !cache!.obj copy /y !cache!.obj v4k.obj 2>nul >nul
    if exist !cache!.lib copy /y !cache!.lib v4k.lib 2>nul >nul
    if exist !cache!.dll copy /y !cache!.dll v4k.dll 2>nul >nul
    if exist !cache!.def copy /y !cache!.def v4k.def 2>nul >nul
    if exist !cache!.pdb copy /y !cache!.pdb v4k.pdb 2>nul >nul

    if not exist "!cache!" (
        !echo! v4k          && !cc! engine\v4k.c !export! !args!   && if "!dll!"=="dll" copy /y v4k.dll bind\v4k.dll  > nul || set rc=1
        echo. > !cache!
        if exist v4k.o   copy /y v4k.o   !cache!.o   2>nul >nul
        if exist v4k.obj copy /y v4k.obj !cache!.obj 2>nul >nul
        if exist v4k.lib copy /y v4k.lib !cache!.lib 2>nul >nul
        if exist v4k.dll copy /y v4k.dll !cache!.dll 2>nul >nul
        if exist v4k.def copy /y v4k.def !cache!.def 2>nul >nul
        if exist v4k.pdb copy /y v4k.pdb !cache!.pdb 2>nul >nul
    ) else (
        rem cached. do not compile...
        echo v4k.c ^(cached^)
        if "!dll!"=="dll" copy /y !cache!.dll bind\v4k.dll > nul || set rc=1
    )
)


rem editor
if "!editor!"=="yes" (
set edit=-DCOOK_ON_DEMAND
rem set edit=-DUI_LESSER_SPACING -DUI_ICONS_SMALL !edit!
!echo! editor      && !cc! !o! editor.exe engine\editor.c !edit! -Iengine/joint !args! || set rc=1

rem if "!cc!"=="cl" (
rem set plug_export=/LD
rem ) else if "!cc!"=="clang-cl" (
rem set plug_export=/LD
rem ) else (
rem set plug_export=-shared
rem )

rem for %%f in ("workbench\plugins\*.c") do (
rem     !echo! %%~nf && !cc! !o! %%~nf.dll %%f -Iworkbench !plug_export! !args! !import! || set rc=1
rem )

rem !echo! workbench && !cc! !o! workbench.exe workbench\workbench.c -Iworkbench !args! !import! || set rc=1
)

rem demos
if "!demos!"=="yes" (
    for %%f in ("demos\??-*") do (
        set "fname=%%~nf"
        echo !fname!| findstr /R "^[0-9][0-9]-" >nul && (
            if not "!fname:~0,2!"=="99" (
                set limport=!import!
                >nul find "V4K_IMPLEMENTATION" "demos\!fname!.c" && (
                    set limport=
                )
                !echo! !fname! && !cc! !o! !fname!.exe "demos\!fname!.c" !limport! !args! || set rc=1
            )
        )
    )
)


rem lab
if "!lab!"=="yes" (
    for %%f in ("demos\99-*") do (
        set limport=!import!
        >nul find "V4K_IMPLEMENTATION" demos\%%~nf.c && (
          set limport=
        )
        !echo! %%~nf         && !cc! !o! %%~nf.exe         demos\%%~nf.c            !limport! !args! || set rc=1
    )
)

rem hello
if "!hello!"=="yes" (
!echo! hello         && !cc! !o! hello.exe         hello.c                           !args! || set rc=1
)

rem user-defined apps
if not "!other!"=="" (
    if "!vis!"=="yes" echo !cc! !other! !import! !args!
    rem if "!cc!"=="cl" (
    rem     if "!build!"=="rel" (
    rem         set "import=!import! engine\v4k_win32_rel_glue.c"
    rem         set "args=!args! /SUBSYSTEM:WINDOWS"
    rem     )
    rem )
    !echo! !other! && !cc! !other! !import! !args! || set rc=1
)

if "!run!"=="yes" (
    if "!rc!"=="1" (
        echo build failed. skipping run!
    ) else (
        set exename=hello.exe
        if not "!other!"=="" (
            for /f "tokens=*" %%a in ("!other!") do set exename=%%~na.exe
        )
        echo run !exename! !run_args!
        !exename! !run_args! || set rc=1
    )
)

rem PAUSE only if double-clicked from Windows explorer
(((echo.%cmdcmdline%)|%WINDIR%\system32\find.exe /I "%~0")>nul)&&pause

cmd /c exit !rc!
