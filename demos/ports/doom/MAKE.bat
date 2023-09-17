..\..\..\tools\tcc game.c PureDOOM.c -I ..\..\..\engine ..\..\..\engine\v4k.c ^
    -DWIN32 ^
    %*

del *.obj
del *.exp
del *.lib
