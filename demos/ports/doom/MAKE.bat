cl game.c PureDOOM.c -I ..\..\..\engine ..\..\..\engine\v4k.c ^
    -DWIN32 ^
    %* ^
    /link /SUBSYSTEM:WINDOWS /entry:mainCRTStartup

del *.obj
del *.exp
del *.lib
del *.pdb
