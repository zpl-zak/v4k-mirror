cl game.c ..\..\..\engine\v4k.c -I..\..\..\engine %* /link /SUBSYSTEM:WINDOWS /entry:mainCRTStartup

del *.obj
del *.exp
del *.lib
del *.pdb
