cd /d %~dp0
call ..\make.bat dll
call ..\make.bat bind

luajit hello.lua
python hello.py
cd ..
