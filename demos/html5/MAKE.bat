if "%1"=="" MAKE.bat demo_ddraw.c

del index.html 2>NUL >NUL
del index.worker.js 2>NUL >NUL
del index.data 2>NUL >NUL

rem clone emscripten sdk
if not exist "emsdk" (
	git clone https://github.com/emscripten-core/emsdk emsdk
	pushd emsdk
		call emsdk install  3.0.0 && rem latest
		call emsdk activate 3.0.0 && rem latest
	popd
)
if "%EMSDK%"=="" call emsdk\emsdk_env.bat --system

rem cook art
..\..\tools\cook.exe --cook-jobs=1 --cook-ini=..\..\tools\cook_web.ini

rem host webserver, compile and launch
rem start python -m http.server --bind 127.0.0.1 8000
call emcc %* -g ..\..\engine\v4k.c -I..\..\engine -o index.html -pthread -s FULL_ES3 -s USE_PTHREADS -s USE_GLFW=3 -s SINGLE_FILE=1 -s PRECISE_F32=1 -s TOTAL_MEMORY=256mb -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=worker,web --shell-file template.html -Wfatal-errors --preload-file .art[00].zip@index.zip -lidbfs.js
rem emrun index.html
